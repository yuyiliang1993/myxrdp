CFLAGS=-Wall -Werror
cc=gcc
targets=common
all:$(targets)
common:common.h common.c
	$(cc) $(CFLAGS) common.c -o $@
clean:
	rm -rf common
