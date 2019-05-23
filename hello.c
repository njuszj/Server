#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sqlite3.h>

int main(){
	sqlite3 *db;
	char acCmd[128]="insert into client values('jeffrey','123456')";
	int ret=sqlite3_open("./data.db",&db);
	if(ret != SQLITE_OK)
	{
	    printf("open database fail!\n");
	    return -1;
	}
	ret = sqlite3_exec(db, acCmd, NULL, NULL, NULL);
	{
	    if(ret != SQLITE_OK)
	    {
		printf("exec fail,ret %d\n", ret);
		return -1;
	    }
	}
}
