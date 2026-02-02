CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -g -O2
MAKE ?= make

TPS := tp01 tp02 tp03 tp04 tp05 tp06 tp07 tp08 tp09 tp10 tp11 tp12 tp14 pipeline

.PHONY: all common clean check test $(TPS)

all: common
	@for dir in $(TPS); do \
		echo "==> $$dir"; \
		$(MAKE) -C $$dir || exit 1; \
	done

common:
	$(MAKE) -C common

$(TPS):
	$(MAKE) -C $@

clean:
	$(MAKE) -C common clean
	@for dir in $(TPS); do $(MAKE) -C $$dir clean; done

check:
	$(MAKE) clean
	$(MAKE) CFLAGS="-std=c11 -Wall -Wextra -Werror -g -O2" all

test: all
	python3 -m pytest -q
