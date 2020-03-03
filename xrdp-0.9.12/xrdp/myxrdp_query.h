#ifndef __MYXRDP_QUERY__
#define __MYXRDP_QUERY__
typedef struct ConnInfo{
	char hostname[256];
	char username[256];
	char password[256];
	char port[20];
	char protocol[128];
	char connect_path[256];
	char domain[256];
	char initial_program[256];
	char security_type[20];
	int share_dask_flag;
	int upside_clip_flag;
	int downside_clip_flag;
	int file_upside_clip_flag;
	int file_downside_clip_flag;
	char console[256];
	int mstsc;
	char ssid[256];
}ConnInfo_t;

struct xrdp_client_info;

typedef struct QueryRedisData{
	ConnInfo_t *conn;
	struct xrdp_client_info *client_info;
	int (*data_send)(int sock,const char *data,int size);
	int (*data_recv)(int sock,char *data,int size);
	int (*query)(struct QueryRedisData *query);
}QueryRedisData_t;

#ifndef REDIS_CONN_SERVER_IP
#define REDIS_CONN_SERVER_IP "127.0.0.1"
#endif

#ifndef REDIS_CONN_SERVER_PORT
#define REDIS_CONN_SERVER_PORT 11190
#endif

QueryRedisData_t *qrd_create(ConnInfo_t*conn,struct xrdp_client_info*client_info);

void qrd_delete(QueryRedisData_t *obj);

void conn_data_show(ConnInfo_t *conn);

void conn_data_init(ConnInfo_t *conn);

#endif
