CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin
PROGRAMS := init clean first second rdv2 rdv3 emetteur recepteur

.PHONY: all clean

all: $(addprefix $(BIN)/,$(PROGRAMS))

$(BIN):
	mkdir -p $(BIN)

$(BIN)/%: %.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

../common/libpc.a:
	$(MAKE) -C ../common

clean:
	rm -rf $(BIN)
