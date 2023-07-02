CC = gcc
LIB = ./lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g
SHARED_FLAGS = -shared -fpic

target = get_num.so error_functions.so signal_functions.so

.PHONY: all clean

all: ${target}
# 编译动态库

get_num.so: $(LIB)/get_num.c $(LIB)/get_num.h
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<

error_functions.so: $(LIB)/error_functions.c $(LIB)/error_functions.h $(LIB)/tlsp_hdr.h $(LIB)/ename.c.inc
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<

signal_functions.so: $(LIB)/signal_functions.c error_functions.so $(LIB)/tlsp_hdr.h $(LIB)/signal_functions.h
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $< $(LIB)/error_functions.so

clean:
	-rm $(LIB)/*.so
