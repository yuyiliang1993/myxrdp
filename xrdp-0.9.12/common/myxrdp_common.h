#ifndef __COMMON_H__
#define __COMMON_H__

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#define SESSIONPATH "/usr/local/tmp/xrdp"

enum SessionType{
	SES_MASTER=0x01,
	SES_SLAVE=0x02,
	SES_OTHER=0x03,
	SES_ERROR=0x04,
};


typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define BUFFERSIZE 1024
#define BUFFER_PACKET_SIZE 1024*16 //数据传输buffer大小
#define MAX_FDS_SELECT 1024
#define MAX_MASTER_WAIT_SEC 10
#ifndef __DEBUG_OUT__
#define __DEBUG_OUT__
#endif

//typedef unsigned short uint8_t;
typedef struct PacketInfo{
	uint16_t type;
	uint32_t sum;
	uint32_t length;
	char buffer[BUFFER_PACKET_SIZE];
}PacketInfo_t;

int gl_open_fifo(const char *pathname,int mode);

int gl_create_fifo(const char *path);

void gl_closeFd(int fd);

void gl_delete_file(const char *filename);
	
int gl_create_file(const char*filename,int mode);

int gl_file_existed(const char *pathname);
	
int gl_file_null(const char *filename);

int gl_fd_can_recv(int fd, int ms);

int gl_fd_can_send(int fd, int ms);

int is_fd_can_read(int fd,long usec);

int is_fd_can_write(int fd,long usec);

int gl_write(int fd,const void*data,int size);

int gl_read(int fd,void*data,int size);

int gl_file_append_line(const char*filename,const void *data,int size);

int gl_file_block_lock(const char *filename);

int gl_file_unlock_close(int fd);

typedef struct session_arg{
	int rdp5_performanceflags;
	int rdp_compression;
}session_arg_t;

typedef struct MyTransInfo{
  int session_type;
  int fifoFd[2];
  char filename1[256];
  char filename2[256];
  char filename3[256];
  struct ssl_tls *tls;
  int conn_status;
  int send_to_slave;
}MyTransInfo_t;



enum data_type{
	DATA_TYPE_RDP=0x01,
	DATA_TYPE_DISCONN=0x03,
};

enum CONN_STAT{
	CONNECTED=0x02,
	DISCONNECTED=0x04,
};

enum SNED_STAT{
	SEND_OFF=0x00,
	SEND_ON=0x01,
};
	
enum WORK_MODE{
	MODE_DIRECT_MENU=0x01,
	MODE_PROTOCOL_AGENT,
};

#endif
