//实现功能：接收每个客户端的消息并分发给所有连接上的程序
#include<stdio.h>
#include<stdlib.h>//标准库头文件
#include<errno.h> //抛出错误
#include<sys/types.h>//pid_t等数据类型所在
#include<netinet/in.h>
#include<sys/socket.h>//socket通信所需头文件
#include<unistd.h>//fork函数所需头文件
#include<string.h>
#include<sqlite3.h>

#define CHAT_PORT 1234//用于正常聊天的端口

int count=0;
char delete_soc_table[64]="drop temp";
char insert[64]={};
char select_from[64]="select socket from temp";
sqlite3 *db;

void recv_message(int);
void init_chat_room(){
	int socket_fd, socket_acpt;
	struct sockaddr_in chataddr;
	
	//由于fork函数进程之间连全局变量都不共享，所以用数据库储存信息

	int ret = sqlite3_open("./data.db",&db);//打开数据库文件
	if(ret != SQLITE_OK){
	 	printf("打开数据库失败，服务器启动失败\n");
	 	exit(0);
	 }
	ret = sqlite3_exec(db,"delete from temp", NULL, NULL, NULL);//执行数据库命令操作,创建一个表储存套接字
	if(ret != SQLITE_OK){
	 	printf("创建套接字库失败，服务器启动失败\n");
	 	exit(0);
	}


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
	else printf("开始监听1234端口，最大连接数：100");

	while(1){
		if((socket_acpt = accept(socket_fd, (struct sockaddr*)NULL, NULL))==-1){  
        		printf("接受客户端连接时发生错误: %s(errno: %d)",strerror(errno),errno);  
        		continue;  
		}//等待客户端的连接,注：该函数默认阻塞

		else{
			if(count<100){
				strcpy(insert,"");
				sprintf(insert,"insert into temp values(%d)",socket_acpt);
				ret = sqlite3_exec(db,insert, NULL, NULL, NULL);//执行数据库命令操作,向表中插入一个套接字
				if(ret != SQLITE_OK){
	 			printf("数据库存储套接字发生错误\n");
				} 
			}
   			else {
				send(socket_acpt,"抱歉，连接客户已达上限\n",30,0);
				close(socket_acpt);
				break;
			}//将连接写入数据库，如果数组满了就阻止连接

			pid_t pid=fork();//一旦有一个客户连接上，就创建一个子进程
			if(pid<0){
				printf("fork()函数调用出现问题: %s(errno:%d)",strerror(errno),errno);
				continue;
			}
			else if(pid>0){
				continue;
			}//如果是父进程，则继续等待下一个客户端链接

			else	{
				recv_message(socket_acpt);
				close(socket_acpt);
			}
				//如果是子进程，则调用函数接受客户发来的消息
		}
	}
	sqlite3_close(db);
	close(socket_fd);
}

void recv_message(int acpt){
//接收客户端发来的信息
	int BUFFSIZE=4096;
	int nRow, nCol;
	char **dbResult;
	int index;
	char buff[BUFFSIZE];
	while(1){
		int size=recv(acpt, buff, BUFFSIZE, 0);//从客户端接受信息，返回值是字节长度
		int ret=sqlite3_get_table(db,select_from,&dbResult,&nRow,&nCol,NULL);
		if(ret != SQLITE_OK)printf("查询失败，可能无法转发信息\n");

	    if(size>0){
		printf("收到来自%d号用户的消息：%s\n",acpt,buff);
		
		for(index=nCol;index<nRow+nCol;index++){
			int soc=atoi(dbResult[index]);
			int val=send(soc,buff,size,0);
	        printf("分发给%d号用户,返回值:%d\n",soc,val);
		}	
		sqlite3_free_table(dbResult);
		memset(buff,0,sizeof(buff)/sizeof(char));
		}
	}
}


int main(){
	init_chat_room();
}
