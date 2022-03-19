#include "Server.h"

int main() {
	int port = 4000;
	int threadnum = 6;
	Server server(threadnum,port);
	server.start();
}

