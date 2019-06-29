#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>//抛出错误信息
#include<sys/types.h>//pid_t等数据类型所在
#include<netinet/in.h>//网络通信所需头文件
#include<sys/socket.h>//socket通信所需头文件
#include<unistd.h>//unix标准库，含有常用的函数,比如fork


int init_socket(){
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
    sevr_addr.sin_port = htons(1234);//设置端口为聊天用的端口
	
	if(bind(socket_fd, (struct sockaddr*)&sevr_addr, sizeof(sevr_addr)) == -1){
    		fprintf(stdout,"套接字绑定失败: %s(errno: %d)\n",strerror(errno),errno);
    		exit(1);
	}//将套接字与地址绑定
	else printf("绑定套接字成功...\n");

	if(listen(socket_fd, 100) == -1){
    	 fprintf(stdout,"监听失败: %s(errno: %d)\n",strerror(errno),errno);
   		 exit(1);
   	 }//开始监听1234端口，最大连接数设置为100
	else printf("开始监听1234端口，最大连接数：100\n");
    return socket_fd;
}

int main(){
    int socket_fd=init_socket();
    int socket_acpt;
    if((socket_acpt = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1){  
        		fprintf(stdout,"接受客户端连接时发生错误: %s(errno: %d)",strerror(errno),errno);  
		}//等待客户端的连接,注：该函数默认阻塞
    else
        printf("一个客户连接\n");
}