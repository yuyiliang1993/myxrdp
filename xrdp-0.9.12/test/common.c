#include "common.h"

int isFileExisted(const char *pathname){
	if(access(pathname,F_OK) == 0){
		return true;
	}
	return false;
}

int getSessionType(const char *name){
	return 0;
}

int createFile(const char*filename,int mode){
	if(creat(filename,mode) < 0)
		return false;
	return true;
}

void makeDirectory(const char *path){
	if((strcmp(path,".") == 0) || (strcmp(path,"/")==0))
		return ;
	if(access(path,F_OK) == 0)
		return ;
	else{
		char *duppath = strdup(path);
		const char *dir_name = dirname(duppath);
		printf("dir_name:%s\n",dir_name);
		makeDirectory(dir_name);
		free(duppath);
	}
	if(mkdir(path,0766) < 0){
		perror("mkdir");
		exit(1);
	}
	return;
}

void test_makeDirectory(){
	char *test_path = "/home/tt/ttt";
	makeDirectory(test_path);
}

int test1(){
	makeDirectory("/var/xrdp");
	const char *filename = "/var/xrdp/test.txt";
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

int main(int argc,char *argv[]){
	test1();
//	test_makeDirectory();
	return 0;
}
