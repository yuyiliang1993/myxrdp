#include "mq.h"
#include <poll.h>


int sendpacket(int fd,const char *data,int type,int sum,int len){
	struct msgType tdata;
	printf("sum-%d,len-%d:%s\n",sum,len,data);
	tdata.type = type;
//	tdata.sum = sum;
//	tdata.length = len;
	memcpy(tdata.msg,data,len);
//s	printf("size:%ld\n",sizeof(tdata));
	return mq_write(fd,&tdata,sizeof(tdata));
}


int senddata(int fd,const char *data,int len)
{
	struct msgType tmp;
	int msgBufSize = sizeof(tmp.msg);
	if(len <= msgBufSize){
		return sendpacket(fd,data,1,len,len);
	}else{
		
	}
	return -1;
}

int main(int argc,char *argv[]){
	
	int msgid = mq_create("123123");
	if(msgid < 0 ){
		perror("mq_create");
		exit(0);
	}
	int sendbytes = 0;
	char buf[100];
	while(1){		
#if 1
		printf("input:%d\n",msgid);
		scanf("%s",buf);
		printf("buf:%s\n",buf);
		sendbytes = senddata(msgid,buf,strlen(buf));
		printf("write:%d,%ld\n",sendbytes,strlen(buf));
		if(sendbytes == -1){
		//	perror("mq_write");
			break;
		}
#endif
	}
	msgctl(msgid,IPC_RMID,0);
	return 0;

}

