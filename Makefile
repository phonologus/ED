.SUFFIXES: .o .c .h
.PHONY: all clean install uninstall

CFLAGS=-std=c89 -O -Wall
CC=cc
INSTALL=install

PROG=ed
ALT=e

INSTALLD=/usr/local

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
	$(INSTALL) -d $(INSTALLD)/bin
	$(INSTALL) -d $(INSTALLD)/share/man/man1
	$(INSTALL) -s $(PROG) $(INSTALLD)/bin/
	$(INSTALL) -s $(PROG) $(INSTALLD)/bin/$(ALT)
	$(INSTALL) $(MANPAGE) $(INSTALLD)/share/man/man1/
	$(INSTALL) $(MANPAGE) $(INSTALLD)/share/man/man1/$(ALT).1

uninstall:
	rm -f $(INSTALLD)/bin/ed
	rm -f $(INSTALLD)/bin/$(ALT)
	rm -f $(INSTALLD)/share/man/man1/ed.1
	rm -f $(INSTALLD)/share/man/man1/$(ALT).1

