#include "myxrdp_common.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

void makeDirectory(const char *path){
	if((strcmp(path,".") == 0) || (strcmp(path,"/")==0))
		return ;
	if(access(path,F_OK) == 0)
		return ;
	else{
		char *duppath = strdup(path);
		const char *dir_name = dirname(duppath);
		makeDirectory(dir_name);
		free(duppath);
	}
	if(mkdir(path,0766) < 0){
		perror("mkdir");
		exit(1);
	}
	return;
}  

int createFile(const char*filename,int mode){
	if(creat(filename,mode) < 0){
		if(errno == ENOENT){
			char *dup_filename=strdup(filename);
			char *dir = dirname(dup_filename);
			makeDirectory(dir);
			free(dup_filename);
			creat(filename,mode);
		}
		return false;
	}
	return true;
}

int isFileExisted(const char *pathname){
	if(access(pathname,F_OK) == 0){
		return true;
	}
	return false;
}

int isFileNull(const char *filename){
	FILE *fp = fopen(filename,"r");
	if(fp == NULL)
		return -1;	
	int val = 0;
	char ch = fgetc(fp);
	if(ch == EOF)
		val = 1;
	fclose(fp);
	return val;
}



void writeFile(const char*filename,const void *data,int size){
	FILE *fp = fopen(filename,"w");
	if(fp == NULL)
		return ;
	fwrite(data,size,1,fp);
	fclose(fp);
}

enum SessionType getSessionType(const char *session_name){
	if(session_name == NULL){
		errno = EINVAL;
		return SES_OTHER;
	}
	char pathname[BUFFERSIZE]={0};
	snprintf(pathname,sizeof(pathname),"%s/%s",SESSIONPATH,session_name);
	if(isFileExisted(pathname) == false){
		createFile(pathname,0664);
		return SES_MASTER;
	}
	else {
		if(isFileNull(pathname) == 0){
			return SES_OTHER;
		}else{
			writeFile(pathname,"000000",6);
			return SES_SLAVE;
		}
	}
	return SES_OTHER;
}



int create_fifo(const char *path){
	if(mkfifo(path,O_CREAT|O_EXCL) < 0){
		if(errno == EEXIST)
			return 0;
		if(errno == ENOENT){
			char *dup = strdup(path);
			const char *dir = dirname(dup);
			makeDirectory(dir);
			create_fifo(path);
			free(dup);
		}else{
			perror("mkfifo");
			return -1;
		}
	}
	return 0;
}

int open_fifo(const char *pathname,int mode){
	int fd=open(pathname,mode);
	if(fd==-1){
		perror("open");
		return (-1);	
	}
	return fd;
}

int myWrite(int fd,const void*data,int size){
	int rv;
	do{
		rv=write(fd,data,size);
	}while(rv<0 && (errno==EAGAIN ||errno==EINTR));
	return rv;
}

int myRead(int fd,void*data,int size){
	int rv;
	do{
		rv=read(fd,data,size);
	}while(rv<0 && (errno==EAGAIN ||errno==EINTR));
	return rv;
}

int is_fd_can_read(int fd, long usec){
	if(fd < 0 || fd > 1024){
		return 0;			
	}
	struct timeval timeOut;
	bzero(&timeOut,sizeof(timeOut));
	timeOut.tv_sec = usec/1000000;
	timeOut.tv_usec = usec%1000000;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);
	int rv=select(fd+1, &rfds, NULL, NULL, &timeOut);	
	if(rv == 1)
		return 1;
	return 0;

}

int is_fd_can_write(int fd, long usec){
	struct timeval timeOut;
	bzero(&timeOut,sizeof(timeOut));
	timeOut.tv_sec = usec/1000000;
	timeOut.tv_usec = usec%1000000;
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(fd,&wfds);
	int rv=select(fd+1,NULL,&wfds, NULL, &timeOut);	
	if(rv == 1)
		return 1;
	return 0;
}


void closeFd(int fd){
	if(fd >=0)
		close(fd);
}
void deleteFile(const char *filename){
	if(isFileExisted(filename))
		unlink(filename);
}
