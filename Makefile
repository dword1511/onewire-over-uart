OBJS += onewire.o

ifeq ($(OS), Windows_NT)
	OBJS += uart_win.o
else
	OBJS += uart_posix.o
endif

OBJS1 = $(OBJS) lsow.o
OBJS2 = $(OBJS) digitemp_evil.o devices/common.o devices/ds18x20.o

all: lsow digitemp_evil

lsow: $(OBJS1)
	gcc -O3 -o $@ $(OBJS1)

digitemp_evil: $(OBJS2)
	gcc -O3 -o $@ $(OBJS2)

%.o: %.c
	gcc -c -Wall -O3 -o $@ $<

clean:
	rm -f $(OBJS1) $(OBJS2) lsow digitemp_evil