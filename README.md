# SharedApplication

## Introduction

SharedApplication is a class used for sharing data between processes, and design to be derived by multi-process data structure.

SharedQueue is a lockfree queue with fixed capacity, derived from the SharedApplication. Easy to use.

## Usage

[SharedApplication](#jump1)

[SharedQueue](#jump2)

### SharedApplication
<span id="jump1"></span>

Define a SharedApplication instance in the global namescope

	SharedApplication sApp;

At the begin of the main function, we can do some basic configuration:

	sApp.setSize(size_t size);
	
Set the size of the memory you are going to alloc, and this must called before start. If it wasn't called, then the default size will be 1024 bytes.

	void setCPU();
	void setCPU(int id);

If you are trying to binding particular CPU (for the better performance), then you may call the setCPU() to automatically bind a CPU;

After the configurations, you can start the sApp, simple write:

	sApp.start();

Then, in you program, you can use malloc to alloc memory.

	void* ptr = sApp.malloc(size_t size);

But remember, the total size should not excess the capacity decloration before started.

In other program, you can also write the same code. And the contents in the memory allocated is synchronized.


### SharedQueue
<span id="jump2"></span>

The SharedQueue is a private inherited form the SharedApplication, means you can just replace SharedApplication with SharedQueue.


	shared_application :: SharedQueue<T,capacity,APPKEY> sQueue;

In the main function, you can use...

	sQueue::init();//called first
	sQueue::push(T &val);
	sQueue::pop(T &val);
	sQueue::full(T &val);
	sQueue::empty(T &val);

## Attention
1. In the SharedApplication, make sure you alloc each memory block using the same order and size
2. The destructive function must be called, otherwise the shared memory cannot be deleted.






