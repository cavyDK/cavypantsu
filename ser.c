#include "ser.h"

int main(int argc,const char* argv[])
{
	system("clear");
	int ret = -1;//校验位


	//创建打开数据库
	ret = sqlite3_open("./sq.db",&db);
	if(ret != 0)
	{
		fprintf(stderr,"数据库打开失败\n");
		fprintf(stderr,"%s\n",sqlite3_errmsg(db));
		return -1;
	}
	printf("数据库打开成功\n");

	//创建用户表
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists usrinfo\
			(usrname char primary key, password char)");
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return -1;
	}
	printf("用户表已初始化\n");
	//创建信息表	
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists staffinfo\
			(name char, address char, age int, telephone char,\
			 salary int, department char, position char,\
			 number int, id char, usrname char primary key)");
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return -1;
	}

	printf("信息表已初始化\n");
	//创建管理员列表
	bzero(sql,sizeof(sql));
	sprintf(sql,"create table if not exists adminlist(usrname char primary key)");
	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != 0)
	{
		fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
		return -1;
	}

	printf("权限表已初始化\n");
	//创建套接字
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0)

	{
		perror("socket create fail");
		return -1;
	}
	int reuse = 1;//本地接口快速重用;

	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,&reuse,sizeof(int));


	struct sockaddr_in sin; //服务器信息结构体
	sin.sin_family = AF_INET;
	sin.sin_port = htons(2021);
	if((inet_pton(AF_INET, "0.0.0.0", &(sin.sin_addr))) < 0)
	{
		perror("pton error");
		return -1;
	}
	//printf("%s-%s-%d\n",__FILE__,__func__,__LINE__);
	//绑定
	if(bind(sfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("bind");
		exit(1);
	}
	printf("绑定成功\n");
	//监听
	if(listen(sfd, 3) < 0)
	{
		perror("listen");
		exit(1);
	}
	printf("服务器启动成功\n");


	struct sockaddr_in cin; //客户端信息结构体
	socklen_t len = sizeof(cin);
	int cntfd = -1;
	char clientip[20] = "" ; //网络字节序转换后的客户端ip
	while(1)
	{
		cntfd = accept(sfd, (void*)&cin, &len);
		if(cntfd < 0)
		{
			perror("accept");
			exit(1);
		}
		memset(clientip,0,20);
		inet_ntop(AF_INET, &(cin.sin_addr.s_addr), clientip, 20);
		printf("客户端连接成功-%d:%s\n",cntfd,clientip);

		//线程传参结构体
		__cli_info cliInfo;
		cliInfo.newfd = cntfd;
		cliInfo.cin = cin;


		pthread_t tid = -1;
		if(pthread_create(&tid, NULL, recv_cli_msg, (void*)&cliInfo) < 0)
		{
			perror("pthread create error");
			close(cntfd);
		}
	}
	closepth(sfd);
	return 0;	

}

void* recv_cli_msg(void* arg)
{
	int ret = -1;
	pthread_detach(pthread_self());	 //线程分离
	__cli_info cli = *(__cli_info*)arg;
	int newfd = cli.newfd;
	struct sockaddr_in cin = cli.cin;
	char ip[20] = "";
	inet_ntop(AF_INET, &cin.sin_addr, ip, 20); 
//	int port = ntohs(cin.sin_port);
	char** resultp = NULL;
	int row,column;
	char clipwd[20];
	char sql[256]= ""; //sqlite3指令字符串
	char* errmsg;       //sqlite3错误信息
	__cli_mesg buf;     //消息结构体
	char sendbuf[2048]; //发送给客户端消息
	Setstaff setstaff;
	char constusr[20];  //登录用户名

	while(1)
	{	
		recvmesg(&buf,newfd,ip);
		switch(buf.func)
		{
	
		//注册
		case REGISTER:
			bzero(sql,sizeof(sql));
			sprintf(sql,"select usrname from usrinfo where usrname=\"%s\"",buf.usr);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				break;
			}
			if(row != 0)
			{
				printf("[%d:%s]用户名已存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户名已存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}
				break;
			}
			sqlite3_free_table(resultp);
			bzero(sql,sizeof(sql));
			sprintf(sql,"insert into usrinfo values(\"%s\",\"%s\")",buf.usr, buf.mesg);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
			{
				fprintf(stderr, "sqlite3_exec:%s\n", errmsg);
				printf("[%d:%s]注册失败\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"注册失败\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}
				break;
			}
	
			printf("[%d:%s]用户表添加\n", newfd, ip);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"用户表添加\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}


			strcpy(constusr,buf.usr);
			recvmesg(&buf,newfd,ip);
			ret = setstaffinfo(buf,&setstaff);
			if(ret < 0)
			{
				fprintf(stderr,"[%d,%s]setstaffinfo error\n",newfd,ip);
				bzero(sql,sizeof(sql));
				sprintf(sql,"delete from usrinfo where usrname=\"%s\"",constusr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec:%s-%d\n",errmsg,__LINE__);
					break;

				}
				closepth(newfd);
			}			
			if(!strcmp(setstaff[0],REG_KEY))//判断是否注册管理
			{
				bzero(sql,sizeof(sql));
				sprintf(sql,"insert into adminlist values(\"%s\")",constusr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
					printf("[%d:%s]注册失败\n", newfd, ip);
					bzero(sendbuf,sizeof(sendbuf));
					sprintf(sendbuf,"注册失败\n");
					do
					{
						ret = send(newfd,sendbuf,strlen(sendbuf),0);
					}while(ret < 0 && errno == EINTR);
					if(ret < 0)
					{
						perror("send");
					}
					closepth(newfd);
					break;
				}

				printf("[%d:%s]注册管理\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"注册管理\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			//注册普通用户
			bzero(sql,sizeof(sql));
			sprintf(sql,"insert into staffinfo values\
					(\"%s\",\"%s\",%s,\"%s\",%s,\
					 \"%s\",\"%s\",%s,\"%s\",\"%s\")"\
					,setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],constusr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
			{

				fprintf(stderr,"sqlite3_exec:%s-%d\n",errmsg,__LINE__);
				if(!strcmp(errmsg,"column usrname is not unique"))
				{
					printf("[%d:%s]注册成功,登记信息失败(管理员已登记你的信息)\n", newfd, ip);
					bzero(sendbuf,sizeof(sendbuf));
					sprintf(sendbuf,"注册成功,登记信息失败(管理员已登记你的信息)\n");
					do
					{
						ret = send(newfd,sendbuf,strlen(sendbuf),0);
					}while(ret < 0 && errno == EINTR);
					if(ret < 0)
					{
						perror("send");
						closepth(newfd);
						break;
					}
					break;

				}
				bzero(sql,sizeof(sql));
				sprintf(sql,"delete from usrinfo where usrname=\"%s\"",constusr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
				{
					fprintf(stderr,"sqlite3_exec:%s-%d\n",errmsg,__LINE__);
				}
				printf("[%d:%s]注册失败\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"注册失败\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}
				break;
			}
			printf("[%d:%s]注册成功\n", newfd, ip);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"注册成功\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
				break;
			}
			break;

		//登录
		case LOGIN:


			bzero(sql,sizeof(sql));
			sprintf(sql,"select password from usrinfo where usrname=\"%s\"",buf.usr);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg) != 0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				break;
			}
			if(row == 0)
			{	
				printf("[%d:%s]用户名不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户名不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}
				break;
			}
			memset(clipwd,0,sizeof(clipwd));
			sprintf(clipwd,"%s",resultp[1]);
			sqlite3_free_table(resultp);
			if(!strcmp(buf.mesg,clipwd))
			{
				strcpy(constusr,buf.usr);
				goto USER;
			}

			printf("[%d:%s]登录失败,密码错误\n", newfd, ip);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"登录失败,密码错误\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
				break;
			}



			break;
	
			
			
		//注销
		case CANCEL:
			bzero(sql,sizeof(sql));
			sprintf(sql,"select password from usrinfo where usrname=\"%s\"",buf.usr);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg) != 0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				break;
			}
			if(row == 0)
			{	
				printf("[%d:%s]用户名不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户名不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}

				break;
			}
			bzero(clipwd,sizeof(clipwd));
			sprintf(clipwd,"%s",resultp[1]);
			sqlite3_free_table(resultp);
			if(!strcmp(buf.mesg,clipwd))
			{
				bzero(sql,sizeof(sql));
				sprintf(sql,"delete from usrinfo where usrname=\"%s\"",buf.usr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
					closepth(newfd);

				}
				bzero(sql,sizeof(sql));
				sprintf(sql,"delete from staffinfo where usrname=\"%s\"",buf.usr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec:%s\n",errmsg);	
					closepth(newfd);
	
				}

				printf("[%d:%s]注销成功\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"注销成功\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}

				break;
			}	
			printf("[%d:%s]注销失败,密码错误\n", newfd, ip);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"注销失败\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
				break;
			}

			break;
		default:
			printf("[%d:%s]客户端错误\n", newfd, ip);
			closepth(newfd);
			break;
		}
	}
	closepth(newfd);
	
USER://登录后区域
	//判断是否管理登录
	bzero(sql,sizeof(sql));
	sprintf(sql,"select usrname from adminlist where usrname =\"%s\"",constusr);
	if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
	{
		fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
		closepth(newfd);
	}
	sqlite3_free_table(resultp);
	if(row != 0)
	{
		goto ADMIN;
	}
	//用户登录成功
	printf("[%d:%s]登录成功\n", newfd, ip);
	bzero(sendbuf,sizeof(sendbuf));
	sprintf(sendbuf,"登录成功\n");
	do
	{
		ret = send(newfd,sendbuf,strlen(sendbuf),0);
	}while(ret < 0 && errno == EINTR);
	if(ret < 0)
	{
		perror("send");
		closepth(newfd);
			}
	while(1)
	{	
		recvmesg(&buf, newfd, ip);
		switch(buf.func)
		{
		case USR_PRINT:
			sendstaffinfo(constusr,newfd,ip);
			break;
		case USR_CHG:
			ret = setstaffinfo(buf,&setstaff);
			if(ret < 0)
			{
				fprintf(stderr,"[%d,%s]setstaffinfo error\n",newfd,ip);
				closepth(newfd);
			}

			sprintf(sql,"UPDATE staffinfo SET name=\"%s\" , address=\"%s\" , age=%s , telephone=\"%s\"\
					, salary=%s , department=\"%s\" ,  position=\"%s\"\
					, number=%s , id=\"%s\" where usrname=\"%s\"",setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],constusr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
			{
				fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
				break;
			}
			break;
		default:
			printf("[%d:%s]客户端错误\n", newfd, ip);
			closepth(newfd);
			break;
		}
	}
	closepth(newfd);

ADMIN://登录管理后区域
	printf("[%d:%s]登录管理\n", newfd, ip);
	bzero(sendbuf,sizeof(sendbuf));
	sprintf(sendbuf,"登录管理\n");
	do
	{
		ret = send(newfd,sendbuf,strlen(sendbuf),0);
	}while(ret < 0 && errno == EINTR);
	if(ret < 0)
	{
		perror("send");
		closepth(newfd);
	}
	printf("%s-%s-%d\n",__FILE__,__func__,__LINE__);
	
	//管理操作
	while(1)
	{
		int i; 
		recvmesg(&buf, newfd, ip);
		switch(buf.func)
		{
		case ADMIN_ADD:
			ret = setstaffinfo(buf,&setstaff);
			if(ret < 0)
			{
				fprintf(stderr,"[%d,%s]setstaffinfo error\n",newfd,ip);
				closepth(newfd);
			}
			sprintf(sql,"insert into staffinfo values\
					(\"%s\",\"%s\",%s,\"%s\",%s,\
					 \"%s\",\"%s\",%s,\"%s\",\"%s\")"\
					,setstaff[0],setstaff[1],setstaff[2],setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
			{
			
				fprintf(stderr, "sqlite3_exec:%s\n", errmsg);
				printf("[%d:%s]添加失败\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"添加失败\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
					break;
				}
				break;
			}
			printf("[%d:%s]添加成功\n", newfd, ip);
			bzero(sendbuf,sizeof(sendbuf));	
			sprintf(sendbuf,"添加成功\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
				break;
			}
			break;
		case ADMIN_DEL_TEL:

			//根据电话查找
			bzero(sql,sizeof(sql));
			sprintf(sql,"select usrname from staffinfo where telephone=\"%s\"",buf.mesg);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			bzero(constusr,sizeof(constusr));
			strcpy(constusr,resultp[1]);
			sqlite3_free_table(resultp);
			
			//根据电话删除
				sprintf(sql,"delete from usrinfo where usrname=\"%s\"",constusr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec(该用户未创建账号):%s\n",errmsg);
				}
				bzero(sql,sizeof(sql));
				sprintf(sql,"delete from staffinfo where usrname=\"%s\"",constusr);
				if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
				{
					fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
					break;

				}

				printf("[%d:%s]删除成功(usrname=%s)\n", newfd, ip,constusr);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"删除成功\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
			break;
		case ADMIN_DEL_ID:
			//根据id查找
			bzero(sql,sizeof(sql));
			sprintf(sql,"select usrname from staffinfo where id=\"%s\"",buf.mesg);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			bzero(constusr,sizeof(constusr));
			strcpy(constusr,resultp[1]);
			sqlite3_free_table(resultp);

			//根据id删除
			sprintf(sql,"delete from usrinfo where usrname=\"%s\"",constusr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
			{
				fprintf(stderr,"sqlite3_exec(该用户未创建账号):%s\n",errmsg);
			}
			bzero(sql,sizeof(sql));
			sprintf(sql,"delete from staffinfo where usrname=\"%s\"",constusr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) !=0 )
			{
				fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
				break;

			}

			printf("[%d:%s]删除成功(usrname=%s)\n", newfd, ip,constusr);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"删除成功\n");
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}
			break;
		case ADMIN_FIND_TEL:
			//根据电话查找
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from staffinfo where telephone=\"%s\"",buf.mesg);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			strcpy(constusr,resultp[19]);
			printf("[%d:%s]查找成功(usrname=%s)\n", newfd, ip,constusr);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"%s:%s:%s:%s:%s:%s:%s:%s:%s|%s",\
					resultp[10],resultp[11],resultp[12],resultp[13],\
					resultp[14],resultp[15],resultp[16],resultp[17],resultp[18],resultp[19]);
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}	
			
			sqlite3_free_table(resultp);
			break;
		case ADMIN_FIND_ID:
			//根据id查找
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from staffinfo where id=\"%s\"",buf.mesg);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			strcpy(constusr,resultp[19]);
			printf("[%d:%s]查找成功(usrname=%s)\n", newfd, ip,constusr);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"%s:%s:%s:%s:%s:%s:%s:%s:%s|%s",\
					resultp[10],resultp[11],resultp[12],resultp[13],\
					resultp[14],resultp[15],resultp[16],resultp[17],resultp[18],resultp[19]);
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}	
			
			sqlite3_free_table(resultp);
			break;
		case ADMIN_FIND_USRNAME:
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from staffinfo where usrname=\"%s\"",buf.usr);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			strcpy(constusr,resultp[19]);
			printf("[%d:%s]查找成功(usrname=%s)\n", newfd, ip,constusr);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"%s:%s:%s:%s:%s:%s:%s:%s:%s|%s",\
					resultp[10],resultp[11],resultp[12],resultp[13],\
					resultp[14],resultp[15],resultp[16],resultp[17],resultp[18],resultp[19]);
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}	
			
			sqlite3_free_table(resultp);	
			break;
		case ADMIN_FIND_NUM:
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from staffinfo where number=\"%s\"",buf.mesg);
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]用户不存在\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"用户不存在\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			strcpy(constusr,resultp[19]);
			printf("[%d:%s]查找成功(usrname=%s)\n", newfd, ip,constusr);
			bzero(sendbuf,sizeof(sendbuf));
			sprintf(sendbuf,"%s:%s:%s:%s:%s:%s:%s:%s:%s|%s",\
					resultp[10],resultp[11],resultp[12],resultp[13],\
					resultp[14],resultp[15],resultp[16],resultp[17],resultp[18],resultp[19]);
			do
			{
				ret = send(newfd,sendbuf,strlen(sendbuf),0);
			}while(ret < 0 && errno == EINTR);
			if(ret < 0)
			{
				perror("send");
				closepth(newfd);
			}	
			
			sqlite3_free_table(resultp);	
			break;
		case ADMIN_PRINT:
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from staffinfo");
			if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg)!=0)
			{
				fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
				closepth(newfd);
			}
			if(row == 0)
			{
				printf("[%d:%s]职员表空\n", newfd, ip);
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"职员表空\n");
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
				break;
			}
			for(i=1;i<=row;i++)
			{
				bzero(sendbuf,sizeof(sendbuf));
				sprintf(sendbuf,"%-10s%-10s%-10s%-10s%-10s%-15s%-10s%-10s%-10s%s\n",\
						resultp[10*i+0],resultp[10*i+1],resultp[10*i+2],resultp[10*i+3],\
						resultp[10*i+4],resultp[10*i+5],resultp[10*i+6],resultp[10*i+7],resultp[10*i+8],resultp[10*i+9]);
				do
				{
					ret = send(newfd,sendbuf,strlen(sendbuf),0);
				}while(ret < 0 && errno == EINTR);
				if(ret < 0)
				{
					perror("send");
					closepth(newfd);
				}
			}
			sqlite3_free_table(resultp);
			break;
		case ADMIN_CHG:
			ret = setstaffinfo(buf,&setstaff);
			if(ret < 0)
			{
				fprintf(stderr,"[%d,%s]setstaffinfo error\n",newfd,ip);
				closepth(newfd);
			}

			sprintf(sql,"UPDATE staffinfo SET name=\"%s\" , address=\"%s\" , age=%s , telephone=\"%s\" \
					, salary=%s , department=\"%s\" ,  position=\"%s\"\
					, number=%s , id=\"%s\" where usrname=\"%s\"",setstaff[0],setstaff[1],setstaff[2],\
					setstaff[3],setstaff[4],setstaff[5],setstaff[6],setstaff[7],setstaff[8],buf.usr);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != 0)
			{
				fprintf(stderr,"sqlite3_exec:%s\n",errmsg);
				closepth(newfd);
			}
			break;
		default:
			printf("[%d:%s]客户端错误\n", newfd, ip);
			closepth(newfd);
			break;
		}
	}
	closepth(newfd);
}//线程函数结束



void closepth(int fd)
{
	close(fd);
	pthread_exit(NULL);

}


int setstaffinfo(__cli_mesg buf,Setstaff* set)
{
	int i=0,j=0,k=0;
	Setstaff setstaff;
	bzero(setstaff,sizeof(Setstaff));
	buf.mesg[511]='\0';
	for(i=0; buf.mesg[i]!='\0'; i++)
	{
		if(buf.mesg[i] == ':')
		{
			setstaff[k][j]='\0';
			j=0;
			k++;
			continue;
		}
		setstaff[k][j]=buf.mesg[i];
		j++;
	}
	setstaff[k][j]='\0';
	if(k != 8)
	{
		fprintf(stderr,"setstaffinfo type error\n");
		return -1;
	}
	for(i=0;i<9;i++)
	{
		strcpy((*set)[i],setstaff[i]);
	}

	return 0;
}


void recvmesg(__cli_mesg* buf, int newfd, char* ip)
{
	int ret = -1;
	memset(buf,0,sizeof(__cli_mesg));
	do
	{
		ret = recv(newfd, buf, sizeof(__cli_mesg), 0);
	}while(ret < 0 && errno == EINTR);
	if(ret < 0)
	{
		perror("recv");
		closepth(newfd);
	}
	else if(ret == 0)
	{
		fprintf(stderr,"[%d:%s]断开连接\n",newfd,ip);
		closepth(newfd);
	}
}




void sendstaffinfo(char* constusr,int newfd,char* ip)
{
	char sql[256];
	char** resultp;
	int row,column,ret;
	char sendbuf[2048];
	bzero(sendbuf,sizeof(sendbuf));
	bzero(sql,sizeof(sql));
	sprintf(sql,"select * from staffinfo where usrname=\"%s\"",constusr);
	if(sqlite3_get_table(db,sql,&resultp,&row,&column,&errmsg) != 0)
	{
		fprintf(stderr,"sqlite3_get_table:%s\n",errmsg);
		closepth(newfd);
	}
	if(row == 0)
	{	
		fprintf(stderr,"[%d,%s]table:staffinfo error\n",newfd,ip);
		closepth(newfd);
	}
	sprintf(sendbuf,"%s:%s:%s:%s:%s:%s:%s:%s:%s",resultp[10],resultp[11],resultp[12],resultp[13],resultp[14],resultp[15],resultp[16],resultp[17],resultp[18]);
	sqlite3_free_table(resultp);
	do	
	{
		ret = send(newfd,sendbuf,strlen(sendbuf),0);
	}while(ret < 0 && errno == EINTR);
	if(ret < 0)
	{
		perror("send");
		closepth(newfd);
	}
}




