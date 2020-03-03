#ifndef __COMMON_H__
#define __COMMON_H__


#define SESSIONPATH "/usr/local/tmp/xrdp"

enum SessionType{
	SES_MASTER=1,
	SES_SLAVE,
	SES_OTHER,
	SES_ERROR,
};

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define BUFFERSIZE 1024

#define BUFFER_PACKET_SIZE 1024*16 //数据传输buffer大小

//typedef unsigned short uint8_t;
typedef struct PacketInfo{
	uint16_t type;
	uint32_t sum;
	uint32_t length;
	char buffer[BUFFER_PACKET_SIZE];
}PacketInfo_t;




int open_fifo(const char *pathname,int mode);

int create_fifo(const char *path);

void closeFd(int fd);

void deleteFile(const char *filename);

void makeDirectory(const char *path) ;
	
int createFile(const char*filename,int mode);

int isFileExisted(const char *pathname);
	
int isFileNull(const char *filename);
	
void writeFile(const char*filename,const void *data,int size);

int myWrite(int fd,const void*data,int size);

int myRead(int fd,void*data,int size);

int is_fd_can_read(int fd,long usec);

typedef struct session_arg{
	int rdp5_performanceflags;
	int rdp_compression;
}session_arg_t;

enum SessionType getSessionType(const char *session_name);

#endif
