#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sqlite3.h> //for sqlite3_open ..
#include <signal.h>
#define PATH_DATA "./staff.db" //数据库
#define N 32                                                                                               
#define M 128
#define A 0x1 //增加用户
#define D 0x2 //删除用户
#define R 0x3 //修改用户信息
#define RP 0x4    //客户端修改密码
#define QA 0x5  //查询所有的用户信息
#define QU 0x6  //根据user_name查询用户的信息
#define QN 0x7  //根据name查询用户信息
#define L 0x8  //登录
#define E 0x9 //退出

typedef struct{
	int type;//消息类型
	char user_name[N];//用户名
	char text[M];//文本 或 密码
	char name[N];//员工名字
	int id[N];//员工编号
	int tel[N];//联系电话
	char Address[M];//住址
	int Age[N];//年龄
	int wages[N];//工资
	char lev[N];//等级
}MSG;                                                                                                      


#define LEN_MSG sizeof(MSG)
#define err_log(log)\
	do{\
		perror(log);\
		exit(1);\
	}while(0)

typedef struct sockaddr SA;


void process_login(int clientfd,MSG *msg,sqlite3 *db);
void process_add(int clientfd,MSG *msg,sqlite3 *db);
void process_del(int clientfd,MSG *msg,sqlite3 *db);
void process_update(int clientfd,MSG *msg,sqlite3 *db);
void process_query_all(int clientfd,MSG *msg,sqlite3 *db);
void process_query_name(int clientfd,MSG *msg,sqlite3 *db);
void process_revise_password(int clientfd,MSG *msg,sqlite3 *db);
void process_self_query(int clientfd,MSG *msg,sqlite3 *db);


void handler(int arg)
{
	wait(NULL);
}

int main(int argc, const char *argv[])
{
	int serverfd,clientfd;
	struct sockaddr_in serveraddr,clientaddr;
	socklen_t len = sizeof(SA);
	int cmd;
	char clean[M]={0};
	MSG msg;
	pid_t pid;
	sqlite3 *db;
	ssize_t bytes;

	if(sqlite3_open(PATH_DATA,&db)!=SQLITE_OK)//打开数据库
	{
		printf("%s\n",sqlite3_errmsg(db));
		return -1;
	}
	if((serverfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err_log("fail to socket");
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(55555);
	serveraddr.sin_addr.s_addr=0;
	if(bind(serverfd,(SA*)&serveraddr,len)<0)
	{
		err_log("fail to bind");
	}
	if(listen(serverfd,10)<0)
	{
		err_log("fail to listen");
	}
	signal(SIGCHLD,handler);//处理僵尸进程

	while(1)
	{
		if((clientfd=accept(serverfd,(SA*)&clientaddr,&len))<0)
		{
			perror("fail to accept");
			continue;
		}
		pid=fork();
		if(pid<0)
		{
			perror("fail to fork");
			continue;
		}
		else if(pid==0) //接收客户端的请求处理过程
		{
			close(serverfd);
			while(1)
			{
				bytes=recv(clientfd,&msg,LEN_MSG,0);
				if(bytes<=0)
					break;
				switch(msg.type)
				{
				case L:
					process_login(clientfd,&msg,db);
					break;
				case A:
					process_add(clientfd,&msg,db);
					break;
				case D:
					process_del(clientfd,&msg,db);
					break;
				case R:
					process_update(clientfd,&msg,db);
					break;
				case QA:
					process_query_all(clientfd,&msg,db);
					break;
				case QN:
					process_query_name(clientfd,&msg,db);
					break;
				case RP:
					process_revise_password(clientfd,&msg,db);
					break;
				case QU:
					process_self_query(clientfd,&msg,db);
					break;
				case E:
					exit(0);
				}
			}
			close(clientfd);
			exit(1);
		}
		else
		{
			close(clientfd);
		}


	}

	return 0;
}


void process_login(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;

	if(strncmp(msg->user_name,"admin",5)==0){ //用户名为admin
		if(strncmp(msg->text,"admin",5)==0){//admin的密码正确
			strcpy(msg->text,"admin");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}else{//admin的密码不正确			
			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}

	}else{//用户名不为admin,从数据库表中查找该用户是否存在并判断密码是否正确

		sprintf(sql,"select * from login_msg where user_name='%s' and passwd='%s'",msg->user_name,msg->text);
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
			printf("%s\n",errmsg);
			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
		else
		{
			if(n_row==0)//查不到
			{

				strcpy(msg->text,"Fail");
				send(clientfd,msg,LEN_MSG,0);
				return;
			}
			else  //只要行数大于0，无需打印，直接返回成功
			{

				strcpy(msg->text,"user");
				send(clientfd,msg,LEN_MSG,0);
				return;
			}
		}
	}

}


void process_add(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;
	//先对login_msg表进行插入表操作
	sprintf(sql,"insert into login_msg values('%s','%s')",msg->user_name,msg->text); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}

	//再对staff_msg表进行插入操作
	sprintf(sql,"insert into staff_msg values('%s','%s',%d,%d,'%s',%d,%d,'%s')"
			,msg->user_name,msg->name,*(msg->id),*(msg->tel),msg->Address,*(msg->Age),*(msg->wages),msg->lev); 
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}
	strcpy(msg->text,"OK");
	send(clientfd,msg,LEN_MSG,0);

	return;
}


void process_del(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;

	//根据客户端传来的user_name对数据库表进行先删除员工信息
	sprintf(sql,"delete from staff_msg where user_name='%s'",msg->user_name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}
	
	//再删除账号
	sprintf(sql,"delete from login_msg where user_name='%s'",msg->user_name);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}

	strcpy(msg->text,"OK");
	send(clientfd,msg,LEN_MSG,0);
	return;

}


void process_update(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	//根据id来修改数据
	sprintf(sql,"update staff_msg set name='%s',tel=%d,Address='%s',Age=%d,wages=%d,lev='%s' where id=%d"
			,msg->name,*(msg->tel),msg->Address,*(msg->Age),*(msg->wages),msg->lev,*(msg->id));

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"update failed");
		send(clientfd,msg,LEN_MSG,0);
		return;
	}

	strcpy(msg->text,"OK");
	send(clientfd,msg,LEN_MSG,0);
	return;

}



void process_query_all(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;
	int i,j;
	sprintf(sql,"select * from staff_msg");
	if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"Fail");
		send(clientfd,msg,LEN_MSG,0);
		return;
	}
	else
	{
		if(n_row==0)
		{

			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
		else
		{

			strcpy(msg->text,"OK");
			send(clientfd,msg,LEN_MSG,0);
			for(i=0;i<n_row+1;i++)
			{
				for(j=0;j<n_column;j++)	
				{
					strcpy(msg->text,*rep++);
					send(clientfd,msg,LEN_MSG,0);
					usleep(1000);//防止粘包
				}
			}
			strcpy(msg->text,"over");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
	}
}

void process_query_name(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;
	int i,j;
	sprintf(sql,"select * from staff_msg where name='%s'",msg->name);
	if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"Fail");
		send(clientfd,msg,LEN_MSG,0);
		return;
	}
	else
	{
		if(n_row==0)
		{

			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
		else
		{

			strcpy(msg->text,"OK");
			send(clientfd,msg,LEN_MSG,0);
			for(i=0;i<n_row+1;i++)
			{
				for(j=0;j<n_column;j++)	
				{
					strcpy(msg->text,*rep++);
					send(clientfd,msg,LEN_MSG,0);
					usleep(1000);//防止粘包
				}
			}
			strcpy(msg->text,"over");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
	}
}


void process_revise_password(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	//根据user_name来修改密码
	sprintf(sql,"update login_msg set passwd='%s' where user_name='%s'"
	,msg->text,msg->user_name);

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"modify failed");
		send(clientfd,msg,LEN_MSG,0);
		return;
	}else{
	
		strcpy(msg->text,"OK");
		send(clientfd,msg,LEN_MSG,0);
		return;	
	}
}



void process_self_query(int clientfd,MSG *msg,sqlite3 *db)
{
	char sql[M]={0};
	char *errmsg;
	char **rep;
	int n_row;
	int n_column;
	int i,j;
	sprintf(sql,"select * from staff_msg where user_name='%s'",msg->user_name);
	if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		strcpy(msg->text,"Fail");
		send(clientfd,msg,LEN_MSG,0);
		return;
	}
	else
	{
		if(n_row==0)
		{

			strcpy(msg->text,"Fail");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
		else
		{

			strcpy(msg->text,"OK");
			send(clientfd,msg,LEN_MSG,0);
			for(i=0;i<n_row+1;i++)
			{
				for(j=0;j<n_column;j++)	
				{
					strcpy(msg->text,*rep++);
					send(clientfd,msg,LEN_MSG,0);
					usleep(1000);//防止粘包
				}
			}
			strcpy(msg->text,"over");
			send(clientfd,msg,LEN_MSG,0);
			return;
		}
	}
}

