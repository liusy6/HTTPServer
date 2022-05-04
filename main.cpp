#include "Server.h"

int main() {
	//指定端口号和线程池的数量 
	int port = 4000;
	int threadnum = 6;
	//启动server 
	Server server(threadnum,port);
	server.start();
}

