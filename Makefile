ROOT := $(shell pwd)
INCLUDE := $(ROOT)/include
EXAMPLES := $(ROOT)/examples

export CC = g++
export CXXFLAGS = -std=c++11 -g -I $(INCLUDE) -DDEBUG
EXAMPLES_SUB_DIR := $(EXAMPLES)/sample
  
files := $(shell find ./examples -name "makefile" -o -name "Makefile")

.PHONY: all

all:
	@for i in $(files); do $(MAKE) -C `dirname $$i` ; done  

clean:
	@echo $(files);
	@for i in $(files); do $(MAKE) -C `dirname $$i` clean; done  
