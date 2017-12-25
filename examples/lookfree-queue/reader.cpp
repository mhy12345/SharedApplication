#include <iostream>
#include <sys/signal.h>
#include "shared_queue.hpp"
using namespace std;
#define APPKEY 13131

shared_application :: SharedQueue<int,1024,APPKEY> sQueue;

volatile int keepRunning = 1;
void signalDealer(int id)
{
	if (id == SIGINT || id == SIGSEGV)
		keepRunning = 0;
}

int main()
{
	sQueue.init();
	signal(SIGINT,signalDealer);
	int cnt = 0;
	while (keepRunning)
	{
		int val;
		if (!sQueue.pop(val))
			continue;
		cnt ++;
		if (cnt % 100 == 0)
			cout<<cnt<<" "<<val<<endl;
	}
}
