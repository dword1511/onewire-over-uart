OBJS   += onewire.o
CC     ?= gcc
CFLAGS += -O2 -Wall

ifeq ($(OS), Windows_NT)
	OBJS += uart_win.o
else
	OBJS += uart_posix.o
endif

OBJS1 = $(OBJS) lsow.o
OBJS2 = $(OBJS) digitemp_evil.o devices/common.o devices/ds18x20.o

all: lsow digitemp_evil

lsow: $(OBJS1)
	$(CC) -static -O3 -o $@ $(OBJS1)

digitemp_evil: $(OBJS2)
	$(CC) -static -O3 -o $@ $(OBJS2)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS1) $(OBJS2) uart_*.o lsow digitemp_evil README.html

doc: README.md
	markdown README.md > README.html

install: lsow digitemp_evil
	install -m 0755 lsow /usr/local/bin/lsow
	install -m 0755 digitemp_evil /usr/local/bin/digitemp_evil

uninstall:
	rm /usr/local/bin/lsow /usr/local/bin/digitemp_evil
