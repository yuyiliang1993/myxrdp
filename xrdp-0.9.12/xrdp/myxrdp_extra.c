#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif


#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <sys/types.h>
#include <ctype.h>

//file stat
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

//socket
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//sem
#include <semaphore.h>

#include <fcntl.h>
#include <unistd.h>



#include "myxrdp_common.h"
#include "xrdp.h"
#include "myxrdp.h"

#define if_free(ptr) if(ptr){free(ptr);}

static char *trimStringSpace(char *str){
	if(str == NULL || strlen(str) == 0)
		return str;
	char *left = str;
	char *right = str+strlen(str)-1;
	while(left < right && isspace((int)*left))
		++left;
	while(right > left && isspace((int)*right))
		*right-- = 0x00;
	return left;
}

/*
*filename:文件名
*lineNum:1,2,3,4...
*line:缓存buf
*size:line大小
*/
static char * getLineFromFile(const char*filename,int lineNum,char *line,int size){
	if(line==NULL ||size<=0||\
		filename == NULL ||lineNum <=0){
		errno = EINVAL;
		return NULL;
	}
	int count = 0;	
	char *result = NULL;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		return NULL;
	}
	while(1){
		memset(line,0,size);
		if(fgets(line,size,fp) == NULL){
			if(feof(fp) || ferror(fp)){
				break;
			}
			continue;
		}
		if(++count == lineNum){
			result = trimStringSpace(line);
			break;
		}
	}
	fclose(fp);
	return result;	
}


int extra_process_stop(long pid){
	log_message(LOG_LEVEL_DEBUG,"kill pid:%ld",pid);
	return kill((pid_t)pid,SIGKILL);
}

xrdpExtraConfig_t *extra_cfg_create(void){
	xrdpExtraConfig_t *ptemp = (xrdpExtraConfig_t*)malloc(sizeof(xrdpExtraConfig_t));
	ptemp->localIp = NULL;
	ptemp->directMenuPathName = NULL;
	ptemp->directMenuFlag = NULL;
	ptemp->x11PathName = NULL;
	ptemp->OCRFlag = NULL;
	ptemp->connRedisIp = NULL;
	ptemp->connRedisPort = NULL;
	ptemp->userPoolIp = NULL;
	ptemp->userPoolPort = NULL;
	ptemp->CssProgramName = NULL;
	ptemp->XnestProgramName=NULL;
	return ptemp;
}

void extra_cfg_delete(xrdpExtraConfig_t *cfg){
	if(cfg){
		if_free(cfg->localIp);
		if_free(cfg->directMenuFlag);
		if_free(cfg->directMenuPathName);
		if_free(cfg->x11PathName);
		if_free(cfg->OCRFlag);
		if_free(cfg->connRedisIp);
		if_free(cfg->connRedisPort);
		if_free(cfg->userPoolIp);
		if_free(cfg->userPoolPort);
		if_free(cfg->XnestProgramName);
		if_free(cfg->CssProgramName);
	}
	if_free(cfg);
}


//malloc
//you need free
static char *extra_trimSpace(const char *str){
	if(str == NULL)
		return NULL;
	if(strlen(str) == 0)
		return NULL;
	const char *start = str;
	const char *end = str+strlen(str)-1;
	while(start < end && isspace((int)*start))
		++start;
	while(end > start && isspace((int)*end))
		--end;
	if(start >= end)
		return NULL;
	
	int len = end-start+1;
	char * mem = (char*)malloc(sizeof(char)*len+1);
	if(mem == NULL)
		return NULL;
	memcpy(mem,start,len);
	mem[len]=0x00;
	return mem;
}


//strdup
//linenum > 0
char *extra_getLineStringFromFile(const char*filename,int lineNum){
	if(filename == NULL ||lineNum <=0)
		return NULL;
	char buffer[1024];
	int count = 0;	
	char *result = NULL;
	FILE *fp = fopen(filename,"r");
	if(fp == NULL){
		perror("fopen");
		return NULL;
	}
	while(1){
		memset(buffer,0,sizeof(buffer));
		if(fgets(buffer,sizeof(buffer),fp) == NULL){
			if(feof(fp)){
				break;
			}
			else if(ferror(fp)){
				perror("fgets");
				break;
			}
			continue;
		}
		if(buffer[0]==0x00 || buffer[0] == '#')
			continue;
		if(++count == lineNum){
			result = extra_trimSpace(buffer);
			break;
		}
	}
	fclose(fp);
	return result;	
}


static bool extra_configStringParse(/*const*/char *data,xrdpExtraConfig_t *cfg){
	if(strlen(data) == 0)
		return false;
	char *pstr = strstr(data,"=");
	if(pstr == NULL)
		return false;
	const char *start = data;
	char *end = data+strlen(data)-1;
	char *temp = pstr+1;
	while(pstr>data &&(*pstr == '=' ||\
		isspace((int)*pstr)))
		*pstr-- = 0x00;
	while(temp<end &&(*temp == '=' ||\
		isspace((int)*temp)))
		++temp;

	if(strcasecmp(start,"LocalIp") == 0){
		cfg->localIp = strdup(temp);
	}
	else if(strcasecmp(start,"DMFlag") == 0){
		cfg->directMenuFlag = strdup(temp);
	}
	else if(strcasecmp(start,"DMPathname") == 0){
		cfg->directMenuPathName = strdup(temp);
	}	
	else if(strcasecmp(start,"X11Pathname") == 0){
		cfg->x11PathName = strdup(temp);
	}
	else if(strcasecmp(start,"OCRFlag") == 0){
		cfg->OCRFlag = strdup(temp);
	}
	else if(strcasecmp(start,"ConnRedisIp") == 0){
		cfg->connRedisIp = strdup(temp);
	}
	else if(strcasecmp(start,"ConnRedisPort") == 0){
		cfg->connRedisPort = strdup(temp);
	}
	else if(strcasecmp(start,"UserPoolIp") == 0){
		cfg->userPoolIp = strdup(temp);
	}
	else if(strcasecmp(start,"UserPoolPort") == 0){
		cfg->userPoolPort = strdup(temp);
	}	
	else if(strcasecmp(start,"CssProgramName") == 0){
		cfg->CssProgramName = strdup(temp);
	}
	else if(strcasecmp(start,"XnestProgramName") == 0){
		cfg->XnestProgramName = strdup(temp);
	}	
	return true;
}


static void test_show(xrdpExtraConfig_t*p){
	printf("LocalIp:%s\n",p->localIp);
	printf("DMFlag:%s\n",p->directMenuFlag);
	printf("DMPathname:%s\n",p->directMenuPathName);
	printf("X11Pathname:%s\n",p->x11PathName);
	printf("OCRFlag:%s\n",p->OCRFlag);
	printf("ConnRedisIp:%s\n",p->connRedisIp);
	printf("ConnRedisPort:%s\n",p->connRedisPort);
	printf("UserPoolIp:%s\n",p->userPoolIp);
	printf("UserPoolPort:%s\n",p->userPoolPort);
	printf("XnestProgramName:%s\n",p->XnestProgramName);
	printf("CssProgramName:%s\n",p->CssProgramName);
}


int extra_read_config(xrdpExtraConfig_t *cfg,const char *filename){
	if(cfg==NULL && filename==NULL){
		return false;
	}
	FILE *fp = fopen(filename,"r");		
	if(fp == NULL){
		return false;
	}
	char buffer[1024]={0};
	while(!feof(fp) && !ferror(fp)){
		bzero(buffer,sizeof(buffer));
		if(fgets(buffer,1024,fp) == NULL)
			continue;
		char * const data = extra_trimSpace(buffer);
		if(data == NULL)
			continue;
		extra_configStringParse(data,cfg);
		if_free(data);
	}
	test_show(cfg);
	fclose(fp);
	return true;
}



void extra_thread_sleep(unsigned long usec){
	struct timeval timeOut;
	bzero(&timeOut,sizeof(timeOut));
	timeOut.tv_sec = usec/1000000;
	timeOut.tv_usec = usec%1000000;
	select(0, NULL, NULL, NULL, &timeOut);
}

int extra_isFileChanged(const char *filename,long lastModTime,int max_sec){
	struct stat s;
	long prevtime = lastModTime;
	int cont = 0;
	while(stat(filename,&s) != -1){	
		long mt = s.st_mtime;
		if(prevtime == 0L){
			prevtime = mt;
			continue;
		}
		if(prevtime == mt){
			extra_thread_sleep(100000);//100ms
			++cont;
			if(cont/10 >= max_sec)
				return 0; //timeout
		}else{
			return 1;
		}
	}
	return (-1);
}



long extra_getPidFromFile(const char *filename,int linenum){
	int pid = 0L;
	char *data = extra_getLineStringFromFile(filename,linenum);
	if(data){
		pid = atoi(data);
		free(data);
	}
	return pid;
}


//3th lines for data
int extra_getDirectMenuTempSidFromFile(char *dstBuf,int size,const char *filename)
{
	if(dstBuf==NULL||size<= 0||filename==NULL){
		errno = EINVAL;
		return (-1);
	}
	char buffer[1024]={0};
	char *line = getLineFromFile(filename,3,buffer,sizeof(buffer));
	if(line && strlen(line)<size){
		strcpy(dstBuf,line);
		trimStringSpace(dstBuf);
		return 0;
	}
	return (-1);
}

//4th lines for data
long extra_getDirectMenuPidFromFile(const char *filename)
{
	if(filename == NULL){
		errno = EINVAL;
		return 0;
	}
	char buf[1024]={0};
	char *line = getLineFromFile(filename,4,buf,sizeof(buf));
	if(line == NULL)
		return 0;
	return atol(line);
}


//return value need free
int extra_getSessionIdFromFile(char *dest,int size,const char *filename)
{
	if(filename==NULL)
		return 0;
	char buf[1024]={0};
	char *line = getLineFromFile(filename,1,buf,sizeof(buf));
	if(line == NULL)
		return (-1);
	
	size_t len = strlen(line);
	if(len >= size)
		return (-1);
	strcpy(dest,line);
	dest[len] = 0x00;
	return 0;
}

long extra_getLastModTime(const char *filename){
	if(filename == NULL)
		return 0;
	struct stat s;
	if(stat(filename,&s) != -1){
		return s.st_mtime;
	}
	return 0;
}

int extra_getUserInfoFromPool(char *username,char *passwd,const char *ip,int port)
{
	if(username == NULL || passwd == NULL){
		fprintf(stderr,"Argments username/passwd is NULL\n");
		return false;
	}

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		perror("socket");
		return false;
	}
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=inet_addr(ip);
	const char *data = "$GET USER\n";
	size_t len = strlen(data);
	int sent=sendto(sock,data,len,0,(struct sockaddr*)&addr,addrlen);
	if(sent <= 0){
		perror("sendto");
		close(sock);
		return false;
	}
	if(gl_fd_can_recv(sock,5000)){//5 s
		char buff[512];
		memset(buff,0,sizeof(buff));
		int bytes = recvfrom(sock,buff,512,0,NULL,NULL);
		if(bytes > 0){
			fprintf(stdout,"Recv:%s\n",buff);
			if(strncmp(buff,"$ANS USER",strlen("$ANS USER")) == 0){
				char *tok = NULL;
				int k = 0;
				for(tok=strtok(buff," ");tok!=NULL;tok = strtok(NULL," ")){
					++k;
					if(k == 3){
						strcpy(username,tok);
					}
					if(k == 4){
						strcpy(passwd,tok);
					}
				}
				if(k >= 4){//complete parse return true
					close(sock);
					return true;
				}
			}
			else {
				fprintf(stderr,"Not a valid protocol\n");
			}
			close(sock);
			return false;
		}
	}
	else {
		fprintf(stderr,"Waited server timeout\n");
	}
	close(sock);
	return false;
}


int extra_returnUserInfoToPool(xrdpExtraConfig_t *conf,const char *username,const char*password)
{
	const char *address = "0.0.0.0";
	int port = 10001;
	if(conf){
		if(conf->userPoolIp)
			address = conf->userPoolIp;
		if(conf->userPoolPort)
			port = atoi(conf->userPoolPort);
	}
	
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		perror("socket");
		return false;
	}	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(address);

	char data[512];
	snprintf(data,512,"$RET USER %s %s\n",username,password);
	size_t len = strlen(data);

	int sent = sendto(sock,data,len,0,(struct sockaddr*)&addr,addrlen);
	if(sent <= 0){
		perror("sendto");
		close(sock);
		return false;
	}
	log_message(LOG_LEVEL_DEBUG,"ReturnUserInfoToPool:%s",data);
	close(sock);
	return true;
}
