#include "myredis.h"
#include "public.h"

#include <fstream>
#include<cstring>
#include<cctype>
#include<mutex>

static void check_conn_value(ConnectInfo_t &conn)
{
	if(conn.username.length() == 0){
		conn.username = "null";
	}

	if(conn.hostname.length() == 0){
		conn.hostname = "0.0.0.0";
	}

	if(conn.port.length() == 0){
		conn.port = "null";
	}

	if(conn.password.length() == 0){
		conn.password = "null";
	}

	if(conn.protocol.length() == 0){
		conn.protocol = "null";
	}

	if(conn.connect_path.length()==0){
		conn.connect_path = "/tmp/";
	}
	
	if(conn.domain.length() == 0){
		conn.domain = "null";
	}
	
	if(conn.initial_program.length() == 0){
		conn.initial_program = "null";

	}
	
	if(conn.security_type.length()==0){
		conn.security_type = "rdp";
	}
	
	if(conn.share_dask_flag.length() == 0){
		conn.share_dask_flag="false";
	}
	if(conn.upside_clip_flag.length() == 0){
		conn.upside_clip_flag="false";
	}
	
	if(conn.downside_clip_flag.length() ==0){
		conn.downside_clip_flag="false";
	}
	
	if(conn.file_upside_clip_flag.length() == 0){
		conn.file_upside_clip_flag="false";
	}
	if(conn.file_downside_clip_flag.length() == 0){
		conn.file_downside_clip_flag="false";	
	}
	
	if(conn.console.length() ==0){
		conn.console = "false";
	}
	
	if(conn.mstsc.length() == 0){
		conn.mstsc = "false";
	}

}


static bool GetConnectInfo(Json::Value &root,ConnectInfo_t &conn)
{
	//解析
	if(root["hostname"].isNull() || !root["hostname"].isString()){
		conn.hostname="null";	
	}
	else {
		conn.hostname = root["hostname"].asString();
	}

	//
	if(root["username"].isNull() || !root["username"].isString()){
		conn.username = "null";
	}
	else {
		conn.username = root["username"].asString();
	}

	//
	if(root["password"].isNull() || !root["password"].isString()){
		conn.password = "null";	
	}else {
		conn.password = root["password"].asString();
	}

	//
	if(root["port"].isNull() || !root["port"].isString()){
		conn.port = "null";
	}else {
		conn.port =  root["port"].asString();
	}

	//
	if(root["protocol"].isNull() || !root["protocol"].isString()){
		conn.protocol = "null";
	}else{
		conn.protocol =  root["protocol"].asString();
	}

	//keybord_path
	if(root["connect_path"].isNull() || !root["connect_path"].isString()){
		conn.connect_path = "/tmp/";
	}
	else {
		conn.connect_path = root["connect_path"].asString();
	}

	//domain
	if(root["domain"].isNull() || !root["domain"].isString()){
		conn.domain = "null";
	}
	else {
		conn.domain = root["domain"].asString();
	}
	
	//program
	if(root["initial-program"].isNull() || !root["initial-program"].isString()){
		conn.initial_program = "null";
	}
	else {
		conn.initial_program = root["initial-program"].asString();
	}
	
	//share_dask_flag
	if(root["enable-drive"].isNull() || !root["enable-drive"].isString()){
		conn.share_dask_flag = "false";
	}
	else {
		conn.share_dask_flag= root["enable-drive"].asString();
		printf("share_dask:%s\n",conn.share_dask_flag.c_str());
	}

	//security_type
	if(root["security"].isNull() || !root["security"].isString()){
		conn.security_type = "rdp";
	}
	else {
		conn.security_type = root["security"].asString();
	}

	//upside_clip_flag
	if(root["upside_clip_flag"].isNull() || !root["upside_clip_flag"].isString()){
		conn.upside_clip_flag = "false";
	}
	else {
		conn.upside_clip_flag = root["upside_clip_flag"].asString();
	}
	//downside_clip_flag
	if(root["downside_clip_flag"].isNull() || !root["downside_clip_flag"].isString()){
		conn.downside_clip_flag = "false";
	}
	else {
		conn.downside_clip_flag = root["downside_clip_flag"].asString();
	}
	//file_upside_clip_flag
	if(root["file_upside_clip_flag"].isNull() || !root["file_upside_clip_flag"].isString()){
		conn.file_upside_clip_flag = "false";
	}
	else {
		conn.file_upside_clip_flag = root["file_upside_clip_flag"].asString();
	}
	//file_downside_clip_flag
	if(root["file_downside_clip_flag"].isNull() || !root["file_downside_clip_flag"].isString()){
		conn.file_downside_clip_flag = "false";
	}
	else {
		conn.file_downside_clip_flag= root["file_downside_clip_flag"].asString();
	}
	
	//console	
	if(root["console"].isNull() || !root["console"].isString()){
		conn.console = "false";
		
	}
	else {
		conn.console= root["console"].asString();
	}

	//mstsc
	if(root["mstsc"].isNull() || !root["mstsc"].isString()){
		conn.mstsc = "false";
		
	}
	else {
		conn.mstsc= root["mstsc"].asString();
	}
	
//设置默认值
	check_conn_value(conn);
	
	return true;
}


static bool transStringToJson(Json::Value &root,const char*data){
	Json::Reader r;
	const char *p = data;
	while(*p != '{')
		++p;
	if(r.parse(p,root,1) == false){
		log_stdout("Json parse failed:%s\n",p);
		return false;
	}
	log_stdout("Json:%s\n",root.toStyledString().c_str());
	return true;
}


static int parseJsonString(const char*data,int sock,struct sockaddr_in *addr_cli)
{
	if(data== NULL)
		return -1;
	const char *p = data;
	while(*p != '{')
		++p;
	
	Json::FastWriter w;
	Json::Reader r;
	Json::Value root;
	if(r.parse(p, root,1) == false){
		log_stdout("Json parse failed:%s\n",p);
		return -1;
	}
	
	log_stdout("Json:%s\n",root.toStyledString().c_str());
	
	ConnectInfo_t conn;
	int len  = 0;
	char buffer[MAX_BUFFER_LEN];
	if(GetConnectInfo(root,conn)){
		memset(buffer,0,sizeof(buffer));	
		/*len = snprintf(buffer,sizeof(buffer),"$LJXX,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",\*/
		len = snprintf(buffer,sizeof(buffer),"$LJXX\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s",\
			conn.hostname.c_str(),\
			conn.username.c_str(),\
			conn.password.c_str(),\
			conn.port.c_str(),\
			conn.protocol.c_str(),\
			conn.connect_path.c_str(),\
			conn.domain.c_str(),\
			conn.initial_program.c_str(),\
			conn.security_type.c_str(),\
			conn.share_dask_flag.c_str(),\
			conn.upside_clip_flag.c_str(),\
			conn.downside_clip_flag.c_str(),\
			conn.file_upside_clip_flag.c_str(),\
			conn.file_downside_clip_flag.c_str(),\
			conn.console.c_str(),
			conn.mstsc.c_str());
		if(len < 0){
			perror("snprintf");
			return -1;
		}
		if(sendto(sock,buffer,len,0,(struct sockaddr*)addr_cli,sizeof(struct sockaddr_in))< 0){
			perror("sendto");
		}
		log_stdout("Send(%s:%d):%s\n",inet_ntoa(addr_cli->sin_addr),ntohs(addr_cli->sin_port),buffer);		
	}
	return 0;
}



static redisContext *connect_redis_nonblocking(std::string address,std::string port,std::string password)
{
	const char *ip = address.c_str();
	int tPort = atoi(port.c_str());
	struct timeval tout={0,200000};
	redisContext *c = redisConnectWithTimeout(ip,tPort,tout);
	if(c->err){
		log_stderr("RedisConnect failed:%s\n",c->errstr);
		return NULL;
	}	
	redisReply * reply = (redisReply *)redisCommand(c, "AUTH %s", password.c_str());
    if (reply->type == REDIS_REPLY_ERROR) {
        log_stderr("Redis auth failed\n");
		return NULL;
	}
    freeReplyObject(reply);
	return c;
}


static int run_string_cmd(redisContext *c,const char *cmd){
	if(c == nullptr || cmd == nullptr){
		log_stderr("c==NULL\n");
		return -1;
	}
	
	redisReply * reply = (redisReply *)redisCommand(c, cmd);
	if(reply == NULL){
		log_stderr("Call redisCommand failed:%s\n",cmd);
		return -1;
	}
	
	if(reply->type == REDIS_REPLY_ERROR){
		log_stderr("Failed to execute cmd:%s\n",cmd);
		log_stderr("Error:%s\n",reply->str);
		freeReplyObject(reply);
		return -1;
	}
	log_stderr("Successed to execute cmd:%s\n",cmd);
	freeReplyObject(reply);
	return 0;
}




ConfigIn *ConfigIn::getStance(std :: string filename){
	static ConfigIn config_singleton(filename);
	return &config_singleton;
}

ConfigIn *ConfigIn::getStance(){
	static ConfigIn config_singleton(DEF_CONFIG_FILENAME);
	return &config_singleton;
}


ConfigIn::ConfigIn(std::string filename):done(false){
	if(doingReadFile(filename))
		done = true;
}

bool ConfigIn::isValid(){
	return done;
}

std::string ConfigIn::getString(const char *buf)
{
	if(buf == NULL)
		return "";
	char *dupBuf =  strdup(buf);
	
	char *pStart = dupBuf;
	char *pEnd = dupBuf+strlen(dupBuf)-1;
	
	while(pStart!=pEnd &&isspace((int)*pStart))
		++pStart;
	
	while(pEnd!=pEnd &&isspace((int)*pStart))
		*pEnd-- = '\0';
	
	if(pStart == pEnd)
		return "";
	std::string res(pStart);
	free(dupBuf);
	return res;
} 



bool ConfigIn::doingReadFile(std::string &filename){
	std::ifstream in(filename);
	if(!in.is_open()){
		std::cerr<<"open file failed:"<<filename<<std::endl;
		return false;
	}
	
	char buffer[1024];
	int section = 0;
	while(!in.eof() && in.getline(buffer,1024)){
		std::string str = getString(buffer);
		if(str.length() == 0 || str[0] == '#')
			continue;
		if(str == "[global]"){
			section = 1;
			continue;
		}
		if(str == "[normal]"){
			section = 2;
			continue;
		}
		
		if(str == "[cluster]"){
			section = 3;
			continue;
		}
		
		if(1 == section){
			int pos = str.find("=");
			if(pos > 0){
				std::string first= str.substr(0,pos);
				std::string second = str.substr(pos+1,str.size());
				if(first == "password")
					section1.password=second;
				else if(first == "cluster_flag")
					section1.cluster_flag=second;
				else if(first == "localIp")
					section1.localIp=second;
			}
		}
		else if(2==section){
			int pos = str.find("=");
			if(pos > 0){
				std::string first= str.substr(0,pos);
				std::string second = str.substr(pos+1,str.size());
				if(first == "redisIp")
					section2.redisIp=second;
				else if(first == "redisPort")
					section2.redisPort=second;
			}
		}	
		else if(3==section){
			int pos = str.find(":");
			if(pos > 0){
				section3.m_mp.insert(std::make_pair(str.substr(0,pos),str.substr(pos+1,str.size())));			
			}
		}
	}

#ifdef __DEBUGOUT__
	std::cout<<"password:"<<section1.password<<std::endl;
	std::cout<<"cluster_flag:"<<section1.cluster_flag<<std::endl;
	std::cout<<"redisIp:"<<section2.redisIp<<std::endl;
	std::cout<<"redisPort:"<<section2.redisPort<<std::endl;
	std::cout<<"localIp:"<<section1.localIp<<std::endl;
	for(auto i = section3.m_mp.begin();i!=section3.m_mp.end();++i){
		std::cout<<"first:"<<i->first<<std::endl;
		std::cout<<"second:"<<i->second<<std::endl;
	}
#endif

	in.close();	
	return true;	
}



bool ClusterQueryRedis::request_connection_session(redisContext *c,std::string sid){
	if(c == NULL)
		return -1;
	char buffer[1024];
	memset(buffer,0,sizeof(buffer));
	snprintf(buffer,1024,"get connection_session::%s",sid.c_str());

	log_stdout("Redis cmd:%s\n",buffer);
	redisReply * reply = (redisReply *)redisCommand(c, buffer);
	if(reply == NULL){
		log_stderr("Call redisCommand failed:%s\n",buffer);
		return false;
	}
	
	if(reply->type != REDIS_REPLY_STRING){
		log_stderr("reply->type != REDIS_REPLY_STRING\n");
		freeReplyObject(reply);
		return false;
	}
	
	if(transStringToJson(this->root,reply->str)){
		//获取json成功
		log_stdout("transStringToJson success\n");
		freeReplyObject(reply);
		return true;
	}

	freeReplyObject(reply);
	return false;
}

bool ClusterQueryRedis::loopQuery(){
	if(cfg == NULL){
		log_stderr("cfg is null\n");
		return false;
	}
	std::string address;
	std::string port;
	std::string password = cfg->section1.password;	
	std::map<std::string,std::string>&mp = cfg->section3.m_mp;
	redisContext *c = NULL;
	for(auto iter = mp.rbegin();iter!=mp.rend();++iter){
		address = iter->first;
		port = iter->second;
		c = connect_redis_nonblocking(address,port,password);
		if(c == NULL){
			log_stderr("connect %s:%s failed\n",address.c_str(),port.c_str());
			continue;
		}else{
			log_stderr("connect %s:%s success\n",address.c_str(),port.c_str());
		}
		if(request_connection_session(c, sid) == true){
			this->c = c;//不用释放
			break;
		}
		redisFree(c);
	}
	return true;	
}

void ClusterQueryRedis:: runCommand(std::string cmd){
	run_string_cmd(c, cmd.c_str());
}


bool ClusterQueryRedis::query(const std::string sid){
	cfg = ConfigIn::getStance(DEF_CONFIG_FILENAME);
	if(!cfg->isValid()){
		std::cerr<<"config failed"<<std::endl;
		return false;
	}
	
	this->sid = sid;
	return loopQuery();
}


bool  ClusterQueryRedis::sendConnInfo(int sock,struct sockaddr*addr){
//以下代码存在问题
	char buffer[MAX_BUFFER_LEN];
	ConnectInfo_t conn;
	int len = 0;
	if(::GetConnectInfo(this->root,conn)){
		memset(buffer,0,sizeof(buffer));
		len = snprintf(buffer,sizeof(buffer),\
		"$LJXX\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s",\
			conn.hostname.c_str(),\
			conn.username.c_str(),\
			conn.password.c_str(),\
			conn.port.c_str(),\
			conn.protocol.c_str(),\
			conn.connect_path.c_str(),\
			conn.domain.c_str(),\
			conn.initial_program.c_str(),\
			conn.security_type.c_str(),\
			conn.share_dask_flag.c_str(),\
			conn.upside_clip_flag.c_str(),\
			conn.downside_clip_flag.c_str(),\
			conn.file_upside_clip_flag.c_str(),\
			conn.file_downside_clip_flag.c_str(),\
			conn.console.c_str(),
			conn.mstsc.c_str());
		if(len < 0){
			perror("snprintf");
			return -1;
		}
		
		log_stdout("send:%s",buffer);
		if(sendto(sock,buffer,len,0,addr,sizeof(struct sockaddr_in)) < 0){
			perror("sendto");
		}		
	}	
	return true;
}


ClusterQueryRedis::ClusterQueryRedis()
{
	sid.clear();
	cfg = NULL;
	c = NULL;
}

ClusterQueryRedis::~ClusterQueryRedis(){
	std::cout<<"~ClusterQueryRedis()"<<std::endl;
	redisFree(c);
}


int read_redis_connection(char *redis_ip,int *redis_port,char *redis_passwd,const char *file_conf){
	FILE*fp = fopen(file_conf,"r");
	if(fp == NULL){
		perror("fopen");
		return -1;
	}

	char buffer[1024];
	char *p = NULL;
	int flags = 0;
	while(1){
		memset(buffer,0,sizeof(buffer));
		if(fgets(buffer,1024,fp) == NULL){
			if(!feof(fp)){
				perror("fgets");
			}
			break;
		}
		p = buffer+strlen(buffer)-1;
		while(*p == ' ' || *p=='\n'){
			*p = 0;
			--p;
		}

		p=buffer;
		while(*p == ' ')
			++p;
		if(strncmp(p,"redis_ip",strlen("redis_ip")) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				memcpy(redis_ip,pstr,strlen(pstr));
				++flags;
		}

		if(strncmp(p,"redis_port",strlen("redis_port")) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				*redis_port = atoi(pstr);
				++flags;
		}

		if(strncmp(p,"redis_password",strlen("redis_password")) == 0){
				const char *pstr = strstr(p,"=");
				if(pstr == NULL){
					fclose(fp);
					return -1;
				}
				++pstr;
				memcpy(redis_passwd,pstr,strlen(pstr));
				++flags;
		}

	}

	fclose(fp);
	return (flags == 3)?0:-1;
}



int pull_dis_conn_redis(const char*sid)
{
	char buffer[MAX_BUFFER_LEN];
	char redis_ip[32];
	int redis_port;
	char redis_password[256];
	if(read_redis_connection(redis_ip,&redis_port,redis_password,XRDP_NET_CONF_FILE) < 0){
		log_stderr("get_redis_connection failed\n");
		return -1;
	}
	redisContext *c = cm_redisConnect(redis_ip,redis_port,redis_password);
	if(c == NULL){
		return -1;
	}
	memset(buffer,0,sizeof(buffer));
	snprintf(buffer,256,"publish session_close %s",sid);
	log_stdout("Call run_string_cmd:%s\n",buffer);
	run_string_cmd(c,buffer);
	redisFree(c);
	return 0;
}





int my_request_redis(const char *sid,const char*xrdpip,int sock,struct sockaddr_in *addr_cli,int w,int h,int bpp)
{	
#if 1
	char buffer[MAX_BUFFER_LEN];
	char redis_ip[32];
	int redis_port;
	char redis_password[256];
	if(read_redis_connection(redis_ip,&redis_port,redis_password,XRDP_NET_CONF_FILE) <0){
		log_stderr("get_redis_connection < 0:need check xrdp_network.conf\n");
		return -1;
	}
	redisContext *c = cm_redisConnect(redis_ip,redis_port,redis_password);
	if(c == NULL){
		return -1;
	}
	memset(buffer,0,sizeof(buffer));
	snprintf(buffer,1024,"get connection_session::%s",sid);

	log_stdout("Redis cmd:%s\n",buffer);
	redisReply * reply = (redisReply *)redisCommand(c, buffer);
	if(reply == NULL){
		log_stderr("Call redisCommand failed:%s\n",buffer);
		redisFree(c);
		return -1;
	}
	
	if(reply->type != REDIS_REPLY_STRING){
		log_stderr("reply->type != REDIS_REPLY_STRING\n");
		redisFree(c);
		freeReplyObject(reply);
		return -1;
	}
	
	parseJsonString(reply->str,sock,addr_cli);
	
	memset(buffer,0,sizeof(buffer));
	snprintf(buffer,256,"publish session_serverIp [%s][%s][%d][%d][%d]",sid,xrdpip,w,h,bpp);
	log_stdout("Call run_string_cmd:%s\n",buffer);
	
	run_string_cmd(c,buffer);
	redisFree(c);
	freeReplyObject(reply);
	return 0;
#endif
}


redisContext *cm_redisConnect(const char * ip, int port,const char *password)
{
	redisContext *c = redisConnect(ip,port);
	if(c->err){
		log_stderr("RedisConnect failed:%s\n",c->errstr);
		return NULL;
	}
	log_stdout("RedisConnect success\n");
	redisReply * reply = (redisReply *)redisCommand(c, "AUTH %s", password);
    if (reply->type == REDIS_REPLY_ERROR) {
        fprintf(stderr,"Redis auth failed\n");
		return NULL;
	}
    fprintf(stdout,"Redis auth success\n");
    freeReplyObject(reply);
	return c;
}

