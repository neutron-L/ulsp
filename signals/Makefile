CC = gcc
LIB = ../lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g

.PHONY: all clean

target = ouch intquit t_kill sigset sig_sender sig_receiver raise_sig ignore_kill_stop ignore_sig


all: ${target}

%: %.c $(LIB)/tlsp_hdr.h $(LIB)/get_num.so $(LIB)/error_functions.so 
	$(CC) $(CFLAGS) -o $@ $^

sigset: sigset.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so 
	$(CC) $(CFLAGS) -o $@ $^

sig_sender: sig_sender.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $(CFLAGS) -o $@ $^


sig_receiver: sig_receiver.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $(CFLAGS) -o $@ $^

	
ignore_sig: ignore_sig.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $(CFLAGS) -o $@ $^


raise_sig: raise_sig.c $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.so  $(LIB)/error_functions.so $(LIB)/get_num.so 
	$(CC) $(CFLAGS) -o $@ $^


# append-seek: $(BIN)/append-seek.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# fileinfo: $(BIN)/fileinfo.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# acfile: $(BIN)/acfile.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# umask: $(BIN)/umask.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# truncfile: $(BIN)/truncfile.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# unlink: $(BIN)/unlink.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# wd: $(BIN)/wd.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# parse_args: $(BIN)/parse_args.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# jmp: $(BIN)/jmp.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

	
# limit: $(BIN)/limit.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# open-append-rw: $(BIN)/open-append-rw.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# getfl: $(BIN)/getfl.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# filed: $(BIN)/filed.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# shared_file: $(BIN)/shared_file.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# id: $(BIN)/id.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# link: $(BIN)/link.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# modefile: $(BIN)/modefile.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# fork-process: $(BIN)/fork-process.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# vfork-process: $(BIN)/vfork-process.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


# ex-exit: $(BIN)/ex-exit.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

clean:
	-rm ${target}
