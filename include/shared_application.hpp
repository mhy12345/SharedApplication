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
namespace shared_application {
	template<int APPKEY>
		class SharedApplication
		{
			private:
				class MemoryBlock
				{
					public:
						size_t dataSize;
						std::atomic_int total;
						MemoryBlock():total(0)
					{
					}
				};
				size_t currentIndex;
				size_t defaultSize;
				int appId;
				bool newApp;
				MemoryBlock* header;
			public:
				SharedApplication()
				{
					defaultSize = 1024;
					header = NULL;
					currentIndex = 0;
				}
				~SharedApplication(){
					releaseInstance();
				}
				void* malloc(size_t s){
					if (!header)
					{
						fprintf(stderr,"Error not initialized???\n");
						return NULL;
					}
					if (currentIndex + s > header->dataSize) {
						fprintf(stderr,"Data Buffer is full!\n");
						return NULL;
					}
					void* ret = (char*)(header+1) + currentIndex;
					currentIndex += s;
					return ret;
				}
				void setSize(size_t s){
					if (header)
					{
						fprintf(stderr,"Cannot set size after started!\n");
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
							fprintf(stderr,"CANNOT CREATE SHM!\n");
							return ;
						}
#ifdef DEBUG
						printf("Shared Memory Segment not exist, creat one.\n");
#endif
						newApp = true;
					}
#ifdef DEBUG
					printf("Create header block<%d>\n",appId);
#endif
					header = (MemoryBlock*)shmat(appId,NULL,0);
					if (header == (void*)-1) {
						{
							fprintf(stderr,"Cannot alloc shm\n");return;
						}
					}
					if (newApp)
					{
						header = new(header) MemoryBlock();
						header->dataSize = defaultSize;
					}
					header->total += 1;
				}
				void releaseInstance() {
					if (!header)
						return ;
#ifdef DEBUG
					printf("Release instance...\n");
#endif
					header->total -= 1;
					if (header->total.load() == 0)
					{
						shmdt(header);
						shmctl(appId,IPC_RMID,NULL);
						header = NULL;
#ifdef DEBUG
						printf("Delete header block<%d>\n",appId);
#endif
					}else
					{
						shmdt(header);
						header = NULL;
#ifdef DEBUG
						printf("Detach header block<%d>\n",appId);
#endif
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
					setCPU(this->header->total);
				}
				void setCPU(int id)
				{
					cpu_set_t mask;  
					int i = 0;  
					int num = sysconf(_SC_NPROCESSORS_CONF);
					pthread_detach(pthread_self());  
					CPU_ZERO(&mask);      
					CPU_SET(id, &mask);
					if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
						printf("set affinity failed..");  
					CPU_ZERO(&mask);  
					if(sched_getaffinity(0, sizeof(mask), &mask) == -1)   
						printf("get failed..\n");  

					for(i = 0; i < num; i++)  
						if(CPU_ISSET(i, &mask))  
							printf("new thread %d run on processor %d\n", getpid(), i);  
				}
		};
}
#endif
