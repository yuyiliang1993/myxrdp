#ifndef __MY_XRDP_H__
#define __MY_XRDP_H__
#include "myxrdp_common.h"
#include "myxrdp_query.h"

typedef struct MyTransInfo MyTransInfo_t;

int myxrdp_read_fifo_data(struct xrdp_wm *self);

void myxrdp_clean(struct xrdp_wm *self);

void myxrdp_set_wait_objs(struct xrdp_wm *self,tbus*robjs,int *rc);

void myxrdp_mytrans_init(MyTransInfo_t*p);

void myxrdp_mytrans_delete(MyTransInfo_t*p);

int myxrdp_conn_manager(struct xrdp_wm *self);

int myxrdp_direct_connect_menu(struct xrdp_wm *self);

enum SessionType;//in myxrdp_common.h

enum SessionType myxrdp_get_session_type(const char *filename);


#ifndef XRDP_EXTRA_CONF_NAME
#define XRDP_EXTRA_CONF_NAME "/etc/xrdp/xrdp_extra.cfg"
#endif

#ifndef X11_SSID_FILENAME 
#define X11_SSID_FILENAME "/tmp/.sessionx11.dat"
#endif

#ifndef DIRECT_MENU_FILENAME 
#define DIRECT_MENU_FILENAME  "/tmp/.directmenu.dat"
#endif



typedef struct xrdp_extra_config{
	char *localIp;
	char *directMenuFlag;
	char *directMenuPathName;
	char *x11PathName;
	char *OCRFlag;
	char *connRedisIp;
	char *connRedisPort;
	char *userPoolIp;
	char *userPoolPort;
	char *CssProgramName;
	char *XnestProgramName;
}xrdpExtraConfig_t;


#include <semaphore.h>
#ifndef SEM_X11_NAME
#define SEM_X11_NAME "/sem_name"
#endif

xrdpExtraConfig_t *extra_cfg_create(void);

void extra_cfg_delete(xrdpExtraConfig_t *cfg);

int extra_getUserInfoFromPool(char * username, char * passwd, const char * ip, int port);

int extra_returnUserInfoToPool(xrdpExtraConfig_t *conf,const char *username,const char*password);

long extra_getLastModTime(const char *filename);

long extra_getPidFromFile(const char *filename,int linenum);

int extra_process_stop(long pid);

int extra_isFileChanged(const char *filename,long lastModTime,int max_sec);

int extra_read_config(xrdpExtraConfig_t *cfg,const char *filename);

int extra_getDirectMenuTempSidFromFile(char *dstBuf,int size,const char *filename);

long extra_getDirectMenuPidFromFile(const char *filename);

int extra_getSessionIdFromFile(char *dest,int size,const char *filename);

#endif
