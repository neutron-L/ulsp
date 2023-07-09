CC = gcc
LIB = ./lib
CFLAGS = -O0 -I$(LIB) -Wall -Wextra -g
SHARED_FLAGS = -shared -fpic

target = $(patsubst $(LIB)/%.c,%.so,$(wildcard $(LIB)/*.c))

.PHONY: all clean

all: ${target}
	@echo "compile all shared library"
	@echo ${target}
# 编译动态库

%.so: $(LIB)/%.c $(LIB)/%.h
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<

error_functions.so: $(LIB)/error_functions.c $(LIB)/error_functions.h $(LIB)/tlsp_hdr.h $(LIB)/ename.c.inc
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $<

signal_functions.so: $(LIB)/signal_functions.c $(LIB)/signal_functions.h error_functions.so $(LIB)/tlsp_hdr.h
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $(LIB)/$@ $< $(LIB)/error_functions.so

clean:
	-rm $(LIB)/*.so
