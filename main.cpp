#include "Server.h"

int main() {
	//ָ���˿ںź��̳߳ص����� 
	int port = 4000;
	int threadnum = 6;
	//����server 
	Server server(threadnum,port);
	server.start();
}

