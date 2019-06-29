#ifndef _SQLFUNC_H_
#define _SQLFUNC_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<sqlite3.h>
int sql_insert_usr(sqlite3*, char*, char*);//增加一个用户名信息
char* sql_query_usr(sqlite3*,char*);//查询一个用户名信息
int sql_usrname_is_exist(sqlite3*, char*);//确定数据库中是否存在一条用户名信息
int sql_open(sqlite3* );
int sql_create_chatting_room(sqlite3* ,char*);
int sql_chatting_room_is_exist(sqlite3*,char*);
/*
create table tablename(
    columnname1 int NOT NULL,
    columnname2 varchar(20) PRIMARY Key
);创建表的操作，直接使用似乎比封装更方便
*/
#endif