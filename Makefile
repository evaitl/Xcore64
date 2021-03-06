CC:=gcc
CFLAGS:=-Wall -O3 -Werror
LDFLAGS:=
LIBS:=-lelf

.PHONY: all clean
%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: read_pc foo foo2

foo: foo.c
	$(CC) -O0 -o $@ $<

foo2: foo2.c
	$(CC) -O0 -o $@ $<

read_pc: read_pc.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f read_pc.o read_pc foo core foo2
