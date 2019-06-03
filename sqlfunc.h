#ifndef _SQLFUNC_H_
#define _SQLFUNC_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<sqlite3.h>
int sql_insert_usr(sqlite3*, char*, char*);//增加一个用户名信息
char* sql_query_usr(sqlite3*,char*);//查询一个用户名信息
int sql_is_exist(sqlite3*, char*);//确定数据库中是否存在一条用户名信息
#endif