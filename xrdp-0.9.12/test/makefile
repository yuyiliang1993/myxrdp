CFLAGS=-Wall -Werror
cc=gcc
targets=common test_mq_write test_mq_recv test_write_fifo test_read_fifo
all:$(targets)
common:common.h common.c test_common.c
	$(cc) $(CFLAGS) common.c test_common.c -o $@
test_mq_write:common.h common.c mq.h mq.c test_mq_write.c
	$(cc) $(CFLAGS) common.c mq.c test_mq_write.c -o $@
test_mq_recv:common.h common.c mq.h mq.c test_mq_recv.c
	$(cc) $(CFLAGS) common.c mq.c test_mq_recv.c -o $@
test_write_fifo:writefifo.c common.c common.h
	$(cc) $(CFLAGS) common.c writefifo.c -o $@
test_read_fifo:readfifo.c common.c common.h
	$(cc) $(CFLAGS) common.c readfifo.c -o $@
clean:
	rm -rf common
