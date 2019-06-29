#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"sqlfunc.h"

int main(){
    sqlite3 *db;
    int ret = sqlite3_open("data.db",&db);
    int ret1;
    if(ret==0) printf("打开数据库成功！\n");
    else {
        sqlite3_close(db);
        exit(1);
    }
    char usr[]="administrator";
    char pswd[]="123456";
    char hack[]="waht";
    char* query_res=NULL;
    ret1=sql_chatting_room_is_exist(db,"clie");
    printf("%d\n",ret1);
    /*
    if((ret1=sql_is_exist(db,hack))==0)
    {
        int ret2 = sql_insert_usr(db,usr,pswd);
        if(ret2 == 0) printf("插入操作执行成功\n");
        else {
            printf("插入操作执行失败\n");
            sqlite3_close(db);
            exit(1);
        }
    }
    else {
        query_res = sql_query_usr(db,usr);
        if(query_res!=NULL)
            printf("该用户名的密码是:%s\n",query_res);
        else
            printf("空字符串\n");
    }
    */
}