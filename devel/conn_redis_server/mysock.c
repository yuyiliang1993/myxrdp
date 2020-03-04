#include "mysock.h"
#include "myredis.h"
#include "public.h"

typedef struct inter
{
	const char *data;
	int len;
	struct sockaddr_in *addr_cli;
	int sock;
}inter_t;

int openUdpBindSocketIpv4(int port)
{
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0 ){
		perror("socket");
		return -1;
	}
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	bzero(&addr,addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
//	int val = 1;
//	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));	
	if(bind(sock,(struct sockaddr*)&addr,addrlen) < 0){
		perror("bind");
		close(sock);
		return -1;
	}
	return sock;
 }


//断开连接
int DKLJ(inter_t *self)
{
	char *data = strdup(self->data);
	if(data == NULL){
		return 1;
	}
	
	char *p = strstr(data,"$DKLJ");
	if(p == NULL){
		return 1;
	}

	int k = 0;
	char *sid = NULL;
	for(p=strtok(p,",");p!=NULL;p=strtok(NULL,",")){		
		if(k == 0){	
		}
		else if(k== 1){//sid
			sid = (char*)calloc(1,strlen(p));
			memcpy(sid,p,strlen(p));
		}
		++k;
	}
	
	if(sid==NULL){
		return 1;
	}
	
	p = sid+strlen(sid)-1;
	while(*p == '\n' || *p=='\r'){
		*p = 0x00;
		--p;
	}
	
	pull_dis_conn_redis(sid);
	free(data);
	free(sid);
	return 0;
}



//连接信息
int LJXX(inter_t *self)
{
	char *data = strdup(self->data);
	if(data == NULL)
		return 1;
	char *p = strstr(data,"$LJXX");
	if(p == NULL){
		return 1;
	}

	int k = 0;
	char *sid = NULL;
	char *xrdpIp = NULL;
	int w = 0;
	int h = 0;
	int bpp = 0;
	for(p=strtok(p,",");p!=NULL;p=strtok(NULL,",")){	
		if(k == 0){	
		}	
		else if(k== 1){//sid
			sid = (char*)calloc(1,strlen(p));
			memcpy(sid,p,strlen(p));
		}
		else if(k == 2){//xrdp ip
			xrdpIp = (char*)calloc(1,strlen(p));
			memcpy(xrdpIp,p,strlen(p));
		}
		else if(k == 3){
			w = atoi(p);
		}
		else if(k == 4){
			h = atoi(p);
		}
		else if(k == 5){
			bpp = atoi(p);
		}
		++k;
	}

	if(sid == NULL || xrdpIp==NULL){
		return 1;
	}
	
	log_stdout("LJXX:sid-%s,ip-%s,width-%d,height-%d,bpp-%d\n",sid,xrdpIp,w,h,bpp);	

	char buffer[512]={0};
	QueryRedis *cq = NULL;
	ConfigIn *cfg = ConfigIn::getStance();
	cq = new ClusterQueryRedis();
	const char *xrdpip = cfg->section1.localIp.c_str();	
	if(cq->query(sid) == true){
		cq->sendConnInfo(self->sock,(struct sockaddr*)self->addr_cli);
		memset(buffer,0,sizeof(buffer));
		snprintf(buffer,256,"publish session_serverIp [%s][%s][%d][%d][%d]",\
			sid,xrdpip,w,h,bpp);
		log_stdout("Call run_string_cmd:%s\n",buffer);
		cq->runCommand(buffer);
	}
	
	delete cq;
	free(data);
	free(xrdpIp);
	free(sid);
	return 0;
}



void data_handle(inter_t *self){
	if(DKLJ(self) != 0){
		LJXX(self);
	}
}

#include "cJSON.h"

void sendJSONpakcet(int fd,struct sockaddr_in *addr){
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"hostname","192.168.0.101");
	cJSON_AddStringToObject(root,"username","administrator");
	cJSON_AddStringToObject(root,"password","000000");		
	cJSON_AddStringToObject(root,"port","3389");
	cJSON_AddStringToObject(root,"protocol","rdp");
	cJSON_AddStringToObject(root,"connect_path","/tmp/");
	cJSON_AddStringToObject(root,"domain","");
	cJSON_AddStringToObject(root,"initial-program","");
	cJSON_AddStringToObject(root,"security","tls");
	cJSON_AddStringToObject(root,"enable-drive","true");
	cJSON_AddStringToObject(root,"upside_clip_flag","true");
	cJSON_AddStringToObject(root,"downside_clip_flag","true");
	cJSON_AddStringToObject(root,"file_upside_clip_flag","true");
	cJSON_AddStringToObject(root,"file_downside_clip_flag","true");
	cJSON_AddStringToObject(root,"console","");
	cJSON_AddStringToObject(root,"mstsc","true");
	char *buffer = cJSON_Print(root);
	int len = strlen(buffer);
	sendto(fd,buffer,len,0,(struct sockaddr*)addr,sizeof(struct sockaddr_in));
	cJSON_free(buffer);
	cJSON_Delete(root);
}


int readSocket(int sock){
	 struct sockaddr_in addr_cli;
	 socklen_t addrlen = sizeof(addr_cli);
	 bzero(&addr_cli,addrlen);
	 char buffer[MAX_BUFFER_LEN];
	 bzero(buffer,sizeof(buffer));
	 int nRecv = recvfrom(sock,buffer,MAX_BUFFER_LEN,0,(struct sockaddr*)&addr_cli,&addrlen);
	 if(nRecv > 0){
	 	log_stdout("recv[%s:%d]:%s\n",inet_ntoa(addr_cli.sin_addr),ntohs(addr_cli.sin_port),buffer);
	 	inter_t in;
		in.addr_cli = &addr_cli;
	 	in.sock = sock;
		in.data = buffer;
		in.len = nRecv;		
		sendJSONpakcet(sock,&addr_cli);
	 	data_handle(&in);
		return 0;
	 }
	 return -1;
 }

