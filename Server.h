#pragma once
#include <tr1/unordered_map>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "base/Define.h"
#include "EPoll.h"
#include "Timer.h"
#include "ThreadPool.h"
#include "http/HttpConn.h"

class Server {
	public:
		Server(int threadnum, int port);
		~Server();
		void start();
		
	private:
		bool Init();
		void AddClient(int fd, sockaddr_in addr);
		void HandleListen();
		void HandleWrite(HttpConn* client);
		void HandleRead(HttpConn* client);
		
		void SendError(int fd, const char* info);
		
		void ExtentTime(HttpConn* client);
		void CloseConn(HttpConn* client);
		
		void OnRead(HttpConn* client);
		void OnWrite(HttpConn* client);
		void OnProcess(HttpConn* client);
		
		int port_;
		bool isClose_;
		int listenFd_;
		char* srcDir_;
		
		uint32_t listenEvent_;
    	uint32_t connEvent_;
		
		std::unique_ptr<Timer> timer_;
    	std::unique_ptr<ThreadPool> threadpool_;
    	std::unique_ptr<EPoll> epoller_;
    	std::unordered_map<int, HttpConn> users_; 
};
