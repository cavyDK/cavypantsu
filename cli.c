#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

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
void fuckclear();
void* recv_ser_msg(void* arg);
void view(void);
typedef char Setstaff[9][40];
typedef struct 
{
	int func;
	char usr[20];
	char mesg[512];
}__cli_mesg;

__cli_mesg buf;
char mesgrev[2048];
pthread_t tid;
int newfd;
int flag = 0;
int pflag =0;
sem_t sem1;
sem_t sem2;
Setstaff setstaff;

int main(int argc, const char* argv[])
{
	newfd = socket(AF_INET, SOCK_STREAM, 0);
	if(newfd < 0)
	{
		perror("socket");
		return -1;
	}
	if(sem_init(&sem1, 0, 1)<0)
	{
		perror("sem_init");
		return -1;
	}
	if(sem_init(&sem2, 0, 0)<0)
	{
		perror("sem_init");
		return -1;                                                                                                                                                                     
	}
	int reuse = 1;
	int len = sizeof(reuse);
	if(setsockopt(newfd, SOL_SOCKET, SO_REUSEADDR, &reuse,len) < 0)
	{
		perror("setsockopt failed");
		exit(1);
	}
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(2021);
	sin.sin_addr.s_addr = inet_addr("192.168.1.250");
	if(connect(newfd, (void*)&sin, sizeof(sin)) < 0)
	{
		perror("connect");
		exit(1);
	}
	printf("连接服务器成功\n");	
	if(pthread_create(&tid,NULL,recv_ser_msg,NULL) < 0)
	{
		perror("pthread create error");
		return -1;
	}
	while(1)
	{
		view();
	}
	return 0;
}





void* recv_ser_msg(void* arg)
{
	int ret = -1;
	while(1)
	{

		sem_wait(&sem2);
		bzero(mesgrev,sizeof(mesgrev));
		do
		{
			ret = recv(newfd, mesgrev, sizeof(mesgrev),0);
		}while(ret < 0 && errno == EINTR);
		if(ret == 0)
		{
			fprintf(stderr,"服务器断开连接\n");
			close(newfd);
			exit(1);
		}
		else if(ret < 0)
		{
			perror("send");
			exit(2);
		}
		if(!strcmp(mesgrev,"用户名已存在\n"))
		{
			flag = 1;
		}
		else if(!strcmp(mesgrev,"登录成功\n"))
		{
			flag = 2;
		}
		else if(!strcmp(mesgrev,"登录管理\n"))
		{
			flag = 3;
		}
		else if(!strcmp(mesgrev,"用户不存在\n"))
		{
			flag = 4;
		}
		else if(!strcmp(mesgrev,"职员表空\n"))
		{
			flag = 5;
		}
		if(pflag == 0)
			fprintf(stderr,"%s",mesgrev);
		sem_post(&sem1);
	}

}

void fuckclear()
{
	printf("按回车键继续\n");
	getchar();
}



void view(void)
{
	int ret = -1;
	while(1)
	{
		fuckclear();
		system("clear");
		
		int i,j,k;
		printf("1.注册, 2.登录, 3.注销. 0.退出\nplease input >");
		fflush(NULL);
		scanf("%d",&i);
		while(getchar()!='\n');
		switch(i)
		{
		case 1:
			
			sem_wait(&sem1);
			system("clear");
			buf.func = REGISTER;
			printf("请输入你要注册的用户名:\n");
			scanf("%s",buf.usr);
			while(getchar()!='\n');
			printf("请输入你的密码:\n");
			scanf("%s",buf.mesg);
			while(getchar()!='\n');
		    do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}                                                             
			sem_post(&sem2);

	

			sem_wait(&sem1);
			if(flag == 1)
			{
				flag =0;
				sem_post(&sem1);
				break;
			}
			printf("please input your name:\n");
			scanf("%s",setstaff[0]);
			while(getchar()!='\n');
			printf("please input your address:\n");	
			scanf("%s",setstaff[1]);
			while(getchar()!='\n');
			printf("please input your age:\n");		
			scanf("%s",setstaff[2]);
			while(getchar()!='\n');
			printf("please input your telephone:\n");		
			scanf("%s",setstaff[3]);
			while(getchar()!='\n');
			printf("please input your salary:\n");		
			scanf("%s",setstaff[4]);
			while(getchar()!='\n');
			printf("please input your department:\n");		
			scanf("%s",setstaff[5]);
			while(getchar()!='\n');
			printf("please input your position:\n");	
			scanf("%s",setstaff[6]);
			while(getchar()!='\n');
			printf("please input your number:\n");	
			scanf("%s",setstaff[7]);
			while(getchar()!='\n');
			printf("please input your id:\n");	
			scanf("%s",setstaff[8]);
			while(getchar()!='\n');
			
			sprintf(buf.mesg,"%s:%s:%s:%s:%s:%s:%s:%s:%s"\
					,setstaff[0],setstaff[1],setstaff[2],setstaff[3],\
					setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);

			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}  
			sem_post(&sem2);
			sem_wait(&sem1);
			sem_post(&sem1);
			break;
		case 2:
			sem_wait(&sem1);
			system("clear");
			buf.func = LOGIN;
			printf("请输入你的用户名:\n");
			scanf("%s",buf.usr);
			while(getchar()!='\n');
			printf("请输入你的密码:\n");
			scanf("%s",buf.mesg);
			while(getchar()!='\n');
		    do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}                                                             
			sem_post(&sem2);

	

			sem_wait(&sem1);
			if(flag == 2)
			{
				flag = 0;
				sem_post(&sem1);
				goto USER;
			}
			else if(flag == 3)
			{
				flag = 0;
				sem_post(&sem1);
				goto ADMIN;
			}
			sem_post(&sem1);
			break;
		case 3:
			sem_wait(&sem1);
			system("clear");
			buf.func = CANCEL;
			printf("请输入你的用户名:\n");
			scanf("%s",buf.usr);
			while(getchar()!='\n');
			printf("请输入你的密码:\n");
			scanf("%s",buf.mesg);
			while(getchar()!='\n');
		    do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}                                                             
			sem_post(&sem2);
			sem_wait(&sem1);
			sem_post(&sem1);
			break;
		case 0:
			exit(1);
			break;
		default:
			printf("输入有误\n");
			break;
		}
	}
	exit(4);
USER:
	while(1)
	{
		fuckclear();
		system("clear");
		int i,ret;
		printf("1.打印用户信息, 2.修改用户信息, 0.退出\nplease input >");
		fflush(NULL);
		scanf("%d",&i);
		while(getchar()!='\n');
		
		switch(i)
		{
		case 1:
			sem_wait(&sem1);
			pflag =1;
			buf.func = USR_PRINT;
			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}
			sem_post(&sem2);
			sem_wait(&sem1);
			ret =setstaffinfo();
			if(ret < 0)
			{
				printf("错误!!!\n");
				exit(4);
			}
	
			printf("name:%s\naddress:%s\nage:%s\ntelephone:%s\nsalary:%s\ndepartment:%s\nposition:%s\nnumber:%s\nid:%s\n",\
					setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);
			pflag = 0;
			sem_post(&sem1);
			break;
		case 2:
			sem_wait(&sem1);
			buf.func = USR_PRINT;
			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			} 
			pflag = 1;
			sem_post(&sem2);
			sem_wait(&sem1);
			ret =setstaffinfo();
			if(ret < 0)
			{
				printf("错误!!!\n");
				exit(4);
			}
			printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n",\
					setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);
			printf("请输入修改项的序号:\n");
			scanf("%d",&i);
			while(getchar()!='\n');
			if(i > 9 || i < 1)
			{
				printf("输入错误\n");
				break;
			}
			printf("请输入修改内容\n");
			scanf("%s",setstaff[i-1]);
			while(getchar()!='\n');
			buf.func = USR_CHG;
			sprintf(buf.mesg,"%s:%s:%s:%s:%s:%s:%s:%s:%s"\
					,setstaff[0],setstaff[1],setstaff[2],setstaff[3],\
					setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);

			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}  
			pflag=0;
			sem_post(&sem1);
			break;
		case 0:
			exit(1);
			break;
		default:
			printf("输入错误\n");
			break;
		}
	}
exit(4);
ADMIN:
	while(1)
	{
		fuckclear();
		system("clear");
		int i,ret;
		printf("1.添加员工, 2.删除员工, 3.查找员工, 4.打印全部信息, 5.修改员工, 0.退出\nplease input >");
		fflush(NULL);
		scanf("%d",&i);
		while(getchar()!='\n');
		
		switch(i)
		{			
		case 1:
			sem_wait(&sem1);
			printf("please input new name:\n");
			scanf("%s",setstaff[0]);
			while(getchar()!='\n');
			printf("please input new address:\n");	
			scanf("%s",setstaff[1]);
			while(getchar()!='\n');
			printf("please input new age:\n");		
			scanf("%s",setstaff[2]);
			while(getchar()!='\n');
			printf("please input new telephone:\n");		
			scanf("%s",setstaff[3]);
			while(getchar()!='\n');
			printf("please input new salary:\n");		
			scanf("%s",setstaff[4]);
			while(getchar()!='\n');
			printf("please input new department:\n");		
			scanf("%s",setstaff[5]);
			while(getchar()!='\n');
			printf("please input new position:\n");	
			scanf("%s",setstaff[6]);
			while(getchar()!='\n');
			printf("please input new number:\n");	
			scanf("%s",setstaff[7]);
			while(getchar()!='\n');
			printf("please input new id:\n");	
			scanf("%s",setstaff[8]);
			while(getchar()!='\n');
			printf("please input new usrname:\n");
			scanf("%s",buf.usr);
			while(getchar()!='\n');
			sprintf(buf.mesg,"%s:%s:%s:%s:%s:%s:%s:%s:%s"\
					,setstaff[0],setstaff[1],setstaff[2],setstaff[3],\
					setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);
			buf.func = ADMIN_ADD;
			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}  
			sem_post(&sem2);
			sem_wait(&sem1);
			sem_post(&sem1);

			break;
		case 2:
			sem_wait(&sem1);
			printf("请输入删除方式 1.按电话删除, 2.按id删除, 0.返回\nplease input >");
			fflush(NULL);
			scanf("%d",&i);
			while(getchar()!='\n');
			switch(i)
			{
			case 1:
				printf("请输入电话:\n");
				scanf("%s",buf.mesg);
				while(getchar()!='\n');
				buf.func = ADMIN_DEL_TEL;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				sem_post(&sem1);
				break;
		
			
			case 2:
				printf("请输入id:\n");
				scanf("%s",buf.mesg);
				while(getchar()!='\n');
				buf.func = ADMIN_DEL_ID;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				break;
			case 0:
				sem_post(&sem1);
				break;
			default:
				printf("输入错误\n");
				pflag = 0;
				sem_post(&sem1);
				break;
			}
			break;
		
		
		case 3:
			sem_wait(&sem1);
			pflag = 1;
			printf("请输入查找方式 1.按电话查找, 2.按id查找, 3.按工号查找, 0.返回\nplease input >");
			fflush(NULL);
			scanf("%d",&i);
			while(getchar()!='\n');
			switch(i)
			{
			case 1:
				printf("请输入电话:\n");
				scanf("%s",buf.mesg);
				while(getchar()!='\n');
				buf.func = ADMIN_FIND_TEL;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				if(flag == 4)
				{
					flag = 0;
					printf("查无此人\n");	
					sem_post(&sem1);
					pflag = 0;
					break;
				}
				ret =setstaffinfo();
				if(ret < 0)
				{
					printf("错误!!!\n");
					exit(4);
				}
				strcpy(buf.usr,&(mesgrev[ret+1]));
				printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
						setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
				pflag = 0;
				sem_post(&sem1);
				break;
			case 2:
				printf("请输入id:\n");
				scanf("%s",buf.mesg);
				while(getchar()!='\n');
				buf.func = ADMIN_FIND_ID;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				if(flag == 4)
				{
					flag = 0;
					printf("查无此人\n");	
					sem_post(&sem1);
					pflag = 0;
					break;
				}
				ret =setstaffinfo();
				if(ret < 0)
				{
					printf("错误!!!\n");
					exit(4);
				}
				strcpy(buf.usr,&(mesgrev[ret+1]));
				printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
						setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
				sem_post(&sem1);
				pflag = 0;

				break;
			case 3:
				printf("请输入工号:\n");
				scanf("%s",buf.mesg);
				while(getchar()!='\n');
				buf.func = ADMIN_FIND_NUM;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				if(flag == 4)
				{
					flag = 0;
					printf("查无此人\n");
					sem_post(&sem1);
					pflag = 0;
					break;
				}
				ret =setstaffinfo();
				if(ret < 0)
				{
					printf("错误!!!\n");
					exit(4);
				}
				strcpy(buf.usr,&(mesgrev[ret+1]));
				printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
						setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
				sem_post(&sem1);
				pflag = 0;
				break;
			case 0:
				pflag = 0;
				sem_post(&sem1);
				break;
			default:
				printf("输入错误\n");
				pflag = 0;
				sem_post(&sem1);
				break;

			}
			break;
		
			
		case 4:
			i = 0;
			sem_wait(&sem1);
			buf.func = ADMIN_PRINT;
			do
			{
				ret = send(newfd,&buf,sizeof(buf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{                                                             
				perror("send");
				exit(3);
			}  
			printf("name      address   age       telephone salary    department     position  number    id        usrname\n");
			sem_post(&sem2);
			sem_wait(&sem1);
			if(flag == 5)
			{
				printf("列表为空");
				sem_post(&sem1);
				flag = 0;
				break;
			}
			sem_post(&sem1);
			break;			
		case 5:
			sem_wait(&sem1);
			printf("请输入修改方式  1.列表修改, 2.查找修改, 0.返回\nplease input >");
			fflush(NULL);
			scanf("%d",&i);
			while(getchar()!='\n');
			switch(i)
			{
			case 1:
				i = 0;
				buf.func = ADMIN_PRINT;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				printf("name      address   age       telephone salary    department     position  number    id        usrname\n");
				sem_post(&sem2);
				sem_wait(&sem1);
				if(flag == 5)
				{
					printf("列表为空");
					sem_post(&sem1);
					flag = 0;
					break;
				}
				pflag = 1;
				printf("请输入你要修改信息的用户名");
				scanf("%s",buf.usr);
				while(getchar()!='\n');
				buf.func = ADMIN_FIND_USRNAME;
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				sem_post(&sem2);
				sem_wait(&sem1);
				if(flag == 4)
				{
					flag = 0;
					printf("查无此人\n");
					sem_post(&sem1);
					pflag = 0;
					break;
				}
				ret =setstaffinfo();
				if(ret < 0)
				{
					printf("错误!!!\n");
					exit(4);
				}
				strcpy(buf.usr,&(mesgrev[ret+1]));
				printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
						setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
				printf("请输入修改项的序号:\n");
				scanf("%d",&i);
				while(getchar()!='\n');
				if(i > 9 || i < 1)
				{
					printf("输入错误\n");
					pflag = 0;
					break;
				}
				printf("请输入修改内容\n");
				scanf("%s",setstaff[i-1]);
				while(getchar()!='\n');
				buf.func = ADMIN_CHG;
				sprintf(buf.mesg,"%s:%s:%s:%s:%s:%s:%s:%s:%s"\
						,setstaff[0],setstaff[1],setstaff[2],setstaff[3],\
						setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				pflag=0;
				sem_post(&sem1);
				break;
			case 2:
				pflag = 1;
				printf("请输入查找方式 1.按电话查找, 2.按id查找, 3.按工号查找, 0.返回\nplease input >");
				fflush(NULL);
				scanf("%d",&i);
				while(getchar()!='\n');
				switch(i)
				{
				case 1:
					printf("请输入电话:\n");
					scanf("%s",buf.mesg);
					while(getchar()!='\n');
					buf.func = ADMIN_FIND_TEL;
					do
					{
						ret = send(newfd,&buf,sizeof(buf),0);
					}while(ret < 0 && errno == EINTR);
					if(ret < 0)
					{                                                             
						perror("send");
						exit(3);
					}  
					sem_post(&sem2);
					sem_wait(&sem1);
					if(flag == 4)
					{
					
						printf("查无此人\n");	
						sem_post(&sem1);
						pflag = 0;
						break;
					}
					ret =setstaffinfo();
					if(ret < 0)
					{
						printf("错误!!!\n");
						exit(4);
					}
					strcpy(buf.usr,&(mesgrev[ret+1]));
					printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
							setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
					pflag = 0;
					sem_post(&sem1);
					break;
				case 2:
					printf("请输入id:\n");
					scanf("%s",buf.mesg);
					while(getchar()!='\n');
					buf.func = ADMIN_FIND_ID;
					do
					{
						ret = send(newfd,&buf,sizeof(buf),0);
					}while(ret < 0 && errno == EINTR);
					if(ret < 0)
					{                                                             
						perror("send");
						exit(3);
					}  
					sem_post(&sem2);
					sem_wait(&sem1);
					if(flag == 4)
					{
					
						printf("查无此人\n");	
						sem_post(&sem1);
						pflag = 0;
						break;
					}
					ret =setstaffinfo();
					if(ret < 0)
					{
						printf("错误!!!\n");
						exit(4);
					}
					strcpy(buf.usr,&(mesgrev[ret+1]));
					printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
							setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
					sem_post(&sem1);
					pflag = 0;

					break;
				case 3:
					printf("请输入工号:\n");
					scanf("%s",buf.mesg);
					while(getchar()!='\n');
					buf.func = ADMIN_FIND_NUM;
					do
					{
						ret = send(newfd,&buf,sizeof(buf),0);
					}while(ret < 0 && errno == EINTR);
					if(ret < 0)
					{                                                             
						perror("send");
						exit(3);
					}  
					sem_post(&sem2);
					sem_wait(&sem1);
					if(flag == 4)
					{
					
						printf("查无此人\n");
						sem_post(&sem1);
						pflag = 0;
						break;
					}
					ret =setstaffinfo();
					if(ret < 0)
					{
						printf("错误!!!\n");
						exit(4);
					}
					strcpy(buf.usr,&(mesgrev[ret+1]));
					printf("1.name:%s\n2.address:%s\n3.age:%s\n4.telephone:%s\n5.salary:%s\n6.department:%s\n7.position:%s\n8.number:%s\n9.id:%s\n该用户的用户名为:%s\n",\
							setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
					sem_post(&sem1);
					pflag = 0;
					break;
				case 0:
					pflag = 0;
					flag =4;
					sem_post(&sem1);
					break;
				default:
					printf("输入错误\n");
					pflag = 0;
					sem_post(&sem1);
					break;

				}
				if(flag == 4)	
				{
					flag = 0;
					break;
				}
				printf("请输入修改项的序号:\n");
				scanf("%d",&i);
				while(getchar()!='\n');
				if(i > 9 || i < 1)
				{
					printf("输入错误\n");
					break;
				}
				printf("请输入修改内容\n");
				scanf("%s",setstaff[i-1]);
				while(getchar()!='\n');
				buf.func = ADMIN_CHG;
				sprintf(buf.mesg,"%s:%s:%s:%s:%s:%s:%s:%s:%s"\
						,setstaff[0],setstaff[1],setstaff[2],setstaff[3],\
						setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8]);
				do
				{
					ret = send(newfd,&buf,sizeof(buf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{                                                             
					perror("send");
					exit(3);
				}  
				pflag=0;
				break;
			case 0:
				sem_post(&sem1);
				pflag = 0;
				break;
			default:
				printf("输入错误\n");
				pflag = 0;
				sem_post(&sem1);
				break;
			}
			break;
		case 0:
			exit(1);
			break;
		default:
			printf("输入错误\n");
			break;
		}




	}//while
	exit(4);
}

int setstaffinfo()          
{                                                       
	int i=0,j=0,k=0;                                                    
	bzero(setstaff,sizeof(Setstaff));                   
	mesgrev[2047]='\0';                              
	for(i=0; mesgrev[i]!='\0' && mesgrev[i]!='|'; i++)                    
	{                                                   
		if(mesgrev[i] == ':')                          
		{                                               
			setstaff[k][j]='\0';                        
			j=0;                                        
			k++;                                        
			continue;                                   
		}                                               
		setstaff[k][j]=mesgrev[i];                     
		j++;                                            
	}                                                   
	setstaff[k][j]='\0';                                
	if(k != 8 && k != 0 )                                          
	{                                                   
		fprintf(stderr,"setstaffinfo type error\n");    
		return -1;                                      
	}                                                                                                    
	return i;                                           
}                                                       


