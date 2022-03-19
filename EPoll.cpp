#include "EPoll.h"
#include <iostream>

using namespace std;

EPoll::EPoll(int maxEvent)
	:epollFd_(epoll_create(512)),
	events_(maxEvent)
	{
		assert(epollFd_ >= 0 && events_.size() > 0);
	}
	
EPoll::~EPoll() {
	close(epollFd_);
}

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

int EPoll::Wait(int timeoutMs) {
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int EPoll::GetEventFd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t EPoll::GetEvents(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}
