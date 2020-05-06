CFLAGS=-ansi -I. -O -D_POSIX_SOURCE -fno-builtin
CC=gcc

TAR=9 tar
COMPRESS=9 compress

PROG=ed
PROG1=e

INSTALLD=$HOME/opt/$PROG

HDRS=\
  ansi.h \
  posix.h

MODULES=\
  ed

DOCS=\
  doc/ed.1\

ARCHIVE=\
  mkfile\
  $DOCS\
  ${MODULES:%=%.c}\
  $HDRS

OBJS=${MODULES:%=%.o}

%.o:  %.c
  $CC -c $CFLAGS $prereq

%.pdf:  %
	9 troff -man $stem | tr2post | psfonts | ps2pdf - > $target

all:V: a.out

a.out:	$OBJS 
	$CC $prereq

clean:V:
	rm -f *.o a.out doc/*.pdf

doc:V:  doc/$PROG.1.pdf

install:V: all doc
	rm -rf $INSTALLD
	mkdir -p $INSTALLD/bin $INSTALLD/man/man1 $INSTALLD/doc
	cp a.out $INSTALLD/bin/$PROG
	cp a.out $INSTALLD/bin/$PROG1
	cp doc/$PROG.1 $INSTALLD/man/man1/$PROG.1
	cp doc/$PROG.1 $INSTALLD/man/man1/$PROG1.1
	cp doc/$PROG.1.pdf $INSTALLD/doc/


archive:V: $ARCHIVE
	$TAR c $prereq | $COMPRESS > $PROG.tar.Z

unarchive:V: $PROG.tar.Z
	$TAR xzvf $prereq

