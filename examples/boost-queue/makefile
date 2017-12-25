HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.cpp)
TARGETS = $(patsubst %.cpp,%,$(SOURCES))

all : target

target : $(TARGETS)

%.o : $(HEADERS)

clean : 
	rm *.o server client
	rm -f logs/*
