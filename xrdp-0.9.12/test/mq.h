#ifndef __MQ_H__
#define __MQ_H__
#include "common.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgType{	
	long int type;
	char msg[BUFFERSIZE];
	//int sum;
	//int length;
	
};


int mq_create(const char *sessionid);

int mq_open(const char *sessionid);

int mq_write(int msqid,const void *msgp,size_t size);

int mq_recv(int msqid,void *msgp,size_t size,long int type);

int mq_can_recv(int msgid,long usec);

int mq_timeout_recv(int msqid,void *msgp,size_t size,long int type,int ms);

#endif

