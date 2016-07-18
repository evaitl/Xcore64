CC:=gcc
CFLAGS:=-Wall -O3 -Werror
LDFLAGS:=
LIBS:=-lelf

.PHONY: all clean
%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: read_pc foo

foo: foo.c
	$(CC) -O0 -o $@ $<

read_pc: read_pc.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	-rm -f read_pc.o read_pc
