#pragma once

#include <sys/epoll.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <assert.h> 
#include <vector>
#include <errno.h>
#include "base/Define.h"

class EPoll {
	public:
		explicit EPoll(int maxEvent = MAX_EVENT);
		~EPoll();
		bool AddFd(int fd, uint32_t events);
		bool ModFd(int fd, uint32_t events);
		bool DelFd(int fd);
		int Wait(int timeoutMs = -1);
		int GetEventFd(size_t i) const;
		uint32_t GetEvents(size_t i) const;
		
	private:
		int epollFd_;
		std::vector<struct epoll_event> events_;	
};
