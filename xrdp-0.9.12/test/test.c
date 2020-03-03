#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#include "cJSON.h"
#define ITEM(r,v) cJSON_GetObjectItem((r),(v))

static char * my_mem_copy(char *dest,int size,char *src){
	if(strlen(src) > size)
		return dest;
	memset(dest,0,size);
	memcpy(dest,src,strlen(src));
}

static void value_string_update(cJSON *item,char *pbuf,int size){
	if(item && item->type == cJSON_String){
		my_mem_copy(pbuf,size,item->valuestring);
	}	
}

static void value_bool_update(cJSON *item,int *pnum){
	if(item && item->type == cJSON_String){
		if(strcmp(item->valuestring,"true") == 0)
			*pnum = true;
	}		
}

void test2(){
	cJSON *root = cJSON_CreateObject();
	cJSON_AddStringToObject(root,"username","yuliang");
	cJSON_AddStringToObject(root,"password","123456");
	cJSON *content = cJSON_CreateObject();
	cJSON_AddStringToObject(content,"username","yuliang");
	cJSON_AddStringToObject(content,"password","123456");
	cJSON_AddStringToObject(root,"value","true");
	
	cJSON_AddItemToObjectCS(root,"content",content);
	char *str = cJSON_Print(root);
	cJSON_Delete(root);
	printf("%s\n",str);
	cJSON *item = NULL;
	cJSON *m_root = cJSON_Parse(str);



	char username[256]={0};
	value_string_update(ITEM(m_root,"usernam"),username,256);

	char password[256]={0};
	value_string_update(ITEM(m_root,"password"),password,256);

	int value = false;
	value_bool_update(ITEM(m_root,"valu"),&value);
	
	printf("%s\n",username);
	printf("%s\n",password);
	printf("%d\n",value);

	//cJSON_Delete(item);
	cJSON_Delete(m_root);
	
	cJSON_free(str);

}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static int gl_fd_select_valid(int fd){
	if(fd < 0 || fd >= FD_SETSIZE)
		return 0;
	return 1;
}

int gl_fd_can_recv(int fd, int ms)
{
	
    fd_set rfds;
    struct timeval time;
    int rv;
	if(!gl_fd_select_valid(fd)){
		printf("gl_fd_select_valid\n");
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
	if(rv < 0)
		perror("select");
    return 0;
}



int
g_directory_exist(const char *dirname)
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



/*****************************************************************************/
/* returns boolean */
int
g_create_dir(const char *dirname)
{
#if defined(_WIN32)
    return CreateDirectoryA(dirname, 0); // test this
#else
    return mkdir(dirname, (mode_t) - 1) == 0;
#endif
}

/*****************************************************************************/
/* will try to create directories up to last / in name
   example /tmp/a/b/c/readme.txt will try to create /tmp/a/b/c
   returns boolean */
int
g_create_path(const char *path)
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

            if (!g_directory_exist(copypath))
            {
                if (!g_create_dir(copypath))
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




int main(int argc,char *argv[]){
	//g_create_path("/home/test/123/");
	//printf("%d\n", FD_SETSIZE);
	while(1){
		if(gl_fd_can_recv(0,1000) == 0)
			printf("timeout:%ld\n",time(NULL));
	}
	return 0;
}
