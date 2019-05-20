#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
int num[2]={1,2};
int count;

void f(int *a){
   printf("f:num0=%d\n",a[0]);
}

int main(){
int b=0;
pid_t pid=fork();
if(pid==0){
	int i;
 	for(i=0;i<100;i++)num[0]=5;
	printf("b=%d\n",b);
}
 
 else{
    int j;
	int* fonum=num;
	for(j=0;j<10;j++) f(fonum);
    printf("num0:%d\n",num[0]);
	b=5;
 }
}
