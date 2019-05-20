#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>//inet_pton函数所需
#include<string.h>
#define BUFFSIZE 4096
char *IP="127.0.0.1";


void SendMessage(int fd,char* usrname){
	char sendbuff[BUFFSIZE]={};
	char message[BUFFSIZE]={};
	while(1){
	scanf("%s",message);
	sprintf(sendbuff,"%s: %s",usrname,message);
	send(fd,sendbuff,strlen(sendbuff),0);
	strcpy(sendbuff,"");
	strcpy(message,"");
	}
}


void initTCP(){
	int socket_fd;
	struct sockaddr_in server_addr; 
	if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("创建套接字失败: %s(errno: %d)\n", strerror(errno),errno);  
    		exit(0); 
	}
										
	memset(&server_addr, 0, sizeof(server_addr));//初始化地址结构空间

	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1234);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if( connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    		printf("连接失败: %s(errno: %d)\n",strerror(errno),errno);
    		exit(0);
    	}//建立连接

	printf("===成功与服务器连接===\n");
	printf("请输入你的名字：");
	char usrname[50]={};
	char send_buff[100]={};
	scanf("%s",usrname);//储存用户名字
	sprintf(send_buff,"======欢迎 %s 进入聊天室======\n",usrname);
	send(socket_fd,send_buff,strlen(send_buff),0);//发送用户名等数据


	pid_t pid=fork();
	if(pid<0) printf("创建新的进程时发生错误！\n");
	else if(pid>0) SendMessage(socket_fd,usrname);//开创新的进程发送信息
	else{
	//父进程用于接收来自服务器的消息
		char recv_buff[BUFFSIZE]={};
	    while(1){
  			int len = recv(socket_fd, recv_buff,BUFFSIZE,0);
			if(len<=0)printf("接收文本失败!\n");
			else{
				recv_buff[len]='\0';
				printf("%s\n",recv_buff);
				strcpy(recv_buff,"");
			}
		}
		close(socket_fd);
	}
}


int main(){
	initTCP();
}
