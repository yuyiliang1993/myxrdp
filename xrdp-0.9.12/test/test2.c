#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h> // 提供flock()函数，从#include可以看出，它是一个系统调用，而不是一个库函数
#include <unistd.h>
#define SESSIONPATH "/usr/local/tmp/xrdp"


int gl_file_block_lock(const char *filename){

	char buf[512]={0};
	snprintf(buf,sizeof(buf),"%s/%s",SESSIONPATH,filename);
	printf("filename:%s\n",buf);
	int fd = open(buf, O_RDONLY|O_CREAT);
	if (-1 == fd){
		perror("open");
		return -1;
	}
    if (flock(fd, LOCK_EX) < 0){
        perror("flock");
		return -1;
	}
	return fd;
}

int gl_file_unlock_close(int fd){
	if(fd < 0)
		return 1;
	if (-1 == flock(fd, LOCK_UN)){
		perror("fulock");
	}
	close(fd);
	return 0;
}



int main(int argc, char *argv[])
{ 
	printf("%d try to get lock\n", getpid());
	int fd = gl_file_block_lock("123456.lock");
    printf("%d locked now, enter any key to continue ...\n", getpid());
    getchar();
    printf("%d prepare to release lock\n", getpid());
	gl_file_unlock_close(fd);
    // 释放锁
    printf("lock was released now\n");
}
