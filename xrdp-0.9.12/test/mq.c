#include "mq.h"
int mq_create(const char *sessionid){
	char pathName[BUFFERSIZE];
	snprintf(pathName,sizeof(pathName),"%s/%s",SESSIONPATH,sessionid);
	key_t key = ftok(pathName,10);
	return msgget(key,0666 | IPC_CREAT);
}

int mq_open(const char *sessionid){
	char pathName[BUFFERSIZE];
	snprintf(pathName,sizeof(pathName),"%s/%s",SESSIONPATH,sessionid);
	key_t key = ftok(pathName,10);
	return msgget(key,0666);
}

int mq_write(int msqid,const void *msgp,size_t size){
	return msgsnd(msqid,msgp, size,0);
}
int mq_recv(int msqid,void *msgp,size_t size,long int type){
	return msgrcv(msqid, msgp, size,type,0);
}

int mq_timeout_recv(int msqid,void *msgp,size_t size,long int type,int ms){
	int rv;
	int count = 0;
	do{
		if((rv=msgrcv(msqid, msgp, size,type,IPC_NOWAIT)) == -1){
			++count;
			usleep(1000);//1ms
		}else {
			break;			
		}	
	}while(count < ms);
	return  rv;
}

int mq_can_recv(int msgid,long usec){
	struct timeval t;
	memset(&t,0x00,sizeof(t));
	t.tv_sec = usec/1000000;
	t.tv_usec = usec%1000000;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(msgid,&rfds);
	int rv = select(msgid+1,&rfds,NULL,NULL,&t);
	if(rv > 0)
		return 1;
	return 0;
}

int is_fd_wait_read(int msgid,long usec){
	struct timeval t;
	memset(&t,0x00,sizeof(t));
	t.tv_sec = 0;
	t.tv_usec = 100;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(msgid,&rfds);
	int rv = select(msgid+1,&rfds,NULL,NULL,&t);
	if(rv > 0)
		return 1;
	return 0;
}


#if 0
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
				  int msgflg);
#endif


