#include <boost/lockfree/queue.hpp>  
#include "shared_application.hpp"
#include <iostream>
#include <cstdio>
#include <cmath>
#include <sys/signal.h>
#include <unistd.h>
using namespace std;
#define APPKEY 13131
shared_application :: SharedApplication<APPKEY> sApp;

volatile int keepRunning = 1;
void signalDealer(int id)
{
	if (id == SIGINT || id == SIGSEGV)
		keepRunning = 0;
}

typedef boost::lockfree::queue<timespec,
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<1024> > queue_t;

int main()
{
	int pid = getpid();
	printf("Client process with pid = %d\n",pid);
	char logfilename[100]; sprintf(logfilename,"logs/client_%d_details.txt",pid); FILE *logfile = fopen(logfilename,"w");
	if (!logfile)
		fprintf(stderr,"Fail to create %s\n",logfilename);
	signal(SIGINT,signalDealer);
	sApp.setSize(sizeof(queue_t));
	sApp.start();
	queue_t *sq;
   	if (sApp.isNewApp())
		sq = new(sApp.malloc(sizeof(queue_t))) queue_t;
	else
		sq = (queue_t*)sApp.malloc(sizeof(queue_t));
	timespec tv1,tv2;
	long long totalNSecs,totalCases;
	totalNSecs = totalCases = 0;
	int total = 0;
	while (keepRunning)
	{
		if (!sq->pop(tv1))continue;
		fprintf(logfile,"%ld,%ld\n",tv1.tv_sec,tv1.tv_nsec);
		total++;
		clock_gettime(CLOCK_REALTIME,&tv2);
		long tp = -((tv1.tv_sec - tv2.tv_sec)*1000000000 + (tv1.tv_nsec - tv2.tv_nsec));
		if (abs(tp) < 100000)
		{
			totalNSecs += tp;
			totalCases ++;
		}
		if (totalCases % 500 == 0)
			fprintf(stdout,"Client <%d> : Timeval = %lldns\n",pid,(long long)(totalNSecs/totalCases));
	}
	if (totalCases)
		fprintf(stderr,"Client <%d> : Timeval = %lldns\n",pid,(long long)(totalNSecs/totalCases));
	else
		fprintf(stderr,"Client <%d> : Read nothing\n",pid);
}
