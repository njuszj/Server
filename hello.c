#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>
void printThreadInfo(void*);
int g_val=0;
int main(){
	pthread_t thread1, thread2;
	char* message1="这里是一号线程";
	char* message2="这里是二号线程";
	int ret1 = pthread_create(&thread1, NULL, (void *)&printThreadInfo, (void *) message1);
	int ret2 = pthread_create(&thread2, NULL, (void *)&printThreadInfo, (void *) message2);
	if(!(ret1||ret2))
		printf("线程创建成功！\n");

	while(1)
    {
       printf("g_val=%d\n",++g_val);
       sleep(1);
    }
}

void printThreadInfo(void* ptr){
	int i;
	printf("hello,world;\n");
	for(i=0;i<10;i++){
	    printf("%s:%d\n", (char *)ptr,++g_val);
		sleep(1);
	}
}
