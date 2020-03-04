#ifndef __PUBLIC_H
#define __PUBLIC_H
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define log_stderr(fmt,...) fprintf(stderr,fmt,##__VA_ARGS__)

#define log_stdout(fmt,...) fprintf(stdout,fmt,##__VA_ARGS__)

#define  MAX_BUFFER_LEN 4096

#define XRDP_NET_CONF_FILE "/etc/xrdp/xrdp_network.conf"

#endif