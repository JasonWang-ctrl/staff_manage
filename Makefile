all:server.o client.o
	gcc server.o -lsqlite3 -o server
	gcc client.o -o client
server.o:server.c
	gcc -c server.c -o server.o
client.o:client.c
	gcc -c client.c -o client.o

clean:
	rm *.o
	rm server client
