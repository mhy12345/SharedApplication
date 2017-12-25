#include "shared_application.hpp"
#include <sys/signal.h>
#define APPKEY 13131

shared_application :: SharedApplication<APPKEY> sApp;
volatile bool keepRunning = true;

void haha(int *data)
{
	printf("old value in the memory is ...\n");
	for (int i=0;i<10;i++) 
		printf("%d ",data[i]);printf("\n");
	for (int i=0;i<10;i++)
		data[i] = i;
	printf("new value in the memory is ...\n");
	for (int i=0;i<10;i++) 
		printf("%d ",data[i]);printf("\n");
	while(keepRunning)
	{
		sleep(1);
	}
}

void signalDealer(int id) {
	if (id == SIGINT) {
		keepRunning = 0;
	}
}

int main()
{
	sApp.setSize(10*sizeof(int));
	sApp.start();
	signal(SIGINT,signalDealer);
	int *data = (int*)sApp.malloc(10*sizeof(int));
	haha(data);
}
