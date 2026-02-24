CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I../common
LDLIBS ?= ../common/libpc.a
BIN := bin
PROGRAMS := miroir moyenne boucle wait enfants analyse-cas1 analyse-cas2 analyse-cas2-ordonne analyse-cas3

.PHONY: all clean

all: $(addprefix $(BIN)/,$(PROGRAMS))

$(BIN):
	mkdir -p $(BIN)

$(BIN)/miroir: miroir.c | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BIN)/moyenne: moyenne.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/boucle: boucle.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/wait: wait.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/enfants: enfants.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/analyse-cas1: analyse/cas1.c | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BIN)/analyse-cas2: analyse/cas2.c ../common/libpc.a | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BIN)/analyse-cas2-ordonne: analyse/cas2_ordonne.c | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

$(BIN)/analyse-cas3: analyse/cas3.c | $(BIN)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

../common/libpc.a:
	$(MAKE) -C ../common

clean:
	rm -rf $(BIN)
