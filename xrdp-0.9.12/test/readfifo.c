#include "common.h"
int main(){
	char buf_r[1024];
	int  fd;
	//int  nread;
		
	/* 创建管道 */
	const char *filename = "test1.fifo";
	create_fifo(filename);
		
	//printf("Preparing for reading bytes...\n");
		
	memset(buf_r,0,sizeof(buf_r));
		
	/* 打开管道 */
	fd = open_fifo(filename,O_RDWR);
	PacketInfo_t pack;
		
	while(1)
	{
		memset(&pack,0,sizeof(pack));
		int rcvBytes = myRead(fd,&pack,sizeof(pack));
		printf("rcvBytes %d,%d,%d,%d:\n%s\n",rcvBytes,pack.type,pack.sum,pack.length,pack.buffer);
		//sleep(1);
	}
	close(fd);
	unlink(filename); //删除文件
	return 0;

}
