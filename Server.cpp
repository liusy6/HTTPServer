#include "Server.h"
#include "base/Util.h"
#include "log/Logging.h"
#include <assert.h>
#include <string.h>
#include <stdio.h> 
#include <stdlib.h>
#include <iostream>

using namespace std;

Server::Server(int threadnum, int port)
	:port_(port),
	isClose_(false),
	timer_(new Timer()),
	threadpool_(new ThreadPool(threadnum)),
	epoller_(new EPoll)
	{
		srcDir_ = getcwd(nullptr,256);
		assert(srcDir_);
		strncat(srcDir_, "/resources/", 16);
		HttpConn::userCount = 0;
    	HttpConn::srcDir = srcDir_;
    	//LOG << "srcdir = "<< srcDir_;
    	//设置为ET模式 
    	listenEvent_ |= EPOLLET;
    	connEvent_ |= EPOLLET;
    	HttpConn::isET = (connEvent_ & EPOLLET);
    	//服务器初始化 
    	if(!Init())  isClose_ = true;
	}

Server::~Server() {
	close(listenFd_);
	isClose_ = true;
	free(srcDir_);
}

void Server::start() {
	int timeMS = -1;
	//循环调用需要的函数 
	while(!isClose_) {
		timeMS = timer_->GetNextTick();
		//调用epoll_wait()（阻塞IO） 
		int eventCount = epoller_->Wait(timeMS);
		for(int i = 0; i < eventCount; ++i) {
			int fd = epoller_->GetEventFd(i);
			//LOG << "fd = "<< fd;
			cout<<"fd = "<<fd<<endl;
			//获取已就绪fd对应的事件 
			uint32_t events = epoller_->GetEvents(i); 
			if(fd == listenFd_) HandleListen();
			//当客户端关闭或者出现错误就关闭连接 
			else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
			}
			//处理读 
			else if(events & EPOLLIN) {
				assert(users_.count(fd) > 0);
                HandleRead(&users_[fd]);
			}
			//处理写
			else if(events & EPOLLOUT) {
				assert(users_.count(fd) > 0);
                HandleWrite(&users_[fd]);
			}
			else { cout<<"no event"<<endl; }
		}
	}
}	

void Server::SendError(int fd, const char* info) {
	assert(fd>0);
	int ret = send(fd, info, strlen(info), 0);
	close(fd);
}

//关闭连接 
void Server::CloseConn(HttpConn* client) {
	assert(client);
	//LOG<<"closeconn fd:"<<client->GetFd();
	cout<<"closeconn"<<endl;
	epoller_->DelFd(client->GetFd());
	client->Close();
}

//添加客户 
void Server::AddClient(int fd, sockaddr_in addr) {
	assert(fd > 0);
	cout<<"add client fd："<<fd<<endl;
	users_[fd].init(fd, addr);
	//添加进时间堆中，超时回调CloseConn函数 
	timer_->add(fd, TimeoutMS, std::bind(&Server::CloseConn, this, &users_[fd]));
	epoller_->AddFd(fd, EPOLLIN | connEvent_);
	setSocketNodelay(fd);
}

//处理监听事件 
void Server::HandleListen() {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	do {
        	int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
			cout<<"fd = "<<fd<<endl;
        	if(fd <= 0)  {return;}
        	else if(HttpConn::userCount >= MAX_FD) {
            		SendError(fd, "Server busy!");
			return;
        	}
        	AddClient(fd, addr);
    	} while(listenEvent_ & EPOLLET);	
}
 
void Server::HandleRead(HttpConn* client) {
	assert(client);
	ExtentTime(client);
	cout<<"handleread"<<endl;
	//往线程池中添加任务 
    threadpool_->AddTask(std::bind(&Server::OnRead, this, client));
}

void Server::HandleWrite(HttpConn* client) {
	assert(client);
	ExtentTime(client);
	cout<<"handlewrite"<<endl;
	//往线程池中添加任务
	threadpool_->AddTask(std::bind(&Server::OnWrite, this, client));
}

//当未关闭连接的fd出现新的操作就更新其消亡时间并调整时间堆 
void Server::ExtentTime(HttpConn* client) {
	assert(client);
	timer_->adjust(client->GetFd(), TimeoutMS);
}

void Server::OnRead(HttpConn* client) {
	assert(client);
    int readErrno = 0;
    int ret = client->read(&readErrno);
    cout<<"ret = "<<ret<<endl;
    //读取失败关闭连接，成功则继续处理 
    if(ret <= 0 && readErrno != EAGAIN) {
        CloseConn(client);
	return;
    }
    OnProcess(client);
}

void Server::OnProcess(HttpConn* client) { 
    if(client->process()) {
	cout<<"out"<<endl;
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    } 
	else {
	cout<<"in"<<endl;
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void Server::OnWrite(HttpConn* client) {
    assert(client);
	cout<<"write"<<endl;
    int writeErrno = 0;
    int ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        if(client->IsKeepAlive()) {
            OnProcess(client);
	return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
		return;
        }
    }
    CloseConn(client);
}

//初始化 
bool Server::Init() {
	//socket创建、绑定并监听，返回监听fd 
	listenFd_ = socket_bind_listen(port_);
	assert(listenFd_);
	cout<<"listenfd = "<<listenFd_<<endl;
	int ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
	if (ret == 0) {
		close(listenFd_);
		return false;
	}
	setSocketNodelay(listenFd_);
	return true;
}
