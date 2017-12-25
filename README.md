# SharedApplication

## Introduction

SharedApplication is a class used for sharing data between process

## Usage

A global instance of SharedApplication is required

	SharedApplication sApp;

At the begin position of the main function, we can do some basic configuration:

	sApp.setSize(1024*sizeof(int));

The function setSize replace the default size 1024(in bytes) with the user defined size. 

After the configurations, you can start the sApp, simple write:

	sApp.start();

The, in you program, you can use malloc to alloc memory.

	void* ptr = sApp.malloc(size);

But remember, the total size should not excess the limitaion.

In the other program, you can also writing the same code. And the contents in the memory allocated is synchronized.

*Make sure you alloc each memory block using the same order and size*
