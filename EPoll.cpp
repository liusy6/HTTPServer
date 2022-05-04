#include "EPoll.h"
#include <iostream>

using namespace std;

//通过epoll_create()创建eventpoll返回epfd 
EPoll::EPoll(int maxEvent)
	:epollFd_(epoll_create(512)),
	events_(maxEvent)
	{
		assert(epollFd_ >= 0 && events_.size() > 0);
	}
	
EPoll::~EPoll() {
	close(epollFd_);
}

//AddFd、ModFd、DelFd函数都是通过系统调用epoll_ctl()对需要检查socket_fd进行增删改 
bool EPoll::AddFd(int fd, uint32_t events) {
	if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
	bool a = (0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev));
    return a;
}

bool EPoll::ModFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev);
}

bool EPoll::DelFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev);
}

//通过调用epoll_wait()得到已准备就绪的fd 
int EPoll::Wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

//获取已就绪的socket_fd
int EPoll::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

//获取已就绪fd对应的事件
uint32_t EPoll::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}
