#ifndef __SQUEUE
#define __SQUEUE
#include <atomic>
#include <cassert>
#include <unistd.h>
#include <cstdio>
#include "shared_application.hpp"

#define MASK 0xc0000000u
#define TMASK 0x80000000u
#define HMASK 0x40000000u
#define TPACK(x) ((x)|TMASK)
#define HPACK(x) ((x)|HMASK)
#define UNPACK(x) ((x)&(~MASK))
#define GHEAD(x) ((x)&MASK)

namespace shared_application{
	template<typename T,unsigned capacity,int APPKEY>
		class SharedQueue : private SharedApplication<APPKEY>
	{
		private:
			class DataContainer{
				private:
					std::atomic<unsigned> head;
					T data[capacity];
					std::atomic<unsigned> tail;
					friend class SharedQueue<T,capacity,APPKEY>;
			};
			DataContainer* data;
			int default_capacity;
		public:
			SharedQueue():SharedApplication<APPKEY>()
			{
				SharedApplication<APPKEY>::setSize(sizeof(DataContainer));
				data = NULL;
			}
			void setCapacity(int newCapacity)
			{
				SharedApplication<APPKEY>::setSize(sizeof(DataContainer));
			}
			void init()
			{
				SharedApplication<APPKEY>::start();
				data = (DataContainer*) SharedApplication<APPKEY>::malloc(sizeof(DataContainer));
				if (SharedApplication<APPKEY>::isNewApp())
				{
					data->head.store(capacity-1,std::memory_order_release);
					data->tail.store(capacity-1,std::memory_order_release);
				}
			}
			void halt()
			{
				SharedApplication<APPKEY>::releaseInstance();
			}
			bool push(const T& item)
			{
				unsigned oldTail,newTail;
				oldTail = data->tail.load(std::memory_order_acquire);
				while (true)
				{
					if (GHEAD(oldTail))
					{
						oldTail = data->tail.load(std::memory_order_consume);
						continue;
					}
					newTail = oldTail+1 == capacity ? 0 : oldTail+1;
					if (newTail == UNPACK(data->head.load(std::memory_order_consume)))
						return false;
					if (!data->tail.compare_exchange_weak(oldTail,TPACK(oldTail),std::memory_order_acq_rel))
						continue;
					if (newTail == UNPACK(data->head.load(std::memory_order_relaxed)))
					{
						data->tail.store(oldTail,std::memory_order_release);
						return false;
					}
					data->data[newTail] = item;
					data->tail.store(newTail,std::memory_order_release);
					return true;
				}
				assert(false);
			}
			bool pop(T& item)
			{
				unsigned newHead,oldHead;
				oldHead = data->head.load(std::memory_order_consume);
				while (true)
				{
					if (GHEAD(oldHead))
					{
						oldHead = data->head.load(std::memory_order_consume);
						continue;
					}
					newHead = oldHead+1 == capacity ? 0 : oldHead+1;
					if (oldHead == UNPACK(data->tail.load(std::memory_order_consume)))
						return false;
					if (!data->head.compare_exchange_weak(oldHead,HPACK(oldHead),std::memory_order_acq_rel))
						continue;
					if (oldHead == UNPACK(data->tail.load(std::memory_order_relaxed)))
					{
						data->head.store(oldHead,std::memory_order_release);
						return false;
					}
					item = data->data[newHead];
					data->head.store(newHead,std::memory_order_release);
					return true;
				}
			}
		bool empty()
		{
			return UNPACK(data->head.load(std::memory_order_acquire)) == UNPACK(data->tail.load(std::memory_order_acquire));
		}
		bool full()
		{
			unsigned ctail = UNPACK(data->tail.load(std::memory_order_acquire));
			return (ctail+1==capacity?0:ctail+1) == UNPACK(data->head.load(std::memory_order_relaxed));
		}
	};
}

#endif
