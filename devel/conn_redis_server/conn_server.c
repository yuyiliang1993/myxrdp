#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "myredis.h"
#include "mysock.h"
#define SERVER_PORT 11190

void server_work_start(int sock)
{
	fd_set rfds;
	while(1){
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		if(select(sock+1,&rfds,NULL,NULL,NULL) < 0){
			if(errno == EINTR){
				continue;
			}
			perror("select");
			break;
		}
		if(FD_ISSET(sock,&rfds)){
			readSocket(sock);
		}
	}
}

void daemon_init(void)
{
	pid_t pid; 
	if((pid= fork()) < 0){
		perror("fork");
		exit(1);
	}
	
	if(pid > 0){
		exit(0);//父进程退出
	}
	setsid();//获取新得session
	chdir("/");//改变工作目录
	umask(0);//设置新的掩码
	int fd = open("/dev/null",O_RDWR);
	if(fd > 0){
		int i = 0;
		for(;i<3;++i){
			dup2(i,fd);
			close(i);
		}
	}
}

int main(int argc,char *argv[])
{
	if(argc>=2 && strcmp(argv[1],"-d")==0){
		daemon_init();
	}
	int sock;
	if((sock = openUdpBindSocketIpv4(SERVER_PORT)) < 0){
		exit(0);
	}
	fprintf(stderr,"connection server starting:%d\n",SERVER_PORT);
	server_work_start(sock);
	close(sock);
	return 0;
}

