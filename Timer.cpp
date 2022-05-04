#include "Timer.h"
#include <iostream>

using namespace std;

//������ʱ��ڵ�����ϵ����� 
void Timer::siftup(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        SwapNode(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

//����ʱ��ڵ� 
void Timer::SwapNode(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
} 

//ɾ���ڵ��ʼ���µ��� 
bool Timer::siftdown(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

//���ʱ��ڵ� 
void Timer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;
    //��ǰ�޸�����fd���ڶ�β�����½ڵ㲢ͨ��siftup()�Զѽ��е��� 
    if(ref_.count(id) == 0) {
		cout<<"insert newnode"<<endl;
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup(i);
    } 
    //����fd�Ѵ��ھ͸���������ʱ�䲢������ 
    else {
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown(i, heap_.size())) {
            siftup(i);
        }
    }
}

//ɾ��ָ��id��㣬�������ص�����
void Timer::doWork(int id) {
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del(i);
}

//ɾ��ָ��λ�õĽ��
void Timer::del(size_t index) {
    assert(!heap_.empty() && index >= 0 && index < heap_.size());
    //��Ҫɾ���Ľ�㻻����β��Ȼ�������
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if(i < n) {
        SwapNode(i, n);
        if(!siftdown(i, n)) {
            siftup(i);
        }
    }
    //ɾ����βԪ��
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

//����ָ��id�Ľ��
void Timer::adjust(int id, int timeout) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    siftdown(ref_[id], heap_.size());
}

//�Ĳ�������ѭ���ӶѶ���ʼ�����ʱ���
void Timer::tick() {
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty()) {
        TimerNode node = heap_.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();
        pop();
	cout<<"remove node"<<endl;
    }
}

//ɾ���Ѷ�Ԫ�� 
void Timer::pop() {
    assert(!heap_.empty());
    del(0);
}

void Timer::clear() {
    ref_.clear();
    heap_.clear();
}

//��ȡ��һ�ε����Ĳ����������ʱ�ڵ��ʱ���뵱ǰʱ���ʱ��� 
int Timer::GetNextTick() {
    tick();
    size_t res = -1;
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}
