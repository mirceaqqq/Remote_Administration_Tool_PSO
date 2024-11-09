all: server/server.c client/client.c
server: server/server.c
	gcc server/server.c -o server/server
client: client/client.c
	gcc client/client.c credentials_utils.c utils.c -o client/client