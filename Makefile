CC = gcc
LIB = ./lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g
SHARED_FLAGS = -shared -fpic

target = get_num.so error_functions.so

.PHONY: all clean

all: ${target}

# seek: $(BIN)/seek.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# hole: $(BIN)/hole.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# copy: $(BIN)/copy.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# append: $(BIN)/append.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# fork-then-open: $(BIN)/fork-then-open.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

# open-then-fork: $(BIN)/open-then-fork.o $(BIN)/error.o
# 	$(CC) $(CFLAGS) -o $(BIN)/$@ $^


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

# 编译动态库

get_num.so: $(LIB)/get_num.c $(LIB)/get_num.h
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<


error_functions.so: $(LIB)/error_functions.c $(LIB)/error_functions.h $(LIB)/tlsp_hdr.h $(LIB)/ename.c.inc
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<

clean:
	-rm $(LIB)/*.so
