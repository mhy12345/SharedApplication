#ifndef __SQUEUE
#define __SQUEUE
#include <atomic>
#include <cassert>
#include <unistd.h>
#include <cstdio>

#define MASK 0xc0000000u
#define TMASK 0x80000000u
#define HMASK 0x40000000u
#define TPACK(x) ((x)|TMASK)
#define HPACK(x) ((x)|HMASK)
#define UNPACK(x) ((x)&(~MASK))
#define GHEAD(x) ((x)&MASK)

template<typename T,unsigned capacity>
class squeue
{
	private:
		std::atomic<unsigned> head;
		T data[capacity];
		std::atomic<unsigned> tail;
	public:
		squeue()
		{
			/*
			 * 初始情况 head = tail = -1
			 * 含义为
			 * head : pop操作上一次pop的位置
			 * tail : push操作上一次push的位置
			 *
			 * 则tail+1对应的位置应该是不能被head+1占用的，所以通过锁实现
			 * */
			head.store(capacity-1,std::memory_order_release);
			tail.store(capacity-1,std::memory_order_release);
		}
		void status()
		{
			fprintf(stderr,"%d %d\n",head.load(),tail.load());
		}
		bool empty()
		{
			return UNPACK(head.load(std::memory_order_acquire)) == UNPACK(tail.load(std::memory_order_acquire));
		}
		bool full()
		{
			unsigned ctail = UNPACK(tail.load(std::memory_order_acquire));
			return (ctail+1==capacity?0:ctail+1) == UNPACK(head.load(std::memory_order_relaxed));
		}
		bool push(const T& item)
		{
			unsigned oldTail,newTail;
			oldTail = tail.load(std::memory_order_acquire);
			while (true)
			{
				if (GHEAD(oldTail))
				{
					oldTail = tail.load(std::memory_order_consume);
					continue;
				}
				newTail = oldTail+1 == capacity ? 0 : oldTail+1;
				if (newTail == UNPACK(head.load(std::memory_order_consume)))
					return false;
				if (!tail.compare_exchange_weak(oldTail,TPACK(oldTail),std::memory_order_acq_rel))
					continue;
				if (newTail == UNPACK(head.load(std::memory_order_relaxed)))
				{
					tail.store(oldTail,std::memory_order_release);
					return false;
				}
				data[newTail] = item;
				tail.store(newTail,std::memory_order_release);
				return true;
			}
			assert(false);
		}
		bool pop(T& item)
		{
			unsigned newHead,oldHead;
			oldHead = head.load(std::memory_order_consume);
			while (true)
			{
				if (GHEAD(oldHead))
				{
					oldHead = head.load(std::memory_order_consume);
					continue;
				}
				newHead = oldHead+1 == capacity ? 0 : oldHead+1;
				if (oldHead == UNPACK(tail.load(std::memory_order_consume)))
					return false;
				if (!head.compare_exchange_weak(oldHead,HPACK(oldHead),std::memory_order_acq_rel))
					continue;
				if (oldHead == UNPACK(tail.load(std::memory_order_relaxed)))
				{
					head.store(oldHead,std::memory_order_release);
					//assert(head.exchange(oldHead) == HPACK(oldHead));
					return false;
				}
				item = data[newHead];
				head.store(newHead,std::memory_order_release);
				//assert(head.exchange(newHead) == HPACK(oldHead));
				return true;
			}
		}
};

#endif
