CC = gcc
LIB = ../lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g

.PHONY: all clean

target = $(patsubst %.c,%,$(wildcard *.c))


all: ${target}

%: %.c $(LIB)/tlsp_hdr.h $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $^ $(CFLAGS) -o $@ 

fork_signal: fork_signal.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $^ $(CFLAGS) -o $@ 

multi_wait: multi_wait.c $(LIB)/tlsp_hdr.h $(LIB)/error_functions.so $(LIB)/get_num.so $(LIB)/curr_time.so 
	$(CC) $^ $(CFLAGS) -o $@ 

child_status: child_status.c $(LIB)/tlsp_hdr.h $(LIB)/error_functions.so $(LIB)/get_num.so $(LIB)/curr_time.so $(LIB)/print_wait_status.so 
	$(CC) $^ $(CFLAGS) -o $@ 

multi_SIGCHLD: multi_SIGCHLD.c $(LIB)/tlsp_hdr.h $(LIB)/error_functions.so $(LIB)/get_num.so $(LIB)/curr_time.so $(LIB)/print_wait_status.so 
	$(CC) $^ $(CFLAGS) -o $@ 


clean:
	-rm ${target}
