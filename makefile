all:sql_test server print_sql socket_test
server:server.c
	gcc server.c sqlfunc.c -o server -lpthread -lsqlite3
sql_test : sql_test.c
	gcc sql_test.c sqlfunc.c -o sql_test -lsqlite3
print_sql:print_sql.c
	gcc print_sql.c -o print_sql -lsqlite3
socket_test:socket_test.c
	gcc socket_test.c -o socket_test
