/*版本v1.0
 *1.首先检查用户名是否存在
 *2.不存在进入注册模式，存在进入登录模式
 *3.登陆后进入一个聊天室，向所有聊天室中的用户发送消息
 *4.发现的问题：一段时间后登陆后无法正常发送消息，服务器直接退出，没有错误信息
 *不知道是程序bug还是服务器问题
 *5.在下一个版本我想将登录和注册的过程分开
 *我想将建立和绑定套接字的过程封装成函数或头文件
 *我想让代码更好看
 *6.不过就目前来说这个服务器还是可用的，所以就作为1.0版本吧
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h> //抛出错误信息
#include<sys/types.h>//pid_t等数据类型所在
#include<netinet/in.h> //网络通信所需头文件
#include<sys/socket.h>//socket通信所需头文件
#include<unistd.h>//unix标准库，含有常用的函数
#include<sqlite3.h>//sqlite3数据库
#include<pthread.h>//含有创建线程的函数
#include<bits/pthreadtypes.h>//pthread_t所在
#include"sqlfunc.h"//自定义的数据库操作封装的头文件

#define CHAT_PORT 1234//用于开放连接的端口
#define MAX_LINKED 100//最大连接数
int g_count=0; //全局变量，表示连接客户的总个数
int socket_list[100]={0}; //socket描述符储存于此
static sqlite3 *s_sql_db=NULL;//数据库对象
//static int s_login=0;//指示一个用户是否可以登录
//char recv_name_buff[20];//用于接收客户端发来的用户名
//char recv_pswd_buff[20];//用于接收客户端发来的密码
static char send_info_buff[100];//用于向客户端发送提示信息

static char is_full[]="1000:too-full-to-enter";//定义一组常量数组，用以发送系统信息
static char to_sign_up[]="1001:start-to-sign-up";
static char to_sign_in[]="1002:start-to-sign-in";
static char password_wrong[]="1003:the-password-is-wrong";
static char log_in[]="1004:successfully-enter-the-chatting-room";


void create_dwm(void*);//创建deal_with_message线程
void deal_with_message(void*);//多线程处理收发信息的函数
void deal_with_login(void*);//多线程处理登录信息的函数

void init_chat_room(){

	sql_open(s_sql_db);//打开数据库
	sqlite3_open("./data.db", &s_sql_db);
	int socket_fd, socket_acpt;
	struct sockaddr_in sevr_addr;
	memset(&sevr_addr, 0, sizeof(sevr_addr)/sizeof(struct sockaddr_in));//初始化地址内存

	printf("========聊天室系统服务端v1.1程序启动========\n");
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0))==-1){
	    fprintf(stdout,"创建套接字失败: %s(errno: %d)\n",strerror(errno),errno);
   	    exit(1);
	}//创建TCP流套接字，如果失败，抛出错误
	else printf("创建套接字成功...\n");

	sevr_addr.sin_family = AF_INET;//协议族AF_INET
    sevr_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY获取本机ip地址
    sevr_addr.sin_port = htons(CHAT_PORT);//设置端口为聊天用的端口
	
	if(bind(socket_fd, (struct sockaddr*)&sevr_addr, sizeof(sevr_addr)) == -1){
    		fprintf(stdout,"套接字绑定失败: %s(errno: %d)\n",strerror(errno),errno);
    		exit(1);
	}//将套接字与地址绑定
	else printf("绑定套接字成功...\n");

	if(listen(socket_fd, MAX_LINKED) == -1){
    	 fprintf(stdout,"监听失败: %s(errno: %d)\n",strerror(errno),errno);
   		 exit(1);
   	 }//开始监听1234端口，最大连接数设置为100
	else printf("开始监听1234端口，最大连接数：100\n");

	//进入循环，主线程一直等待新客户的链接
	while(1){
		if((socket_acpt = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1){  
        		fprintf(stdout,"接受客户端连接时发生错误: %s(errno: %d)",strerror(errno),errno);  
        		break;  
		}//等待客户端的连接,注：该函数默认阻塞

		else{
			if(g_count<100){
				fprintf(stdout,"一个用户准备连接该系统，正在处理其账号密码\n");
				pthread_t log_tid;
				pthread_create(&log_tid,NULL,(void*)deal_with_login,(void*)&socket_acpt);
				fprintf(stdout,"创建了一个新线程用于处理用户登录...\n");
			}
   			else {
				send(socket_acpt,is_full,strlen(is_full),0);
				close(socket_acpt);
				break;
			}//将连接写入全局变量，如果数组满了就阻止连接
			}
		}
	close(socket_fd);
	if(s_sql_db!=NULL) sqlite3_close(s_sql_db);
}

void deal_with_message(void *arg){
//接受一个客户的消息并分发给所有客户
    int acpt = *(int*)arg;//解引用获得int型套接字
	char buff[4096];
	while(1){
		int size=recv(acpt, buff, 4096, 0);//从客户端接受信息，返回值是字节长度
	    if(size>0){
		//如果正常接收
			printf("收到来自%d号用户的消息：%s\n",acpt,buff);
			int i;
			for(i = 0;i < g_count;i++){
				int val = send(socket_list[i],buff,size,0);
	        	printf("分发给%d号用户,返回值:%d\n",socket_list[i],val);
			}		
			memset(buff,0,sizeof(buff)/sizeof(char));
		}
		else{
		//如果接收错误
			fprintf(stdout,"客户%d似乎已经断开了连接,线程结束",acpt);
			pthread_exit(NULL);
		}
	}
}

void deal_with_login(void* arg){
//处理登录信息
	int socket_acpt = *(int*)arg;
	char recv_name_buff[20];
	char recv_pswd_buff[20];
	while(1){
		memset(recv_name_buff,0,sizeof(recv_name_buff)/sizeof(char));
		int size_a = recv(socket_acpt, recv_name_buff, 20, 0);
		if(size_a > 0){
		//正常接收客户端发送的数据
			fprintf(stdout,"收到用户名%s\n",recv_name_buff);
			int ret_a = sql_is_exist(s_sql_db,recv_name_buff);//检查用户名是否已经存在
			if(ret_a == 0){
				//如果用户名不存在
				printf("用户%s是新用户，启动注册\n",recv_name_buff);
				send(socket_acpt,to_sign_up,sizeof(to_sign_up),0);//发送要求注册的命令
				while(1){
					memset(recv_pswd_buff,0,sizeof(recv_pswd_buff)/sizeof(char));
					int size_b = recv(socket_acpt, recv_pswd_buff, 20, 0);
					if(size_b > 0){
						//正常接收密码数据
						printf("收到请求的密码:%s\n",recv_pswd_buff);
						int ret_d = sql_insert_usr(s_sql_db,recv_name_buff,recv_pswd_buff);
						if(ret_d==0) {
							fprintf(stdout,"注册成功!\n");
							send(socket_acpt,log_in,sizeof(log_in),0);
							socket_list[g_count++]=socket_acpt;	
							create_dwm(arg);//创建新的线程处理信息
							pthread_exit(NULL);//既然已经成功登陆，就退出这个线程
						}
						else  fprintf(stdout,"注册失败!\n");
						break;//跳出小循环
					}
					else{
					//接收异常
						fprintf(stdout,"用户%d似乎已断开连接\n",socket_acpt);
						pthread_exit(NULL);
					}
				}
				break;//跳出大循环
			}

			else if(ret_a > 0){
				//如果用户名已存在
				send(socket_acpt,to_sign_in,sizeof(to_sign_in),0);
				while(1){
					memset(recv_pswd_buff,0,sizeof(recv_pswd_buff)/sizeof(char));
					int size_c = recv(socket_acpt, recv_pswd_buff, 20, 0);
					if(size_c > 0){
					//正常接收数据
						char* pswd = sql_query_usr(s_sql_db,recv_name_buff);
						if(strcmp(pswd,recv_pswd_buff)==0){
						//如果密码是正确的
							socket_list[g_count++]=socket_acpt;	
							send(socket_acpt,log_in,sizeof(log_in),0);
							create_dwm(arg);//创建新的线程处理信息
							pthread_exit(NULL);
						}
						else {
						//若密码错误
							send(socket_acpt,password_wrong,strlen(password_wrong),0);
							continue;
						}
					}
					else {
					//接收错误
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
			pthread_exit(NULL);//结束线程
		}
	}             
}

void create_dwm(void* arg){
	pthread_t chat_tid;
	int ret_a = pthread_create(&chat_tid,NULL,(void*)deal_with_message,arg);//一旦有一个客户连接上，就创建一个新的线程
	if(ret_a == 0)printf("创建了一个新的线程,pid:%lu，用于处理消息收发...\n",chat_tid);
	else printf("创建新线程失败！\n");
}
int main(){
	init_chat_room();
}
