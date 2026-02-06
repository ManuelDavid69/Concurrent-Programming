CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin
OBJECTS := main.o tas.o

.PHONY: all clean

all: $(BIN)/tas

$(BIN):
	mkdir -p $(BIN)

$(BIN)/tas: $(OBJECTS) ../common/libpc.a | $(BIN)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

../common/libpc.a:
	$(MAKE) -C ../common

clean:
	rm -rf $(BIN) $(OBJECTS)
