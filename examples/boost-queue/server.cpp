#include <boost/lockfree/queue.hpp>  
#include "shared_application.hpp"
#include <iostream>
#include <cstdio>
#include <random>
#include <sys/signal.h>
#include <ctime>
using namespace std;
#define APPKEY 13131
shared_application :: SharedApplication<APPKEY> sApp;

volatile int keepRunning = 1;
void signalDealer(int id)
{
	if (id == SIGINT || id == SIGSEGV)
	{
		keepRunning = 0;
	}
}

typedef boost::lockfree::queue<timespec,
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<1024> > queue_t;

int main()
{
	int pid = getpid();

	printf("Server process with pid = %d\n",pid);
	char logfilename[100]; sprintf(logfilename,"logs/server_%d_details.txt",pid);
	FILE *logfile = fopen(logfilename,"w");
	if (!logfile)
		fprintf(stderr,"Fail to create %s\n",logfilename);

	signal(SIGINT,signalDealer);
	sApp.setSize(sizeof(queue_t));//set the default size to fit the queue
	sApp.start();
	queue_t *sq;
   	if (sApp.isNewApp())
		sq = new(sApp.malloc(sizeof(queue_t))) queue_t;
	else
		sq = (queue_t*)sApp.malloc(sizeof(queue_t));
	timespec tv;
	int total = 0;
	while (keepRunning)
	{
		clock_gettime(CLOCK_REALTIME, &tv);
		total += sq->bounded_push(tv);
		fprintf(logfile,"%ld,%ld\n",tv.tv_sec,tv.tv_nsec);
		if (total%1000==0)
			printf("%d:Write %lds%ldus\n",total,tv.tv_sec,tv.tv_nsec);
		usleep(rand()%10+2);
	}
}

