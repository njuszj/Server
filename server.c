/*实现功能：接收每个客户端的消息并分发给所有连接上的程序
 *基于多线程
 *使用sqlite3数据库
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h> //抛出错误信息
#include<sys/types.h>//pid_t等数据类型所在
#include<netinet/in.h> //网络通信所需头文件
#include<sys/socket.h>//socket通信所需头文件
#include<unistd.h>//fork pipe IO函数等所在
#include<sqlite3.h>//sqlite3数据库
#include<pthread.h>//含有创建线程
#include<bits/pthreadtypes.h>//pthread_t所在
#include"sqlfunc.h"

#define CHAT_PORT 1234//用于开放连接的端口
#define MAX_LINKED 100//最大连接数
int g_count=0; //全局变量，表示连接客户的总个数
int socket_list[100]={0}; //socket描述符储存于此
static sqlite3 *sql_db=NULL;//数据库对象
static char recv_name_buff[20];//用于接收客户端发来的用户名
static char recv_pswd_buff[20];//用于接收客户端发来的密码
static char send_info_buff[100];//用于向客户端发送提示信息
static int s_login=0;//指示一个用户是否可以登录

void recv_message(void* arg);
void deal_with_login(int);

void init_chat_room(){
	int sql_reta_open=1;
	sql_reta_open = sqlite3_open("./data.db", &sql_db);//打开数据库
	if(sql_reta_open){
		fprintf(stderr, "打开数据库失败，%s\n", sqlite3_errmsg(sql_db));
		sqlite3_close(sql_db);
		exit(1);
	}

	int socket_fd, socket_acpt;
	struct sockaddr_in sevr_addr;
	memset(&sevr_addr, 0, sizeof(sevr_addr));//初始化地址内存

	printf("========聊天室系统服务端程序启动========\n");

	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0))==-1){
	    fprintf(stderr,"创建套接字失败: %s(errno: %d)\n",strerror(errno),errno);
   	    exit(1);
	}//创建TCP流套接字，如果失败，抛出错误
	else printf("创建套接字成功！\n");

	sevr_addr.sin_family = AF_INET;//协议族
    sevr_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY获取本机ip地址
    sevr_addr.sin_port = htons(CHAT_PORT);//设置端口为聊天用的端口

	if(bind(socket_fd, (struct sockaddr*)&sevr_addr, sizeof(sevr_addr)) == -1){
    		fprintf(stderr,"套接字绑定失败: %s(errno: %d)\n",strerror(errno),errno);
    		exit(1);
	}//将套接字与地址绑定
	else printf("绑定套接字成功！\n");
	
	if(listen(socket_fd, MAX_LINKED) == -1){
    	 fprintf(stderr,"监听失败: %s(errno: %d)\n",strerror(errno),errno);
   		 exit(1);
   	 }//开始监听1234端口，最大连接数设置为100
	else printf("开始监听1234端口，最大连接数：100\n");

	//进入循环，主线程一直等待新客户的链接
	while(1){
		if((socket_acpt = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1){  
        		printf("接受客户端连接时发生错误: %s(errno: %d)",strerror(errno),errno);  
        		continue;  
		}//等待客户端的连接,注：该函数默认阻塞

		else{
			if(g_count<100){
				fprintf(stdout,"一个用户准备连接该系统，正在检查账号密码\n");
				s_login=0;
				deal_with_login(socket_acpt);
			}
   			else {
				send(socket_acpt,"too-full-to-enter",17,0);
				close(socket_acpt);
				break;
			}//将连接写入全局变量，如果数组满了就阻止连接
			if(s_login==0) continue;
			pthread_t tid;
			int reta = pthread_create(&tid,NULL,(void*)recv_message,(void*)&socket_acpt);//一旦有一个客户连接上，就创建一个新的线程
			if(reta==0)printf("创建了一个新的线程，pid:%lu\n",tid);
			else printf("创建新线程失败！\n");
			}
		}
	close(socket_fd);
	if(sql_db!=NULL) sqlite3_close(sql_db);
}

void recv_message(void* arg){
//接收客户端发来的信息
    int acpt = *(int*)arg;//解引用获得int型套接字
	char buff[4096];
	while(1){
		int size=recv(acpt, buff, 4096, 0);//从客户端接受信息，返回值是字节长度

	    if(size>0){
		printf("收到来自%d号用户的消息：%s\n",acpt,buff);
		int i;
		for(i = 0;i < g_count;i++){
			int val=send(socket_list[i],buff,size,0);
	        printf("分发给%d号用户,返回值:%d\n",socket_list[i],val);
		}	
		memset(buff,0,sizeof(buff)/sizeof(char));
		}
	}
}
//输入用户名
void deal_with_login(int socket_acpt){
//处理登录信息
	while(1){
		memset(recv_name_buff,0,sizeof(recv_name_buff)/sizeof(char));
		int size_a = recv(socket_acpt, recv_name_buff, 20, 0);
		if(size_a > 0){
			fprintf(stdout,"收到用户名%s\n",recv_name_buff);
			int ret_a = sql_is_exist(sql_db,recv_name_buff);//检查用户名是否已经存在

			if(ret_a == 0){
				//如果用户名不存在
				printf("用户%s是新用户，启动注册\n",recv_name_buff);
				memset(send_info_buff,0,sizeof(send_info_buff)/sizeof(char));
				send(socket_acpt,"enter-the-sign-up",18,0);//发送要求注册的命令
				while(1){
					memset(recv_pswd_buff,0,sizeof(recv_pswd_buff)/sizeof(char));
					int size_b = recv(socket_acpt, recv_pswd_buff, 20, 0);
					if(size_b > 0){
						//正常接收字节
						printf("收到请求的密码:%s\n",recv_pswd_buff);
						int ret_d = sql_insert_usr(sql_db,recv_name_buff,recv_pswd_buff);
						if(ret_d==0) {
							s_login=1;
							fprintf(stdout,"注册成功!\n");
							send(socket_acpt,"enter-the-chatting-room-by-signup",34,0);
						}
						else  fprintf(stdout,"注册失败!\n");
						break;//跳出小循环
					}
				}
				break;//跳出大循环
			}

			else if(ret_a > 0){
				//如果用户名已存在
				memset(send_info_buff,0,sizeof(send_info_buff)/sizeof(char));
				sprintf(send_info_buff,"enter-the-sign-in");
				int ret_c = send(socket_acpt,send_info_buff,sizeof(send_info_buff),0);
				while(1){
					memset(recv_pswd_buff,0,sizeof(recv_pswd_buff)/sizeof(char));
					int size_c = recv(socket_acpt, recv_pswd_buff, 20, 0);
					if(size_c > 0){
						char* pswd = sql_query_usr(sql_db,recv_name_buff);
						if(strcmp(pswd,recv_pswd_buff)==0){
							socket_list[g_count++]=socket_acpt;	
							s_login=1;
							send(socket_acpt,"enter-the-chatting-room",24,0);
							break;
						}
						else {
							send(socket_acpt,"the-password-is-wrong",22,0);
							continue;
						}
					}
					else {
						printf("该客户似乎与服务器断开了连接\n");
						break;//跳出小循环
					}
				}
				break;//跳出大循环
			}
			else {
				//查询出错
				printf("数据库查询出错\n");
				break;
			}
		}	
		else  {
			printf("该客户似乎与服务器断开了连接\n");
			break;
		}
	}             
}

int main(){
	init_chat_room();
}
