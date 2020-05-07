#ifndef UTFH
#define UTFH

/*
 * utf and utfio
 */

typedef unsigned char byte;

enum {
  utfbytes=4,
  utfeof=-1,
  ucbsize=512,
  utfbsize=utfbytes*ucbsize
};

extern byte utf8buff[utfbsize];   /* general purpose buffer for utf8 sequences */
extern int ucbuff[ucbsize];   /* general purpose buffer for unicode sequences */

int convnutf(byte *, int *, int);
int convnucode(int, byte *, int);
int utf8nstring(int*, byte*, unsigned int);
int ucodenstring(byte *, int*, unsigned int);

#define utf8(u) (utf8nstring(u,utf8buff,utfbsize),utf8buff)
#define ucode(b) (ucodenstring(b,ucbuff,ucbsize),ucbuff)

#define convutf(p,z) convnutf(p,z,utfbytes)
#define convucode(p,z) convnucode(p,z,utfbytes)

/*
 * escaped chars
 *
 */

enum {
  ESC=(1 << ((8*utfbytes)-1)),    /* 0x80000000 */
  UNESC=~ESC,
  MARK=1,
  UNMARK=~MARK
};

#define escape(c) ((c)|ESC)
#define escaped(c) ((c)&ESC)
#define unescape(c) ((c)&UNESC)

#define mark(c) ((c)|MARK)
#define marked(c) ((c)&MARK)
#define unmark(c) ((c)&UNMARK)

#endif
