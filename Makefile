.SUFFIXES: .o .c .h
.PHONY: all clean install

CFLAGS=-ansi -O -D_POSIX_SOURCE -fno-builtin -Wall
CC=gcc

PROG=ed
ALT=e

INSTALLD=$(HOME)/opt/$(PROG)

MANPAGE=\
  doc/ed.1

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
	$(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

clean:
	rm -f *.o $(PROG) 

install: all $(MANPAGE)
	rm -rf $(INSTALLD)
	mkdir -p $(INSTALLD)/bin $(INSTALLD)/man/man1
	cp $(PROG) $(INSTALLD)/bin/
	cp $(PROG) $(INSTALLD)/bin/$(ALT)
	cp $(MANPAGE) $(INSTALLD)/man/man1/
	cp $(MANPAGE) $(INSTALLD)/man/man1/$(ALT).1

