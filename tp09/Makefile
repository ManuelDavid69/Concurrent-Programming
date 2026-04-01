CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin
PROGRAMS := exo1 exo2 exo3 exo4 exo5 exo6 exo7

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
