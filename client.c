#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
#define H 0xA //查询历史记录
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


void do_add(int sockfd,MSG *msg);
void do_del(int sockfd,MSG *msg);
void do_update(int sockfd,MSG *msg);
void do_query(int sockfd,MSG *msg);
void modify_pwd(int sockfd,MSG *msg);
void self_query(int sockfd,MSG *msg);
void do_history(int sockfd,MSG *msg);



int main(int argc, const char *argv[])
{

	int sockfd;
	struct sockaddr_in serveraddr;
	socklen_t len=sizeof(SA);
	int cmd;
	MSG msg;

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err_log("fail to socket");
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(55555);
	serveraddr.sin_addr.s_addr=inet_addr("192.168.1.211");

	if(connect(sockfd,(SA*)&serveraddr,len)<0)// 客户端执行马上连接服务器
	{
		err_log("fail to connect");
	}

BEF:
	while(1)//登录界面
	{
		puts("**********************************************");
		puts("   Welcome Employee management system V1.0    ");
		puts("              Please Login            ");
		puts("**********************************************");
		puts("input user name>>>");

		scanf("%s",msg.user_name);
		msg.type = L;
		puts("input password>>>");
		scanf("%s",msg.text);

		send(sockfd,&msg,LEN_MSG,0);//用户名密码发给服务器
		recv(sockfd,&msg,LEN_MSG,0); //等待服务器回应
		if(strncmp(msg.text,"admin",5)==0)
		{
			goto SECOND;
		}
		else if(strncmp(msg.text,"user",4)==0)
		{
			goto THIRD;
		}else{
			printf("User not exist or password error,Please input again\n");
		}

	}

EXIT:  

	msg.type=E;
	send(sockfd,&msg,LEN_MSG,0);
	close(sockfd);  //客户端退出的处理   
	exit(0);

SECOND:
	while(1)//管理员界面
	{
		puts("*************************************************************************");
		puts("1.增加用户 2.删除用户信息 3.修改用户信息 4.查询用户信息 5.历史记录 6.退出");
		puts("*************************************************************************");
		puts("input cmd>>>");
		
		scanf("%d",&cmd);
		
		switch(cmd)
		{
		case 1:
			do_add(sockfd,&msg);
			break;
		case 2:
			do_del(sockfd,&msg);
			break;
		case 3:
			do_update(sockfd,&msg);
			break;
		case 4:
			do_query(sockfd,&msg);
			break;
		case 5:
			do_history(sockfd,&msg);
			break;
		case 6:
			goto BEF;
			break;
		default:
			puts("cmd error");
			break;
		}

	}

THIRD:
	while(1)//用户界面
	{
		puts("**************************************************************");
		puts("            1.修改密码 2.查询用户信息 3.退出");
		puts("**************************************************************");
		puts("input cmd>>>");
		
		scanf("%d",&cmd);
		
		switch(cmd)
		{
			case 1:
				modify_pwd(sockfd,&msg);
				break;
			case 2:
				self_query(sockfd,&msg);
				break;
			case 3:
				goto BEF;
				break;
			default:
				puts("cmd error");
				break;
		}
	}
	return 0;
}


void do_add(int sockfd,MSG *msg)
{
	msg->type = A;
	puts("input user_name>>>");
	scanf("%s",msg->user_name);  //管理员为新员工创建账户
	strcpy(msg->text,"123");           //默认密码为“123”

	puts("input name>>>");
	scanf("%s",msg->name);
	puts("input id>>>");
	scanf("%d",msg->id);
	puts("input tel>>>");
	scanf("%d",msg->tel);
	puts("input Address>>>");
	scanf("%s",msg->Address);
	puts("input Age>>>");
	scanf("%d",msg->Age);
	puts("input wages>>>");
	scanf("%d",msg->wages);
	puts("input level>>>");
	scanf("%s",msg->lev);
	

	send(sockfd,msg,LEN_MSG,0);//用户信息发给服务器
	recv(sockfd,msg,LEN_MSG,0);//等待服务器回应
	if(strncmp(msg->text,"OK",2)==0)
	{
		puts("add information success!");
		return;
	}
	else
	{
		puts("add information fail!");
		return;
	}

	
}

void do_del(int sockfd,MSG *msg)
{
	msg->type = D;
	puts("Enter the account of the staff you want to delete>>>");
	scanf("%s",msg->user_name);
	send(sockfd,msg,LEN_MSG,0);//所要删除的员工账号发给服务器，服务器根据user_name删除员工信息
	recv(sockfd,msg,LEN_MSG,0);//等待服务器回应
	if(strncmp(msg->text,"OK",2)==0)
	{
		puts("del information success!");
		return;
	}
	else
	{
		puts("del information fail!");
		return;
	}

}

void do_update(int sockfd,MSG *msg)
{
	msg->type = R;
	puts("input the id of staff>>>");
	scanf("%d",msg->id);

	puts("input new name>>>");
	scanf("%s",msg->name);
	puts("input new tel>>>");
	scanf("%d",msg->tel);
	puts("input new Address>>>");
	scanf("%s",msg->Address);
	puts("input new Age>>>");
	scanf("%d",msg->Age);
	puts("input new wages>>>");
	scanf("%d",msg->wages);
	puts("input new level>>>");
	scanf("%s",msg->lev);
	send(sockfd,msg,LEN_MSG,0);//根据员工id,将修改的信息发送给服务器
	recv(sockfd,msg,LEN_MSG,0);//等待服务器回应
	if(strncmp(msg->text,"OK",2)==0)
	{
		puts("revise information success!");
		return;
	}
	else
	{
		printf("%s\n",msg->text);
		return;
	}
	
}

void do_query(int sockfd,MSG *msg)
{
	msg->type = QA;
	int i=0;
	send(sockfd,msg,LEN_MSG,0);
	recv(sockfd,msg,LEN_MSG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{
		while(1)
		{
			recv(sockfd,msg,LEN_MSG,0);//等待服务器逐行发送用户信息
			i++;
			if(strncmp(msg->text,"over",4)==0)//所有的用户信息发送结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%8==0)//每7个换行
			{putchar(10);}
		}
		puts("------------------------------------");
		puts("Query ok!");
		
	}
	else
	{
		puts("No information!");	
		return;
	}

	puts("input query's  name>>>");
	msg->type = QN;
	scanf("%s",msg->name);
	send(sockfd,msg,LEN_MSG,0);
	recv(sockfd,msg,LEN_MSG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{
		i = 0;
		while(1)
		{
			recv(sockfd,msg,LEN_MSG,0);//等待服务器逐行发送用户信息
			i++;
			if(strncmp(msg->text,"over",4)==0)//所有的用户信息发送结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%8==0)//每8个换行
			{putchar(10);}
		}
		puts("------------------------------------");
		puts("Query ok!");
		
	}else{		
		puts("No information!");	
	}

	return;
}


void modify_pwd(int sockfd,MSG *msg)
{
	msg->type = RP;
	puts("Please input new password>>>");
	scanf("%s",msg->text);
	send(sockfd,msg,LEN_MSG,0);
	recv(sockfd,msg,LEN_MSG,0);
	if(strncmp(msg->text,"OK",2)==0){
		puts("Password modification success!");
	}else{
		puts("Password modification failed");
	}
  return;
}


void self_query(int sockfd,MSG *msg)
{
	msg->type = QU;  //服务器根据user_name查找信息
	send(sockfd,msg,LEN_MSG,0);
	recv(sockfd,msg,LEN_MSG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{
		int i = 0;
		while(1)
		{
			recv(sockfd,msg,LEN_MSG,0);//等待服务器逐行发送用户信息
			i++;
			if(strncmp(msg->text,"over",4)==0)//所有的用户信息发送结束跳出循环
			{
				break;
			}

			printf("%-15s",msg->text);
			if(i%8==0)//每8个换行
			{putchar(10);}
		}
		puts("------------------------------------");
		puts("Query ok!");
		
	}else{		
		puts("No information!");	
	}
	return;
}

void do_history(int sockfd,MSG *msg)
{
	msg->type=H;
	int i=0;
	send(sockfd,msg,LEN_MSG,0);
	recv(sockfd,msg,LEN_MSG,0);
	if(strncmp(msg->text,"OK",2)==0)
	{

		while(1)
		{
			recv(sockfd,msg,LEN_MSG,0);//等待服务器回应待服务器发送的历史记录条目
			i++;
			if(strncmp(msg->text,"over",4)==0)//记录结束跳出循环
			{
				break;
			}

			printf("%-22s",msg->text);
			if(i%3==0)//每三个换行
			{putchar(10);}
		}
		puts("------------------------------------");
		puts("History ok!");
		return;
	}
	else
	{
		puts("History fail!");
		return;
	}
}
