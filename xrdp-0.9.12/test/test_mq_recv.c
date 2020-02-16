#include "mq.h"
#include <poll.h>

int is_wait_read(int fd,int ms){	
	struct pollfd ep;
	ep.fd = fd;
	ep.events = POLLIN|POLLPRI;
	ep.revents = 0;
	int rv = poll(&ep,1,ms);
	printf("revents:%0x,%0x\n",ep.revents,POLLRDNORM);
	if(rv > 0)
		return 1;
	if(rv < 0)
		perror("poll");
	return 0;	
}

int main(int argc,char* argv[]){
	int msgid = mq_open("123123");
	if(msgid < 0){
		perror("mq_open");
		exit(0);
	}
	int rv;
	struct msgType data;
	while(1){
		printf("start recv:%d\n",msgid);
		rv = mq_recv(msgid,&data,sizeof(data),1);
		if(rv < 0){
			perror("mq_recv");
			break;
		}else{
			//printf("msg %d,%d,%d:%s\n",rv,data.sum,data.length,data.msg);
		}
	}
	return 0;
}
