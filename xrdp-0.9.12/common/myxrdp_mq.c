#include "myxrdp_mq.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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
	return msgrcv(msqid, msgp, size,type,IPC_NOWAIT);
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


#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int mq_can_recv(int fd,int sec){
	printf("1:%d\n",fd);
    fd_set rfds;
    struct timeval time;
	printf("2\n");
	memset(&time,0,sizeof(time));
	printf("3\n");

	time.tv_sec = sec;
	time.tv_usec = 0;
    int rv;
	printf("4\n");
    FD_ZERO(&rfds);
	printf("5\n");
    FD_SET(fd,&rfds);
	printf("6\n");
    rv = select(fd + 1, &rfds, 0, 0, &time);
    if (rv == 1)
    {
        return 1;
    }
    return 0;
}

void mq_clean(int msgid){
	msgctl(msgid,IPC_RMID,0);
}


#if 0
int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp,
				  int msgflg);
#endif


