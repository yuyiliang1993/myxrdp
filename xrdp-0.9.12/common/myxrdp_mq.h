#ifndef __MQ_H__
#define __MQ_H__
#include "myxrdp_common.h"
#define MAX_MSG_BUF_SIZE 1024
struct msgType{	
	long int type;
	int length;
	char msg[MAX_MSG_BUF_SIZE];
};

int mq_create(const char *sessionid);

int mq_open(const char *sessionid);

int mq_write(int msqid,const void *msgp,int size);

int mq_recv(int msqid,void *msgp,int size,long int type);

int mq_can_recv(int msgid,int sec);

void mq_clean(int msgid);

int mq_timeout_recv(int msqid,void *msgp,int size,long int type,int ms);	
#endif

