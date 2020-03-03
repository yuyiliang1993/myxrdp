#include "common.h"
#if 0
int isFileExisted(const char *pathname){
	if(access(pathname,F_OK) == 0){
		return true;
	}
	return false;
}

void makeDirectory(const char *path){
	if((strcmp(path,".") == 0) || (strcmp(path,"/")==0))
		return ;
	if(isFileExisted(path))
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



void writeFile(const char*filename,void *data,int size){
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
	printf("pathname:%s\n",pathname);
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
		if(errno != EEXIST){
			perror("mkfifo");
			return 1;
		}
	}
	return 0;
}

int open_fifo(const char *pathname,int mode){
	int fd = open(pathname,mode);
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

void test_makeDirectory(){
	char *test_path = "/home/tt/ttt";
	makeDirectory(test_path);
}

int test1(){
	makeDirectory(SESSIONPATH);
	char filename[1024];
	snprintf(filename,1024,"%s/%s",SESSIONPATH,"123456");
	//const char *filename = "/var/xrdp/test.txt";
	if(isFileExisted(filename) == false){
		printf("%s is not existed\n",filename);
		if(createFile(filename,0766) == true){
			printf("create %s success.\n",filename);
		}else{
			printf("create %s failed:%s.\n",filename,strerror(errno));
		}	
	}else{
		printf("%s is existed.\n",filename);
	}
	return 0;
}


void test_isFileNull(){
	int ret = isFileNull("test.txt");
	printf("test.txt:%d\n",ret);
	writeFile("test.txt","test",4);
	ret = isFileNull("test.txt");
	printf("test.txt:%d\n",ret);
}

void test_getSessionType(){
	int ret = getSessionType("123123");
	printf("ret:%d\n",ret);

	ret = getSessionType("123123");
	printf("ret:%d\n",ret);

	ret = getSessionType("123123");
	printf("ret:%d\n",ret);

	ret = getSessionType("123123");
	printf("ret:%d\n",ret);
}

int main(int argc,char *argv[]){

	test_getSessionType();

	return 0;
}
#endif
