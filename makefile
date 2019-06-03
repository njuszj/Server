sql_test : sql_test.c
	gcc sql_test.c sqlfunc.c -o sql_test -lsqlite3

