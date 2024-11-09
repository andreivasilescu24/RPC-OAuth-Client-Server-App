CC = g++
CFLAGS = -Wall -I/usr/include/tirpc -g

RPC_FILES = oauth.h oauth_clnt.cpp oauth_svc.cpp oauth_xdr.cpp

SERVER = server
CLIENT = client

SERVER_SRC = oauth_server.cpp oauth_svc.cpp oauth_xdr.cpp database_management.cpp database_management.h
CLIENT_SRC = oauth_client.cpp oauth_clnt.cpp oauth_xdr.cpp

build: 
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC) -lnsl -ltirpc	
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC) -lnsl -ltirpc

rpc:
	rpcgen -NC oauth.x

run_sv:
	./$(SERVER) tests/test1/userIDs.db tests/test1/resources.db tests/test1/approvals.db 2

run_cl:
	./$(CLIENT) localhost tests/test1/client.in

clean:
	rm -f $(SERVER) $(CLIENT) $(RPC_FILES)