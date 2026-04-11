CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin
PROGRAMS := init clean rdv_n prod conso p0 p1 p2

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
