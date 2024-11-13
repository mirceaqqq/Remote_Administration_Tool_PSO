all: 
	gcc server/server.c commonutils.c -o server/server
	gcc client/client.c client/system_info_utils.c client/credentials_utils.c client/utils.c commonutils.c -o client/client
server: server/server.c
	gcc server/server.c commonutils.c -o server/server
client: client/client.c
	gcc client/client.c credentials_utils.c  utils.c commonutils.c -o client/client