#ifndef __SHARED_APPLICATION_H
#define __SHARED_APPLICATION_H
#include <atomic>
#include <cstdlib>
#include <sys/shm.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <new>
#include <stdexcept>
#define APPKEY 112341

class MemoryBlock
{
	public:
		size_t dataSize;
		std::atomic_int timestamp;
		std::atomic_int total;
		MemoryBlock():total(0)
	{
	}
};

class SharedApplication
{
	private:
		size_t currentIndex;
		size_t defaultSize;
		int appId;
		bool newApp;
		MemoryBlock* info;
	public:
		SharedApplication()
		{
			defaultSize = 1024;
			info = NULL;
			currentIndex = 0;
		}
		~SharedApplication(){
			releaseInstance();
		}
		void* malloc(size_t s){
			if (!info)
			{
				printf("Error not initialized???\n");
				return NULL;
			}
			if (currentIndex + s > info->dataSize) {
				printf("Data Buffer is full!\n");
				return NULL;
			}
			printf(">>MALLOC %u\n",s);
			void* ret = (char*)(info+1) + currentIndex;
			currentIndex += s;
			return ret;
		}
		void setSize(size_t s){
			if (info)
			{
				printf("Cannot set size after started!\n");
				return ;
			}
			this->defaultSize = s;
		}
		void start()
		{
			newApp = false;
			if ((appId = shmget(APPKEY,sizeof(MemoryBlock) + defaultSize,0666)) == -1)
			{
				if ((appId = shmget(APPKEY,sizeof(MemoryBlock)+defaultSize,0666 | IPC_CREAT)) == -1)
				{
					printf("CANNOT CREATE SHM!\n");
					return ;
				}
				printf("Shared Memory Segment not exist, creat one.\n");
				newApp = true;
			}
			printf("Create header block<%d>\n",appId);
			info = (MemoryBlock*)shmat(appId,NULL,0);
			if (info == (void*)-1) {
				{
					printf("Cannot alloc shm\n");return;
				}
			}
			if (newApp)
			{
				info = new(info) MemoryBlock();
				info->dataSize = defaultSize;
			}
			info->total += 1;
		}
		void releaseInstance() {
			if (!info)
				return ;
			printf("Release instance...\n");
			info->total -= 1;
			if (info->total.load() == 0)
			{
				shmdt(info);
				shmctl(appId,IPC_RMID,NULL);
				info = NULL;
				printf("Delete header block<%d>\n",appId);
			}else
			{
				shmdt(info);
				info = NULL;
				printf("Detach header block<%d>\n",appId);
			}
		}
		bool isNewApp()
		{
			return newApp;
		}
		size_t getIndex()
		{
			return currentIndex;
		}
		void setIndex(size_t currentIndex)
		{
			this->currentIndex = currentIndex;
		}
		void* malloc_save(size_t size,size_t &oldIndex)
		{
			oldIndex = currentIndex;
			return malloc(size);
		}
		void setCPU()
		{
			setCPU(this->info->total);
		}
		void setCPU(int id)
		{
			cpu_set_t mask;  
			int i = 0;  
			int num = sysconf(_SC_NPROCESSORS_CONF);    //获取当前的cpu总数  
			pthread_detach(pthread_self());  
			CPU_ZERO(&mask);      
			CPU_SET(id, &mask);      //绑定cpu 1  
			if(sched_setaffinity(0, sizeof(mask), &mask) == -1)      //0 代表对当前线程/进程进行设置。  
			{  
				printf("set affinity failed..");  
			}  
			CPU_ZERO(&mask);  
			if(sched_getaffinity(0, sizeof(mask), &mask) == -1)   
			{  
				printf("get failed..\n");  
			}  

			for(i = 0; i < num; i++)  
			{  
				if(CPU_ISSET(i, &mask))  
					printf("new thread %d run on processor %d\n", getpid(), i);  
			}  
		}
};

#endif
