#include"sqlfunc.h"

//向数据库中的client表插入一项:用户名-密码
int sql_insert_usr(sqlite3 *sql_db, char* usrname,char* password){
	char cmd[100];
	memset(cmd, 0, sizeof(cmd));//初始数组内容
	sprintf(cmd,"insert into client values(\"%s\",\"%s\");",usrname,password);
    printf("%s\n",cmd);
	int ret = sqlite3_exec(sql_db,cmd,NULL,NULL,NULL);
	return ret;
}

//查询数据库中的client表中的usrname字段，返回其密码
char* sql_query_usr(sqlite3 *sql_db,char* usrname){
    char cmd[100];
    int row,col;//分别表示查询结果的行数和列数，其中行数不包括列名
    char **res=NULL;//查询结果保存的地方，类似于string数组，结构是一维的，抽象来看是查询结果按行排列（包括列名）
    memset(cmd, 0, sizeof(cmd));//初始命令数组内容
    sprintf(cmd,"select password from client where usrname like '%s';",usrname);
    sqlite3_get_table(sql_db,cmd,&res,&row,&col,NULL);
    
    if(row>0){
        int len=strlen(*(res+1));
        char* ret = (char*)malloc(len*sizeof(char));//分配堆空间，防止函数结束被释放
        ret = *(res+1);
        sqlite3_free_table(res);
        return ret;
    }
    else
        {
        printf("查询数据为空\n");
        sqlite3_free_table(res);
        return NULL;
        }

}

//返回一个用户名存在的条数,用于检查用户名是否已经存在
int sql_usrname_is_exist(sqlite3 *sql_db,char* usrname){
    char cmd[100];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(cmd, 0, sizeof(cmd));//初始命令数组内容
    sprintf(cmd,"select usrname from client where usrname like '%s';",usrname);
    sqlite3_get_table(sql_db,cmd,&res,&row,&col,NULL);
    ret=row;
    sqlite3_free_table(res);
    return row;
}

int sql_open(sqlite3* db){
//该函数负责打开数据库
	int sql_ret_open=1;
	sql_ret_open = sqlite3_open("./data.db", &db);//打开数据库
	if(sql_ret_open!=0){
		fprintf(stderr, "打开数据库失败，%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);//只要调用exit，整个程序就结束
	}
    else return 1;
}

int sql_create_chatting_room(sqlite3* db, char* str){
    char cmd[100];
    int ret = 1;
	memset(cmd, 0, sizeof(cmd));//初始数组内容
	sprintf(cmd,"create table %s(usrname varchar(20),socket_fd int",str);
    if(sql_chatting_room_is_exist(db,str)==0) 
        ret = sqlite3_exec(db,cmd,NULL,NULL,NULL);
    return ret;
}

int sql_chatting_room_is_exist(sqlite3* db,char* str){
    char cmd[100];//同查询函数
    int row,col,ret;
    char **res=NULL;
    memset(cmd, 0, sizeof(cmd));//初始命令数组内容
    sprintf(cmd,"select name from sqlite_master where type='table' and name='%s'",str);
    sqlite3_get_table(db,cmd,&res,&row,&col,NULL);
    ret=row;
    sqlite3_free_table(res);
    return row;
}



