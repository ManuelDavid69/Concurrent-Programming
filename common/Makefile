CC ?= cc
AR ?= ar
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
CPPFLAGS ?= -I.

LIBRARY := libpc.a
OBJECTS := common.o image.o sysv.o

.PHONY: all clean

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(LIBRARY)
