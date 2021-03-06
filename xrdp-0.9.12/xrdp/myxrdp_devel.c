#if defined(HAVE_CONFIG_H)
#include <config_ac.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "xrdp.h"
#include "log.h"

#include "myxrdp.h"

MyTransInfo_t *myxrdp_mytrans_create(const char *sessionid){
	MyTransInfo_t*p = (MyTransInfo_t*)g_malloc(sizeof(struct MyTransInfo),1);
	p->fifoFd[0] = p->fifoFd[1] = -1;
	p->filename1[0] = p->filename2[0] = '\0';
	p->session_type = SES_OTHER;
	p->tls = NULL;
	p->conn_status = DISCONNECTED;
	p->send_to_slave = SEND_OFF;
	g_snprintf(p->filename1,sizeof(p->filename1),"%s/%s_1.fifo",SESSIONPATH,sessionid);
	g_snprintf(p->filename2,sizeof(p->filename2),"%s/%s_2.fifo",SESSIONPATH,sessionid);
	g_snprintf(p->filename3,sizeof(p->filename3),"%s/%s_3.tmp",SESSIONPATH,sessionid);
	return p;
}

static void myxrdp_mytrans_give(MyTransInfo_t *p){	
	if(p == NULL)
		return;
		
#ifdef __DEBUG_OUT__	
	log_message(LOG_LEVEL_DEBUG,"closefd:%d",p->fifoFd[0]);
	log_message(LOG_LEVEL_DEBUG,"closefd:%d",p->fifoFd[1]);
#endif
	gl_closeFd(p->fifoFd[0]);
	gl_closeFd(p->fifoFd[1]);	
	if(p->session_type == SES_MASTER){
		log_message(LOG_LEVEL_DEBUG,"delete:%s",p->filename1);
		gl_delete_file(p->filename1);
		log_message(LOG_LEVEL_DEBUG,"delete:%s",p->filename2);
		gl_delete_file(p->filename2);
		log_message(LOG_LEVEL_DEBUG,"delete:%s",p->filename3);
		gl_delete_file(p->filename3);
		g_sleep(1000);
	}	
	p->session_type = SES_OTHER;
	p->send_to_slave = SEND_OFF;
}

int myxrdp_mytrans_send_disconn(MyTransInfo_t *p){
	if(p == NULL)
		return 1;
	PacketInfo_t pack;
	pack.type = DATA_TYPE_DISCONN;
	int fd = -1;
	if(p->session_type == SES_MASTER){
		fd = p->fifoFd[0];//读1写0
	}
	else if(p->session_type == SES_SLAVE){
		fd = p->fifoFd[1];//读0写1
	}
	if(fd < 0)
		return 1;
	if(gl_fd_can_send(fd,100)){
		gl_write(fd,&pack,sizeof(pack));
	}
	return 0;
}

void myxrdp_mytrans_delete(MyTransInfo_t* p){
	myxrdp_mytrans_send_disconn(p);
	myxrdp_mytrans_give(p);
	g_free(p);
}



int myxrdp_set_rdp(	struct xrdp_mm* self)
{	 
	if(self == NULL)
		return -1;
	//模块动态库
	list_add_item(self->login_names, (long)g_strdup("lib"));
	list_add_item(self->login_values, (long)g_strdup("libxrdpneutrinordp.so"));
		 
	//用户名字
	list_add_item(self->login_names, (long)g_strdup("username"));
	list_add_item(self->login_values, (long)g_strdup("Administrator"));
		 
	//密码	 
	list_add_item(self->login_names, (long)g_strdup("password"));
	list_add_item(self->login_values, (long)g_strdup("000000"));
		 
	//ip
	list_add_item(self->login_names, (long)g_strdup("ip"));
	list_add_item(self->login_values, (long)g_strdup("192.168.0.101"));
		 
	//端口
	list_add_item(self->login_names, (long)g_strdup("port"));
	list_add_item(self->login_values, (long)g_strdup("3389"));
		 
	return 0;
	//其他设置
}

void myxrdp_set_wait_objs(struct xrdp_wm *self,tbus*robjs,int *rc)
{
	if(self == NULL)
		return;
	
	if(self->my_trans_info->session_type == SES_SLAVE){
		   int i = *rc;
           robjs[i++] = self->my_trans_info->fifoFd[0];
           *rc = i;
	}
	if(self->my_trans_info->session_type == SES_MASTER){
		   int i = *rc;
           robjs[i++] = self->my_trans_info->fifoFd[1];
           *rc = i;
	}
}

//open 
int myxrdp_open_fifo(const char *filename){
	if(filename==NULL || strlen(filename)==0){
		log_message(LOG_LEVEL_ERROR,"myxrdp_open_fifo:argments error");	
		return (-1);
	}
	if(gl_create_fifo(filename) != 0){
		return (-1);
	}
	return gl_open_fifo(filename,O_RDWR);
}

int myxrdp_open_fifo_rw(MyTransInfo_t *self){
	self->fifoFd[0] = myxrdp_open_fifo(self->filename1);
	self->fifoFd[1] = myxrdp_open_fifo(self->filename2);
	if(self->fifoFd[0]<0 || self->fifoFd[1] < 0)
		return (-1);
	return 0;
}

int myxrdp_slave_read(MyTransInfo_t*my_trans){
	if(my_trans->session_type != SES_SLAVE)
		return 0;

	int fd = my_trans->fifoFd[0];
	if(g_is_wait_obj_set(fd)){
		PacketInfo_t pack;
		bzero(&pack,sizeof(pack));
		if(gl_read(fd,&pack, sizeof(pack)) <= 0){
			return 1;
		}
		if(pack.type == DATA_TYPE_RDP){
			if(pack.length > 0 && my_trans->tls!=NULL)
				ssl_tls_write(my_trans->tls,pack.buffer,pack.length);
		}
		else if(pack.type == DATA_TYPE_DISCONN){
			log_message(LOG_LEVEL_ERROR,"slave recv disconnect");
			return 1;
		}
	}
	return 0;
}

int myxrdp_master_read(MyTransInfo_t*my_trans){

	if(my_trans->session_type != SES_MASTER)
		return 0;
	if(g_is_wait_obj_set(my_trans->fifoFd[1])){	
		PacketInfo_t pack;
		bzero(&pack,sizeof(pack));
		if(gl_read(my_trans->fifoFd[1],&pack, sizeof(pack)) <= 0){
			return 1;
		}
		log_message(LOG_LEVEL_ERROR,"master recv %d",pack.type);
		if(pack.type == DATA_TYPE_DISCONN){
			log_message(LOG_LEVEL_ERROR,"master recv disconnect");
			return 1;
		}
	}
	return 0;
}


int myxrdp_read_fifo_data(struct xrdp_wm *self){
	if(self == NULL){
		return 0;
	}
	MyTransInfo_t*p = self->my_trans_info;
	if(p == NULL)
		return 1;
	if(p->session_type == SES_SLAVE){	
		return myxrdp_slave_read(p);
	}
	else if(p->session_type == SES_MASTER){
		return myxrdp_master_read(p);
	}
	return 0;
}

int myxrdp_master_recv_args(struct xrdp_wm *self,int fd){
	session_arg_t data_arg;
	//wait slave session write connection argments
	if(gl_fd_can_recv(fd,MAX_MASTER_WAIT_SEC*1000)==0){
		log_message(LOG_LEVEL_ERROR,"wait slave connection timeouts:s");
		return 1;
	}
	
	//read argments
	if(gl_read(fd, &data_arg,sizeof(data_arg))<0){
		log_message(LOG_LEVEL_ERROR,"myread session argments failed:%s",strerror(errno));
		return 1;
	}
	
	#ifdef __DEBUG_OUT__
		log_message(LOG_LEVEL_DEBUG,"master:arg.rdp5_performanceflags:%d",data_arg.rdp5_performanceflags);
		log_message(LOG_LEVEL_DEBUG,"master:arg.rdp_compression:%d",data_arg.rdp_compression);
	#endif
	self->client_info->rdp5_performanceflags=data_arg.rdp5_performanceflags;
	self->client_info->rdp_compression = data_arg.rdp_compression;
	return 0;	
}

//从会话发送参数到管道
static int myxrdp_slave_send_args(struct xrdp_wm *self,int fd){
	session_arg_t data_arg;
	data_arg.rdp5_performanceflags = self->client_info->rdp5_performanceflags;
	data_arg.rdp_compression = self->client_info->rdp_compression;
#ifdef __DEBUG_OUT__
	log_message(LOG_LEVEL_DEBUG,"slave:arg.rdp5_performanceflags:%d",data_arg.rdp5_performanceflags);
	log_message(LOG_LEVEL_DEBUG,"slave:arg.rdp_compression:%d",data_arg.rdp_compression);
#endif
	if(gl_write(fd,&data_arg,sizeof(data_arg))<=0){
		log_message(LOG_LEVEL_ERROR,"slave:myWrite failed:%s",strerror(errno));
		return 1;
	}
	return 0;
}


static int myxrdp_fill_conn_rdp(struct xrdp_mm * self, const ConnInfo_t * conn){
	if(self == NULL||conn==NULL)
			return -1;
	//模块动态库
	list_add_item(self->login_names, (long)g_strdup("lib"));
	list_add_item(self->login_values, (long)g_strdup("libxrdpneutrinordp.so"));
	
	//用户名字
	list_add_item(self->login_names, (long)g_strdup("username"));
	list_add_item(self->login_values, (long)g_strdup(conn->username));
	
	//密码	
	list_add_item(self->login_names, (long)g_strdup("password"));
	list_add_item(self->login_values, (long)g_strdup(conn->password));
	
	//ip
	list_add_item(self->login_names, (long)g_strdup("ip"));
	list_add_item(self->login_values, (long)g_strdup(conn->hostname));
	
	//端口
	list_add_item(self->login_names, (long)g_strdup("port"));
	list_add_item(self->login_values, (long)g_strdup(conn->port));
	
	//domain
	list_add_item(self->login_names, (long)g_strdup("domain"));
	list_add_item(self->login_values, (long)g_strdup(conn->domain));
	
	
	//console
	list_add_item(self->login_names, (long)g_strdup("console"));
	list_add_item(self->login_values, (long)g_strdup(conn->console));
	
	//program
	list_add_item(self->login_names, (long)g_strdup("program"));
	list_add_item(self->login_values, (long)g_strdup(conn->initial_program));
	return 0;
	//其他设置
}
static int myxrdp_fill_conn_vnc(struct xrdp_mm * self, const ConnInfo_t * conn){
	if(self == NULL||conn==NULL)
		return -1;
	//模块动态库libvnc.so
	list_add_item(self->login_names, (long)g_strdup("lib"));
	list_add_item(self->login_values, (long)g_strdup("libvnc.so"));
	//用户名字
	list_add_item(self->login_names, (long)g_strdup("username"));
	list_add_item(self->login_values, (long)g_strdup(conn->username));

	//密码	
	list_add_item(self->login_names, (long)g_strdup("password"));
	list_add_item(self->login_values, (long)g_strdup(conn->password));

	//ip
	list_add_item(self->login_names, (long)g_strdup("ip"));
	list_add_item(self->login_values, (long)g_strdup(conn->hostname));

	//端口
	list_add_item(self->login_names, (long)g_strdup("port"));
	list_add_item(self->login_values, (long)g_strdup(conn->port));
	return 0;
}

static int myxrdp_fill_conn_xorg(struct xrdp_mm * self){	
	if(self == NULL)
		return false;
	if(self->wm == NULL)
		return false;
	if(self->wm->conf_extra->userPoolIp==NULL ||\
		self->wm->conf_extra->userPoolPort==NULL)
		return false;
	if(self->wm->conf_extra->CssProgramName == NULL)
		return false;
	
	//模块动态库libxup.so
	list_add_item(self->login_names, (long)g_strdup("lib"));
	list_add_item(self->login_values, (long)g_strdup("libxup.so"));
	char username[256];
	char password[256];	
#if 1
	//用户名和密码是登陆本地的，需要访问用户池
	bzero(username,256);
	bzero(password,256);
	
	if(extra_getUserInfoFromPool(username,password,self->wm->conf_extra->userPoolIp,\
		atoi(self->wm->conf_extra->userPoolPort)) == false){
		log_message(LOG_LEVEL_ERROR,"GetUserInfoFromPool failed:%s/%s",\
			self->wm->conf_extra->userPoolIp,self->wm->conf_extra->userPoolPort);
		return false;
	}
		
	log_message(LOG_LEVEL_DEBUG,"GET USER :%s,%s",username,password);
	memcpy(self->username,username,strlen(username));
	memcpy(self->password,password,strlen(password));	
#endif

	list_add_item(self->login_names, (long)g_strdup("ip"));
	list_add_item(self->login_values, (long)g_strdup("127.0.0.1"));

	list_add_item(self->login_names, (long)g_strdup("port"));
	list_add_item(self->login_values, (long)g_strdup("-1"));

	list_add_item(self->login_names, (long)g_strdup("username"));
	list_add_item(self->login_values, (long)g_strdup(username));

	list_add_item(self->login_names, (long)g_strdup("password"));
	list_add_item(self->login_values, (long)g_strdup(password));

	list_add_item(self->login_names, (long)g_strdup("code"));
	list_add_item(self->login_values, (long)g_strdup("20"));
	if(self->wm &&self->wm->client_info){
		char *pbuf = self->wm->client_info->program; ///usr/bin/testx11.sh 
		snprintf(pbuf,sizeof(self->wm->client_info->program),\
		self->wm->conf_extra->CssProgramName);
	}

	g_strlen(self->wm->client_info->program);	
	return true;
}


static int myxrdp_fill_conn_x11(struct xrdp_mm * self, const ConnInfo_t * conn){
	if(self == NULL||conn==NULL)
		return -1;
	if(self->wm == NULL)
		return -1;

	//模块动态库libxup.so
	list_add_item(self->login_names, (long)g_strdup("lib"));
	list_add_item(self->login_values, (long)g_strdup("libxup.so"));
	
	char username[256];
	char password[256];
	
#if 1
	//用户名和密码是登陆本地的，需要访问用户池
	bzero(username,256);
	bzero(password,256);
	if(extra_getUserInfoFromPool(username,password,self->wm->conf_extra->userPoolIp,\
		atoi(self->wm->conf_extra->userPoolPort)) == false){
		log_message(LOG_LEVEL_ERROR,"GetUserInfoFromPool failed:%s/%s",\
			self->wm->conf_extra->userPoolIp,self->wm->conf_extra->userPoolPort);
		return -1;
	}
	log_message(LOG_LEVEL_DEBUG,"GET USER :%s,%s",username,password);
	memcpy(self->username,username,strlen(username));
	memcpy(self->password,password,strlen(password));	
#endif

	list_add_item(self->login_names, (long)g_strdup("ip"));
	list_add_item(self->login_values, (long)g_strdup("127.0.0.1"));

	list_add_item(self->login_names, (long)g_strdup("port"));
	list_add_item(self->login_values, (long)g_strdup("-1"));

	list_add_item(self->login_names, (long)g_strdup("username"));
	list_add_item(self->login_values, (long)g_strdup(conn->username));

	list_add_item(self->login_names, (long)g_strdup("password"));
	list_add_item(self->login_values, (long)g_strdup(conn->password));

	list_add_item(self->login_names, (long)g_strdup("code"));
	list_add_item(self->login_values, (long)g_strdup("20"));
#if 1	
	if(self->wm &&self->wm->client_info){
		char *pbuf = self->wm->client_info->program; ///usr/bin/testx11.sh 
		//snprintf(pbuf,sizeof(self->wm->client_info->program),"/usr/bin/testx11.sh");
		snprintf(pbuf,sizeof(self->wm->client_info->program),\
		self->wm->conf_extra->XnestProgramName);
	}
#endif	
	g_strlen(self->wm->client_info->program);	
	return 0;	
}

static int myxrdp_writeSessionIdToFile(const char *s_name,int width,int height,const char *filename)
{
	if(s_name == NULL || filename==NULL)
		return (-1);
	FILE* fp = fopen(filename,"w");
	if(fp == NULL)
		return (-1);
	chmod(filename,0666);

	fwrite(s_name,strlen(s_name),1,fp);
	fwrite("\n",1,1,fp);

	char buffer[512]={0};
	int nlen=snprintf(buffer,512,"%d\n",width);
	fwrite(buffer,nlen,1,fp);
	nlen=snprintf(buffer,512,"%d\n",height);
	fwrite(buffer,nlen,1,fp);
	
	fclose(fp);
	return 0;
}


//连接目标机
static int myxrdp_connect_protocol(struct xrdp_wm *self){
	ConnInfo_t *conn = &self->conn;
	const char *x11_ssid_filename = X11_SSID_FILENAME;
	long lastModTime = 0L;
	int lockfd = -1;
	if(g_strcmp(conn->protocol,"rdp") == 0){
		if(myxrdp_fill_conn_rdp(self->mm, conn) < 0){
			log_message(LOG_LEVEL_ERROR,"set_connect_info_rdp failed");
			return 1;
		}
	}
	else if(g_strcmp(conn->protocol,"vnc") == 0){
		if(myxrdp_fill_conn_vnc(self->mm, conn) < 0){
			log_message(LOG_LEVEL_ERROR,"set_connect_info_vnc failed");
			return 1;
		}	
	}
	else if(g_strcmp(conn->protocol,"x11") == 0){	
		if(myxrdp_fill_conn_x11(self->mm, conn) < 0){
			log_message(LOG_LEVEL_ERROR,"set_connect_info_x11 failed");
			return 1;
		}
		
		char *ssid = self->session->client_info->username;
		int width = self->session->client_info->width;
		int height = self->session->client_info->height;
		//阻塞加锁
		lockfd = gl_file_block_lock("x11share.lock");
		myxrdp_writeSessionIdToFile(ssid,width,height,x11_ssid_filename);
		lastModTime = extra_getLastModTime(x11_ssid_filename);		
	}
	
	
	int rv = 0;
	self->my_trans_info->send_to_slave = SEND_ON;
	if (xrdp_mm_connect(self->mm) == 0){		
		xrdp_wm_set_login_mode(self, 3);
		xrdp_wm_delete_all_children(self);
		self->dragging = 0;
	}else {
		rv = 1;
		self->my_trans_info->send_to_slave = SEND_OFF;
		log_message(LOG_LEVEL_DEBUG,"Connnect failed protocol:%s ",conn->protocol);
	}
	if(g_strcmp(conn->protocol,"x11") == 0){
		if( rv == 0){
			extra_isFileChanged(x11_ssid_filename,lastModTime,3);
			self->pid_x11[0] = extra_getPidFromFile(x11_ssid_filename,4);
			self->pid_x11[1] = extra_getPidFromFile(x11_ssid_filename,5);
		}
		//解锁
		gl_file_unlock_close(lockfd);
		
	}
	return rv;
}

static int myxrdp_query_conn_redis(struct xrdp_wm *self){
	//请求redis
	QueryRedisData_t *qrd = qrd_create(&self->conn,self->client_info);
	if(qrd->query(qrd) == false){
		qrd_delete(qrd);
		return false;
	}	
	qrd_delete(qrd);

#ifdef __DEBUG_OUT__	
	log_message(LOG_LEVEL_ERROR,"query redis failed");
#endif

	return true;
}

//extra code by yuliang
//manager connection for init master and slave session
int myxrdp_conn_manager(struct xrdp_wm *self){
	if(self == NULL)
		return 1;
	if(self->session==NULL)
		return 1;
	if(self->session->trans==NULL)
		return 1;
	struct trans *trans = self->session->trans;
	const char *sessionId = self->session->client_info->username;
	if(g_strlen(self->ssid) > 0)
		sessionId = self->ssid;
	if(g_strlen(sessionId) == 0){
		log_message(LOG_LEVEL_DEBUG,"Invalid sessionId");
		return 1;
	}
	log_message(LOG_LEVEL_DEBUG,"sessionid:%s",sessionId);

	if(self->my_trans_info == NULL){
		self->my_trans_info = myxrdp_mytrans_create(sessionId);
		trans->pMyTransInfo = self->my_trans_info;//主会话转发需要用到此接口
	}
	MyTransInfo_t *p1 = self->my_trans_info;
	p1->tls = trans->tls;
	p1->session_type = myxrdp_get_session_type(p1->filename3);
	log_message(LOG_LEVEL_DEBUG,"session type:%d",p1->session_type);
	
	if(p1->session_type == SES_MASTER){
		log_message(LOG_LEVEL_DEBUG,"I am master session,pid-%d",getpid());
		if(myxrdp_open_fifo_rw(p1) < 0){
			log_message(LOG_LEVEL_ERROR,"master:myxrdp_open_fifo_rw failed:%s",strerror(errno));
			return 1;
		}
		
		//读取从会话的参数
		if(myxrdp_master_recv_args(self,p1->fifoFd[1])!=0){
			return 1;
		}
		
		if(!myxrdp_query_conn_redis(self)){
			log_message(LOG_LEVEL_ERROR,"Query redis failed");
			return 1;
		}
		
#ifdef __DEBUG_OUT__
		conn_data_show(&self->conn);
#endif		
		//连接目标机
		return myxrdp_connect_protocol(self);
	}
	else if(p1->session_type == SES_SLAVE){
		log_message(LOG_LEVEL_DEBUG,"I am slave session,pid-%d",getpid());		
		if(myxrdp_open_fifo_rw(p1) < 0){
			log_message(LOG_LEVEL_ERROR,"slave:myxrdp_open_fifo_rw failed:%s",strerror(errno));
			return 1;
		}
		if(myxrdp_slave_send_args(self,p1->fifoFd[1])!=0){
			return 1;
		}
		
		xrdp_wm_set_login_mode(self, 3);
		xrdp_wm_delete_all_children(self);
		self->dragging = 0;
		return 0;
	}

	log_message(LOG_LEVEL_DEBUG,"I am other session");
	return 1;
}

static int myxrdp_writeUserArgsToFile(struct xrdp_client_info *info,const char *filename){
	if(filename == NULL || info == NULL)
		return (-1);
	FILE* fp = fopen(filename,"w+");
	if(fp == NULL){
		perror("fopen");
		return (-1);
	}

	//文件权限必须是666，因为用户池给的用户权限是普通用户
	if(chmod(filename,0666) < 0){
		perror("chmod");
		return (-1);
	}

	int width = info->width;
	int height = info->height;
	char buffer[512]={0};
	int nlen=snprintf(buffer,512,"%d\n",width);
	fwrite(buffer,nlen,1,fp);
	nlen=snprintf(buffer,512,"%d\n",height);
	fwrite(buffer,nlen,1,fp);	
	fclose(fp);
	return 0;
}


int myxrdp_direct_conn_menu(struct xrdp_wm *self)
{
	if(self == NULL){
		return 0;
	}
	
	if(self->conf_extra->directMenuPathName==NULL){
		log_message(LOG_LEVEL_ERROR,"direct_connect_menu:argments error");
		return 0;
	}
		
	//配置x11信息
	if(!myxrdp_fill_conn_xorg(self->mm)){
		log_message(LOG_LEVEL_ERROR,"set_connect_xorg failed");
		return 1;
	}
	
	//写文件信息
	const char *filename = self->conf_extra->directMenuPathName;

	//加锁,加锁内的任何函数都不能出错，否则会导致所有进程在此处阻塞
	int lockfd = gl_file_block_lock("direct_conn_menu.lock");
	
	if(myxrdp_writeUserArgsToFile(self->session->client_info,filename) < 0){
		log_message(LOG_LEVEL_ERROR,"writeUserArgsToFile failed");	
		gl_file_unlock_close(lockfd);
		return 1;
	}
	
	int ret = 1;
	long lastModTime = extra_getLastModTime(filename);
	//连接xorg环境
	if (xrdp_mm_connect(self->mm) == 0){
		char tempsid[256] = {0};
		extra_isFileChanged(filename,lastModTime,3);
		if(extra_getDirectMenuTempSidFromFile(tempsid,256,filename)==0){
				g_snprintf(self->tempsid,sizeof(self->tempsid),"%s",tempsid);
				xrdp_wm_set_login_mode(self, 22);
				self->work_mode = MODE_DIRECT_MENU;//直连菜单模式
				xrdp_wm_delete_all_children(self);
				self->dragging = 0;
				self->pid_dm = extra_getDirectMenuPidFromFile(filename);
				ret = 0;
		}else{
			log_message(LOG_LEVEL_ERROR,"extra_getDirectMenuTempSidFromFile failed");
			ret = 1;
		}
	}
	
	//解锁
	gl_file_unlock_close(lockfd);
	return ret;
}


enum SessionType myxrdp_get_session_type(const char *filename){
	if(g_strlen(filename) == 0){
		errno = EINVAL;
		return SES_ERROR;
	}
	
	if(gl_file_existed(filename) == false){
		gl_create_file(filename,0664);
		return SES_MASTER;
	}
	else {
		if(gl_file_null(filename) == 0){
			return SES_OTHER;
		}else{
			gl_file_append_line(filename,"000000",6);
			return SES_SLAVE;
		}
	}
	return SES_OTHER;
}
