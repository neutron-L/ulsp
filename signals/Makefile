CC = gcc
LIB = ../lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g

.PHONY: all clean

target = ouch intquit t_kill sigset sig_sender sig_receiver raise_sig ignore_kill_stop


all: ${target}

%: %.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm ${target}
