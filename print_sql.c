#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>

int main(){
    sqlite3 *db;
    int ret_a = sqlite3_open("data.db",&db);
    if(ret_a == 0) printf("打开数据库成功！\n");
    else {
        sqlite3_close(db);
        exit(1);
    }

    int row,col,i,j;//分别表示查询结果的行数和列数，其中行数不包括列名
    char **res=NULL;//查询结果保存的地方，类似于string数组，结构是一维的，抽象来看是查询结果按行排列（包括列名）
    sqlite3_get_table(db,"select * from client",&res,&row,&col,NULL);

    for(i=0;i<=row;i++)
    {
        for(j=0;j<col;j++){
            printf("%s ",*(res+i*col+j));
        }
        printf("\n");
    }


    sqlite3_free_table(res);

}
