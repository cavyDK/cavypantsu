#ifndef SER_H__
#define SER_H__


#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>    
#include <pthread.h>
#include <sqlite3.h>
#include <unistd.h>

#define REGISTER 1
#define LOGIN 2
#define CANCEL 3
#define ADMIN_ADD 4
#define ADMIN_DEL_TEL 5
#define ADMIN_DEL_ID 6
#define ADMIN_FIND_TEL 7
#define ADMIN_FIND_ID 8
#define ADMIN_FIND_NUM 9
#define ADMIN_PRINT 10
#define ADMIN_CHG 11
#define USR_PRINT 12
#define USR_CHG 13
#define ADMIN_FIND_USRNAME 14

#define REG_KEY "wuhu_airport_caption"






typedef struct //消息结构体
{
	int func;
	char usr[20];
	char mesg[512];
}__cli_mesg;


typedef struct //线程传参结构体
{
	int newfd;
	struct sockaddr_in cin;
}__cli_info;
typedef char Setstaff[9][40];



sqlite3* db =NULL;  //句柄


char sql[256]= ""; //sqlite3指令字符串
char* errmsg;       //sqlite3错误信息


void sendstaffinfo(char* constusr,int newfd,char* ip);
void recvmesg(__cli_mesg* buf, int newfd, char* ip);
int setstaffinfo(__cli_mesg buf,Setstaff* set);
void closepth(int fd);
void* recv_cli_msg(void* arg);//线程
#endif
