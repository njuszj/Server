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
#include<bits/pthreadtypes.h>//pthread_t所在
#include"sqlfunc.h"

#define CHAT_PORT 1234//用于开放连接的端口
#define MAX_LINKED 100//最大连接数
int g_count=0; //全局变量，表示连接客户的总个数
int socket_list[100]={0}; //socket描述符储存于此
static sqlite3 *sql_db=NULL;//数据库对象
static char* recv_name_buff[20];//用于接收客户端发来的用户名
static char* recv_pswd_buff[20];//用于接收客户端发来的密码
static char* send_info_buff[100];//用于向客户端发送提示信息

void recv_message(void* arg);

void init_chat_room(){
	int sql_ret_open=1;
	sql_ret_open = sqlite3_open("./data.db", &sql_db);//打开数据库
	if(sql_ret_open){
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
				while(1){
					memset(recv_name_buff,0,sizeof(recv_name_buff)/sizeof(char));
					int size=recv(socket_acpt, recv_name_buff, 20, 0);
					if(size>0){
						int ret=sql_is_exist(sql_db,recv_name_buff);
						if(ret==0){
							memset(send_info_buff,0,sizeof(send_info_buff)/sizeof(char));
							sprintf(send_info_buff,"该用户名是新用户名，请输入密码进行注册");
							int val=send(socket_acpt,send_info_buff,sizeof(send_info_buff),0);
							memset(recv_pswd_buff,0,sizeof(recv_pswd_buff)/sizeof(char));
							int size_1=recv(socket_acpt, recv_pswd_buff, 20, 0);
							if(size_1>0){
								sql_insert_usr(sql_db,recv_name_buff,recv_pswd_buff);
							}
						}
					}
				}
                socket_list[g_count++]=socket_acpt;
			}
   			else {
				send(socket_acpt,"抱歉，连接客户已达上限\n",30,0);
				close(socket_acpt);
				break;
			}//将连接写入全局变量，如果数组满了就阻止连接

			pthread_t tid;
			int ret = pthread_create(&tid,NULL,(void*)recv_message,(void*)&socket_acpt);//一旦有一个客户连接上，就创建一个新的线程
			if(ret==0)printf("创建了一个新的线程，pid:%lu\n",tid);
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


int main(){
	init_chat_room();
}
