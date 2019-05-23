all:hello client server
hello:hello.c
	gcc -Wall hello.c -o hello
client:client.c
	gcc -Wall client.c -o client
server:server.c
	gcc -Wall server.c -o server -lsqlite3

