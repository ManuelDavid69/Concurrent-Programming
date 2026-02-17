CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin

.PHONY: all clean

all: $(BIN)/flou $(BIN)/digicode $(BIN)/table

$(BIN):
	mkdir -p $(BIN)

$(BIN)/flou: flou.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/digicode: digicode.c automate.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) digicode.c automate.c -o $@ $(LDLIBS)

$(BIN)/table: table.c automate.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) table.c automate.c -o $@ $(LDLIBS)

../common/libpc.a:
	$(MAKE) -C ../common

clean:
	rm -rf $(BIN)
