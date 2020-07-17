TARGET: exe

exe: server.o client.o socket.o
	gcc server/server.o server/socket.o  -g -o server/server
	gcc client/client.o client/socket.o  -g -o client/client

server.o:
	gcc -g -c -Wall server/server.c -o server/server.o

client.o:
	gcc -g -c -Wall client/client.c -o client/client.o

socket.o:
	gcc -g -c -Wall server/socket.c -o server/socket.o
	gcc -g -c -Wall client/socket.c -o client/socket.o

clean:
	rm -rf server/server.o client/client.o server/socket.o client/socket.o server/server client/client
