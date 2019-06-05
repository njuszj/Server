all:sql_test client server
server:server.c
	gcc server.c sqlfunc.c -o server -lpthread -lsqlite3
client:client.c
	gcc client.c -o client
sql_test : sql_test.c
	gcc sql_test.c sqlfunc.c -o sql_test -lsqlite3

