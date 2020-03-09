#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include "log.h"
#include "xrdp_client_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h> //true /false
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "myxrdp_query.h"
#include "myxrdp_cjson.h"
#include "myxrdp_common.h"

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#define ITEM(r,v) cJSON_GetObjectItem((r),(v))


//return:need free
static char *packet_json_string(struct xrdp_client_info*info){
	cJSON *root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"type","YHXX");
	cJSON_AddStringToObject(root,"sessionId",info->username);
	cJSON_AddNumberToObject(root,"width",info->width);
	cJSON_AddNumberToObject(root,"height",info->height);
	cJSON_AddNumberToObject(root,"bpp",info->bpp);
	char *string = cJSON_Print(root);
	cJSON_Delete(root);
	return string;
}

static char * my_mem_copy(char *dest,int size,char *src){
	if(strlen(src) > size)
		return dest;
	memset(dest,0,size);
	memcpy(dest,src,strlen(src));
	return dest;
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

void conn_data_show(ConnInfo_t *conn){
	if(conn == NULL)
		return;
	log_message(LOG_LEVEL_DEBUG,"hostname:%s",conn->hostname);
	log_message(LOG_LEVEL_DEBUG,"username:%s",conn->username);
	log_message(LOG_LEVEL_DEBUG,"password:%s",conn->password);
	log_message(LOG_LEVEL_DEBUG,"port:%s",conn->port);
	log_message(LOG_LEVEL_DEBUG,"protocol:%s",conn->protocol);
	log_message(LOG_LEVEL_DEBUG,"connect_path:%s",conn->connect_path);
	log_message(LOG_LEVEL_DEBUG,"domain:%s",conn->domain);
	log_message(LOG_LEVEL_DEBUG,"initial-program:%s",conn->initial_program);
	log_message(LOG_LEVEL_DEBUG,"security:%s",conn->security_type);
	log_message(LOG_LEVEL_DEBUG,"console:%s",conn->console);
	log_message(LOG_LEVEL_DEBUG,"enable-drive:%d",conn->share_dask_flag);
	log_message(LOG_LEVEL_DEBUG,"upside_clip_flag:%d",conn->upside_clip_flag);
	log_message(LOG_LEVEL_DEBUG,"downside_clip_flag:%d",conn->downside_clip_flag);
	log_message(LOG_LEVEL_DEBUG,"file_upside_clip_flag:%d",conn->file_upside_clip_flag);
	log_message(LOG_LEVEL_DEBUG,"file_downside_clip_flag:%d",conn->file_downside_clip_flag);
	log_message(LOG_LEVEL_DEBUG,"mstsc:%d",conn->mstsc);
}

void conn_data_init(ConnInfo_t *conn){
	if(conn == NULL)
		return;
	char en = '\0';
	conn->hostname[0] = conn->username[0] = conn->password[0]=en;
	conn->port[0] = conn->protocol[0] = conn->connect_path[0]=en;
    conn->domain[0] = conn->initial_program[0] =conn->security_type[0]=en;
	conn->console[0] = en;
	conn->share_dask_flag=0;
    conn->upside_clip_flag=0;
	conn->downside_clip_flag=0;
	conn->file_upside_clip_flag=0;
	conn->file_downside_clip_flag=0;
    conn->mstsc=0;	
}


static int parse_connection(char *buffer,ConnInfo_t *conn){
	cJSON *root = cJSON_Parse(buffer);
	if(root == NULL)
		return false;
	value_string_update(ITEM(root,"hostname"),conn->hostname,sizeof(conn->hostname));

	value_string_update(ITEM(root,"username"),conn->username,sizeof(conn->username));
	
	value_string_update(ITEM(root,"password"),conn->password,sizeof(conn->password));
	
	value_string_update(ITEM(root,"port"),conn->port,sizeof(conn->port));

	value_string_update(ITEM(root,"protocol"),conn->protocol,sizeof(conn->protocol));

	value_string_update(ITEM(root,"connect_path"),conn->connect_path,sizeof(conn->connect_path));

	value_string_update(ITEM(root,"domain"),conn->domain,sizeof(conn->domain));

	value_string_update(ITEM(root,"initial-program"),conn->initial_program,sizeof(conn->initial_program));

	value_string_update(ITEM(root,"security"),conn->security_type,sizeof(conn->security_type));

	value_string_update(ITEM(root,"console"),conn->console,sizeof(conn->console));

	value_bool_update(ITEM(root,"enable-drive"),&conn->share_dask_flag);

	value_bool_update(ITEM(root,"upside_clip_flag"),&conn->upside_clip_flag);

	value_bool_update(ITEM(root,"downside_clip_flag"),&conn->downside_clip_flag);

	value_bool_update(ITEM(root,"file_upside_clip_flag"),&conn->file_upside_clip_flag);

	value_bool_update(ITEM(root,"file_downside_clip_flag"),&conn->file_downside_clip_flag);

	value_bool_update(ITEM(root,"mstsc"),&conn->mstsc);

	cJSON_Delete(root);
	return true;
	
}


static int udp_send_data(int sock,const char *data,int size){	
	struct sockaddr_in serv_addr;
	socklen_t addrlen = sizeof(serv_addr);
	bzero(&serv_addr,addrlen);
	serv_addr.sin_family= AF_INET;
	serv_addr.sin_port = htons(REDIS_CONN_SERVER_PORT);
	serv_addr.sin_addr.s_addr = inet_addr( REDIS_CONN_SERVER_IP);
	if(gl_fd_can_send(sock,200) <= 0)
		return false;
	if((sendto(sock,data,size,0,(struct sockaddr*)&serv_addr,addrlen)) < 0 ){
		return false;
	}
	return true;
}



static int udp_recv_data(int sock,char *buffer,int size){
	if(gl_fd_can_recv(sock,5*1000) <= 0)
		return false;
	if(recvfrom(sock,buffer,size,0,NULL,NULL) > 0){
		log_message(LOG_LEVEL_DEBUG,"udp_recv:%s\n",buffer);
		return true;
	}		
	return false;
}



static int query_conn_redis_server(QueryRedisData_t *query){
	if(query == NULL)
		return false;
	bool result = false;
	if(query->client_info==NULL || query->conn==NULL){
		return false;
	}
	my_mem_copy(query->conn->ssid,sizeof(query->conn->ssid),\
	query->client_info->username);
	
	char * packetstring = packet_json_string(query->client_info);
	if(packetstring == NULL){
		return false;
	}
	
	int sock = socket(AF_INET,SOCK_DGRAM,0);
	if(sock < 0){
		result = false;
		goto RET;
	}
	
	int sent = query->data_send(sock,packetstring,strlen(packetstring));
	if(sent == false){
		log_message(LOG_LEVEL_ERROR,"send packet failed:%s",packetstring);
		result = false;
		goto RET;
	}

#ifdef __DEBUG_OUT__	
	log_message(LOG_LEVEL_DEBUG,"send:%s",packetstring);
#endif

	char buffer[2048]={0};
	if(query->data_recv(sock,buffer,sizeof(buffer))){
		result = true;
		if(!parse_connection(buffer,query->conn)){
			result = false;
		}
	}
	
RET:
	cJSON_free(packetstring);
	if(sock >=0 )
		close(sock);
	return result;
}


QueryRedisData_t *qrd_create(ConnInfo_t*conn,struct xrdp_client_info*client_info){
	QueryRedisData_t *qrd = (QueryRedisData_t*)malloc(sizeof(QueryRedisData_t));
	qrd->data_recv = udp_recv_data;
	qrd->data_send = udp_send_data;
	qrd->query = query_conn_redis_server;
	qrd->client_info = client_info;
	qrd->conn = conn;
	return qrd;
}

void qrd_delete(QueryRedisData_t *obj){
	if(obj){
		free(obj);
	}
}
