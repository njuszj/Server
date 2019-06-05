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
int sql_is_exist(sqlite3 *sql_db,char* usrname){
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



