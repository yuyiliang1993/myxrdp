#include "common.h"
#if 0
int main(int argc,char** argv)
{
	char buf_r[1024];
	int  fd;
	//int  nread;
	
	/* 创建管道 */
	const char *filename = "/usr/local/tmp/xrdp/123123.fifo";
	create_fifo(filename);
	
	//printf("Preparing for reading bytes...\n");
	
	memset(buf_r,0,sizeof(buf_r));
	
	/* 打开管道 */
	fd = open_fifo(filename,O_RDWR);
	PacketInfo_t pack;
	
	while(1)
	{
		memset(&pack,0,sizeof(pack));
		printf("input msg:");
		scanf("%s",pack.buffer);
		pack.type=1;
		pack.sum=strlen(pack.buffer);
		pack.length=strlen(pack.buffer);
		int sentBytes = myWrite(fd,&pack,sizeof(pack));
		printf("sentBytes %d,%d,%d,%d:\n%s\n",sentBytes,pack.type,pack.sum,pack.length,pack.buffer);
		//sleep(1);
	}
	close(fd);
	pause(); /*暂停，等待信号*/
	unlink(filename); //删除文件
	return 0;
}
#endif