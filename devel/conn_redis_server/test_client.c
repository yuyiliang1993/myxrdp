#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <sys/ioctl.h>

#define IF_NAME "eth0"


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "public.h"
#include "myredis.h"



#define log_stderr(fmt,...) fprintf(stderr,fmt,##__VA_ARGS__)

#define log_stdout(fmt,...) fprintf(stdout,fmt,##__VA_ARGS__)


#define  MAX_BUFFER_LEN 4096



int GetBindSocket(const char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		perror("socket");
		return -1;
	}

	return sock;
}


int select_wait_sock_read(int sock,long usec){
	struct timeval tim;
	tim.tv_sec = 0;
	tim.tv_usec = usec;

	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(sock,&rset);

	int ret = select(sock+1,&rset,NULL,NULL,&tim);
	if(ret < 0 ){
		perror("select");
		return 0;
	}
	if(ret > 0)
		return ret;
	return 0;
}

typedef struct connInfo{
	char hostname[256];
	char username[256];
	char password[256];
}connInfo_t;


int GetConnecttionInfo(connInfo_t *conn,const char *data){
	char *dupdata = strdup(data);
	int count = 0;
	int k = 0;
	char *p = strtok(dupdata,",");
	for(;p!=NULL;p=strtok(NULL,",")){
		if(k == 0){

		}
		else if(k == 1){
			memset(conn->hostname,0,sizeof(conn->hostname));
			memcpy(conn->hostname,p,strlen(p));
			++count;
		}
		else if(k == 2){
			memset(conn->username,0,sizeof(conn->username));
			memcpy(conn->username,p,strlen(p));
			++count;
		}
		else if(k == 3){
			memset(conn->password,0,sizeof(conn->password));
			memcpy(conn->password,p,strlen(p));
			++count;
		}
		++k;
	}

	if(count != 3){
		return -1;
	}
	
	free(dupdata);
	return 0;
}


int request_conn_server(connInfo_t *conn,const char *ip,int port,const char *sid){	
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		perror("socket");
		return -1;
	}
	struct sockaddr_in serv_addr;
	socklen_t addrlen = sizeof(serv_addr);
	bzero(&serv_addr,addrlen);

	serv_addr.sin_family= AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	char buffer[MAX_BUFFER_LEN];
	bzero(buffer,sizeof(buffer));
	const char *local = "192.168.1.80";
	int len = snprintf(buffer,sizeof(buffer),"$LJXX,%s,%s",sid,local);
	if(len < 0)
		return -1;
	int ret = sendto(sock,buffer,len,0,(struct sockaddr*)&serv_addr,addrlen);
	if(ret <0 ){
		perror("sendto");
		return -1;
	}
	printf("send:%s\n",buffer);
	ret=select_wait_sock_read(sock,1000000);//等待5000毫秒
	if(ret > 0){	//需要读取
		memset(buffer,0,sizeof(buffer));
		int n = recvfrom(sock,buffer,sizeof(buffer),0,NULL,NULL);
		if(n > 0 ){
			printf("recv:%s\n",buffer);
			GetConnecttionInfo(conn, buffer);
		}
	}
	else {
		printf("Wait timeout!\n");
		return -1;
	}
	return 0;
}




int get_local_ip(char * ifname, char * ip)
{
    char *temp = NULL;
    int inet_sock;
    struct ifreq ifr;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 

    memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    memcpy(ifr.ifr_name, ifname, strlen(ifname));

    if(0 != ioctl(inet_sock, SIOCGIFADDR, &ifr)) 
    {   
        perror("ioctl error");
        return -1;
    }

    temp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);     
    memcpy(ip, temp, strlen(temp));

    close(inet_sock);

    return 0;
}


int get_local_ip_by_conf(char *ip,const char*file_conf){
	FILE*fp = fopen(file_conf,"r");
	if(fp == NULL){
		perror("fopen");
		return -1;
	}
	char buffer[1024];
	char *p = NULL;
	while(1){
		memset(buffer,0,sizeof(buffer));
		if(fgets(buffer,1024,fp) == NULL)
			break;
		p = buffer+strlen(buffer)-1;
		while(*p == ' ' || *p=='\n'){
			*p = 0;
			--p;
		}

		p=buffer;
		while(*p == ' ')
			++p;
		
		if(strncmp(p,"localip",7) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				memcpy(ip,pstr,strlen(pstr));
				fclose(fp);
				return 0;
				
		}

	}

	fclose(fp);
	return -1;

}


int get_redis_connection(char *ip,int *port,const char *file_conf){
	FILE*fp = fopen(file_conf,"r");
	if(fp == NULL){
		perror("fopen");
		return -1;
	}

	char buffer[1024];
	char *p = NULL;
	int flags = 0;
	while(1){
		memset(buffer,0,sizeof(buffer));
		if(fgets(buffer,1024,fp) == NULL)
			break;
		p = buffer+strlen(buffer)-1;
		while(*p == ' ' || *p=='\n'){
			*p = 0;
			--p;
		}

		p=buffer;
		while(*p == ' ')
			++p;
		
		if(strncmp(p,"redis_ip",8) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				memcpy(ip,pstr,strlen(pstr));
				++flags;
		}

		if(strncmp(p,"redis_port",strlen("redis_port")) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				*port = atoi(pstr);
				++flags;
		}

	}

	fclose(fp);
	return (flags == 2)?0:-1;
}






void test1(){
	ClusterQueryRedis *cq = new ClusterQueryRedis;
	cq->query("1234567");
	delete cq;
}


int main(void){
	connInfo_t conn;

	request_conn_server(&conn,"127.0.0.1", 11190, "c663c098-1199-4193-9256-f554d9251c6a");

}



int test2(void){
	char ip[32];
	memset(ip,0,32);
   	if(get_local_ip_by_conf(ip, XRDP_NET_CONF_FILE) == 0){
		printf("get ip:%s\n",ip);
	}
	else {
		printf("get ip failed\n");
	}


	char redis_ip[32];
	memset(redis_ip,0,32);
	int redis_port;
	if(get_redis_connection(redis_ip,&redis_port,XRDP_NET_CONF_FILE)==0){
		printf("redis_ip:%s\n",redis_ip);
		printf("redis_port:%d\n",redis_port);
	}
	else {
		printf("get_redis_connection failed\n");
	}
    return 0;
}

