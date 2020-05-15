.SUFFIXES: .o .c .h
.PHONY: all clean

CFLAGS=-ansi -I. -O -D_POSIX_SOURCE -fno-builtin
CC=gcc

PROG=ed

HDRS=\
  utf.h \
  utfio.h

MODULES=\
  utf \
  utfio \
  ed

OBJS=$(MODULES:=.o)

.c.o:
	$(CC) -c $(CFLAGS) $<

all: $(PROG)

$(PROG): $(OBJS)  $(HDRS)
	$(CC) -o $(PROG) $(OBJS)

clean:
	rm -f *.o $(PROG) 

