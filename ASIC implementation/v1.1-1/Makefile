# Makefile for Mithril PLI with libsodium

CC = gcc
CFLAGS = -fPIC -Wall -I$(VCS_HOME)/include -I/usr/local/include
LDFLAGS = -shared -lsodium

PLI_LIB = mithril_pli.so

all: $(PLI_LIB)

$(PLI_LIB): mithril_pli_libsodium.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(PLI_LIB) *.o

test: $(PLI_LIB)
	vcs -full64 -sverilog +vpi -P $(PLI_LIB) \
	    ../rtl/mithril_add_secure.v \
	    ../tb/tb_mithril_add.v \
	    -o simv
	./simv

.PHONY: all clean test
