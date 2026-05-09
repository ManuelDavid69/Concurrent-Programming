CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2 -pthread
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a -pthread -lm
BIN := bin
OBJECTS := pipeline.o gpu.o

.PHONY: all clean

all: $(BIN)/pipeline

$(BIN):
	mkdir -p $(BIN)

$(BIN)/pipeline: $(OBJECTS) ../common/libpc.a | $(BIN)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

../common/libpc.a:
	$(MAKE) -C ../common

clean:
	rm -rf $(BIN) $(OBJECTS) *.ppm
