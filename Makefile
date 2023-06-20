CC=gcc

CCOPTS=--std=gnu99

OBJS=my_mmu.o

HEADERS=my_mmu.h

BINS=IL_TEST

all: $(BINS)

%.o: %.c $(HEADERS)

	$(CC) $(CCOPTS) -c -o $@ $<

IL_TEST: IL_TEST.c $(OBJS)

	$(CC) $(CCOPTS) -o $@ $^

clean: rm -rf *.o *~ $(BINS)