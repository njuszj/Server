//实现功能：接收每个客户端的消息并分发给所有连接上的程序
#include<stdio.h>
#include<stdlib.h>//标准库头文件
#include<errno.h> //抛出错误
#include<sys/types.h>//pid_t等数据类型所在
#include<netinet/in.h>
#include<sys/socket.h>//socket通信所需头文件
#include<unistd.h>//fork函数所需头文件
#include<string.h>
#include<sqlite3.h>//数据库

#define CHAT_PORT 1234//用于正常聊天的端口

int count=0;
int socket_list[100]={0};
sqlite3 *db;

void recv_message(void* arg);
void init_chat_room(){
	int socket_fd, socket_acpt;
	struct sockaddr_in chataddr;
	printf("========聊天室系统服务端程序启动========\n");

	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0))==-1){
	    printf("创建套接字失败: %s(errno: %d)\n",strerror(errno),errno);
   	    exit(0);
	}//创建TCP流套接字，如果失败，抛出错误
	else printf("创建套接字成功！\n");
	memset(&chataddr, 0, sizeof(chataddr));//初始化地址内存
	chataddr.sin_family = AF_INET;//协议类型
    chataddr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY获取本机ip地址
    chataddr.sin_port = htons(CHAT_PORT);//设置端口为聊天用的端口

	if(bind(socket_fd, (struct sockaddr*)&chataddr, sizeof(chataddr)) == -1){
    		printf("套接字绑定失败: %s(errno: %d)\n",strerror(errno),errno);
    		exit(0);
	}//将套接字与地址绑定
	else printf("绑定套接字成功！\n");
	
	if(listen(socket_fd, 100) == -1){
    	 printf("监听失败: %s(errno: %d)\n",strerror(errno),errno);
   		 exit(0);
   	 }//开始监听1234端口，最大连接数设置为100
	else printf("开始监听1234端口，最大连接数：100\n");

	while(1){
		if((socket_acpt = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1){  
        		printf("接受客户端连接时发生错误: %s(errno: %d)",strerror(errno),errno);  
        		continue;  
		}//等待客户端的连接,注：该函数默认阻塞

		else{
			if(count<100){
                socket_list[count++]=socket_acpt;
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
		for(i = 0;i < count;i++){
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
