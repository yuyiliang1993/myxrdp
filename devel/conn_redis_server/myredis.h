#ifndef __MY_REDIS_H
#define __MY_REDIS_H
#include<hiredis/hiredis.h>
#define REDIS_CONN_IP "192.168.1.93"
#define REDIS_CONN_PORT 6379
#define REDIS_PASSWORD "123456"
redisContext *cm_redisConnect(const char * ip, int port,const char *password);
int my_request_redis(const char *sid,const char*xrdpip,int sock,struct sockaddr_in *addr_cli,int w,int h,int bpp);
int pull_dis_conn_redis(const char*sid);

#include<string>
#include<vector>
#include<map>

struct ConnectInfo{
	std::string hostname;
	std::string username;
	std::string password;
	std::string port;
	std::string protocol;
	std::string connect_path;
	std::string domain;
	std::string initial_program;
	std::string security_type;
	std::string share_dask_flag;
	std::string upside_clip_flag;
	std::string downside_clip_flag;
	std::string file_upside_clip_flag;
	std::string file_downside_clip_flag;
	std::string console;
	std::string mstsc;
};

typedef struct ConnectInfo ConnectInfo_t;

#define DEF_CONFIG_FILENAME "/etc/xrdp/conn_redis.conf"

struct InData{
	const char *sid;
	const char*localIp;
	int w;
	int h;
	int bpp;
	int sock;
	struct sockaddr_in *addr;
};

class QueryRedis{
public:
	virtual ~QueryRedis(){
		}
	virtual bool query(const std::string sId)=0;
	virtual bool sendConnInfo(int sock,struct sockaddr*addr)=0;
	virtual void runCommand(std::string cmd)=0;
private:	
};

class ConfigIn;
#include <json/json.h>

class ClusterQueryRedis:public QueryRedis
{
public:
	ClusterQueryRedis();
	virtual ~ClusterQueryRedis();
	virtual bool query(const std::string sId);
	virtual bool sendConnInfo(int sock,struct sockaddr*cli_addr);
	virtual void runCommand(std::string cmd);
	
private:
	ClusterQueryRedis(const ClusterQueryRedis&);
	ClusterQueryRedis&operator=(const ClusterQueryRedis &);
	bool request_connection_session(redisContext *c,std::string sid);
	bool loopQuery();
	std::string sid;
	ConfigIn *cfg;
	Json::Value root;
	redisContext *c;
}; 

class ConfigIn{
public:
	static ConfigIn *getStance(std::string filename);
	static ConfigIn *getStance();
	bool isValid();
	struct global{
		std::string password;
		std::string cluster_flag;
		std::string localIp;
	}section1;
	struct normal{
		std::string redisIp;
		std::string redisPort;
	}section2;
	
	struct cluster{
		std::map<std::string,std::string> m_mp;
	}section3;
private:
	ConfigIn(std::string filename);
	ConfigIn(const ConfigIn &);
	ConfigIn &operator=(const ConfigIn &);
	bool doingReadFile(std::string &filename);
	std::string getString(const char *buf);
	bool done;
};



class SingleQueryRedis:public QueryRedis
{
public:
	virtual bool query(std::string sId);
	virtual bool sendConnInfo(int sock);
private:
	
}; 
	
#endif
