CC = gcc
LIB = ../lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g

.PHONY: all clean

target = $(patsubst %.c,%,$(wildcard *.c))


all: ${target}

%: %.c $(LIB)/tlsp_hdr.h $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $^ $(CFLAGS) -o $@ 

clean:
	-rm ${target}
