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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "myxrdp_common.h"


int gl_strlen(const char *str)
{
    if (str == NULL){
        return 0;
    }

    return strlen(str);
}


int gl_file_existed(const char *pathname){
	if(access(pathname,F_OK) == 0){
		return true;
	}
	return false;
}


int gl_dir_existed(const char *dirname)
{
#if defined(_WIN32)
    return 0; // use GetFileAttributes and check return value
    // is not -1 and FILE_ATTRIBUTE_DIRECTORY bit is set
#else
    struct stat st;

    if (stat(dirname, &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    else
    {
        return 0;
    }
#endif
}

int	gl_create_dir(const char *dirname)
{
#if defined(_WIN32)
    return CreateDirectoryA(dirname, 0);
#else
    return mkdir(dirname, (mode_t) - 1) == 0;
#endif
}


int gl_create_path(const char *path)
{
    char *pp;
    char *sp;
    char *copypath;
    int status;

    status = 1;
    copypath = strdup(path);
    pp = copypath;
    sp = strchr(pp, '/');

    while (sp != 0)
    {
        if (sp != pp)
        {
            *sp = 0;

            if (!gl_dir_existed(copypath))
            {
                if (!gl_create_dir(copypath))
                {
                    status = 0;
                    break;
                }
            }

            *sp = '/';
        }

        pp = sp + 1;
        sp = strchr(pp, '/');
    }

    free(copypath);
    return status;
}


int gl_create_file(const char*filename,int mode){
	if(gl_file_existed(filename)){
		return true;
	}
	if(creat(filename,mode) < 0){
		int er = errno; 
		if( er == ENOENT){
			gl_create_path(filename);
			return gl_create_file(filename,mode);
		}
		return false;
	}
	return true;
}

int gl_file_null(const char *filename){
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


int gl_create_fifo(const char *path){
	if(mkfifo(path,O_CREAT|O_EXCL) < 0){
		if(errno == EEXIST)
			return 0;
		if(errno == ENOENT){
			char *dup = strdup(path);
			gl_create_dir(dirname(dup));
			gl_create_fifo(path);
			free(dup);
		}else{
			return -1;
		}
	}
	return 0;
}

int gl_open_fifo(const char *pathname,int mode){
	int fd = open(pathname,mode);
	if(fd == -1){
		perror("open");
		return (-1);	
	}
	return fd;
}

void gl_closeFd(int fd){
	if(fd >=0)
		close(fd);
}

void gl_delete_file(const char *filename){
	if(filename==NULL)
		return;
	if(gl_strlen(filename) == 0)
		return;
	if(gl_file_existed(filename))
		unlink(filename);
}


static int gl_select_valid(int fd){
	if(fd < 0 || fd >= FD_SETSIZE)
		return 0;
	return 1;
}

int gl_fd_can_recv(int fd, int ms)
{
	
    fd_set rfds;
    struct timeval time;
    int rv;
	if(!gl_select_valid(fd)){
		return 0;
	}
	memset(&time, 0, sizeof(time));
    time.tv_sec = ms / 1000;
    time.tv_usec = (ms * 1000) % 1000000;
	FD_ZERO(&rfds);
    FD_SET(((unsigned int)fd), &rfds);
	rv = select(fd + 1, &rfds, 0, 0, &time);
	if( rv > 0){
		return 1;
    }
    return 0;
}

int gl_fd_can_send(int fd, int ms)
{
    fd_set wfds;
    struct timeval time;
	if(!gl_select_valid(fd)){
		return 0;
	}
	memset(&time, 0, sizeof(time));
    time.tv_sec = ms / 1000;
    time.tv_usec = (ms * 1000) % 1000000;
    FD_ZERO(&wfds);
    FD_SET(((unsigned int)fd), &wfds);
    if(select(fd + 1, 0, &wfds, 0, &time) > 0){
		return 1;
    }
    return 0;
}

int gl_write(int fd,const void*data,int size){
	int rv;
	do{
		rv = write(fd,data,size);
	}while(rv<0 && (errno==EAGAIN ||errno==EINTR));
	return rv;
}

int gl_read(int fd,void*data,int size){
	int rv;
	do{
		rv=read(fd,data,size);
	}while(rv<0 && (errno==EAGAIN ||errno==EINTR));
	return rv;
}

int is_fd_can_read(int fd,long usec){
	struct timeval timeout;
SELECT:
	bzero(&timeout,sizeof(timeout));
	timeout.tv_sec = usec/1000000;
	timeout.tv_usec = usec%1000000;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);
	int rv;
	if((rv=select(fd+1, &rfds, NULL, NULL, &timeout)) < 0){
		if(errno == EINTR){
			goto SELECT;
		}
	}		
	return rv;	
}

int is_fd_can_write(int fd,long usec){
	struct timeval timeout;
SELECT:
	bzero(&timeout,sizeof(timeout));
	timeout.tv_sec = usec/1000000;
	timeout.tv_usec = usec%1000000;
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(fd,&wfds);
	int rv;
	if((rv=select(fd+1, 0, &wfds, NULL, &timeout)) < 0){
		if(errno == EINTR){
			goto SELECT;
		}
	}		
	return rv;	
}

