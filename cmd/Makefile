CC = gcc
SRC = ./src
INCLUDE = ./include
BIN = ./bin
CFLAGS = -O0 -I$(INCLUDE) -Wall -Wextra -g

all: cp cat head tail od chmod chown pwd

cp: $(BIN)/cp.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

cat: $(BIN)/cat.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

head: $(BIN)/head.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

tail: $(BIN)/tail.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^
	
od: $(BIN)/od.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^
	
chmod: $(BIN)/chmod.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

chown: $(BIN)/chown.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

pwd: $(BIN)/pwd.o $(BIN)/error.o
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

$(BIN)/%.o: $(SRC)/%.c $(INCLUDE)/apue.h $(INCLUDE)/system.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:
clean:
	-rm $(BIN)/*
