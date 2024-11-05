CC = gcc
CFLAGS = -Wall -I/usr/include/tirpc -g

RPC_FILES = oauth.h oauth_clnt.c oauth_svc.c oauth_xdr.c

SERVER = server
CLIENT = client

SERVER_SRC = oauth_server.c oauth_svc.c oauth_xdr.c
CLIENT_SRC = oauth_client.c oauth_clnt.c oauth_xdr.c

build: 
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC) -lnsl -ltirpc	
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC) -lnsl -ltirpc

rpc:
	rpcgen -aNC oauth.x

clean:
	rm -f $(SERVER) $(CLIENT) $(RPC_FILES)