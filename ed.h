/*
 * utf and utfio
 */

#ifndef UTF4ED
#define UTF4ED

enum {
  utfbytes=4,
  utfeof=-1,
  ucbsize=512,
  utfbsize=utfbytes*ucbsize
};

typedef unsigned char byte;

extern byte utf8buff[utfbsize];   /* general purpose buffer for utf8 sequences */
extern int ucbuff[ucbsize];   /* general purpose buffer for unicode sequences */

int convnutf(byte *, int *, int);
int convnucode(int, byte *, int);
int utf8nstring(int*, byte*, unsigned int);
int ucodenstring(byte *, int *, unsigned int);

#define utf8(u) (utf8nstring(u,utf8buff,utfbsize),utf8buff)
#define ucode(b) (ucodenstring(b,ucbuff,ucbsize),ucbuff)

#define convutf(p,z) convnutf(p,z,utfbytes)
#define convucode(p,z) convnucode(p,z,utfbytes)

enum {
  UBSIZE=4096
};

struct utfio_s {
  int fd;          /* attached file */
  byte b[UBSIZE];  /* raw byte buffer */
  int i;           /* index of next unused byte, or UBSIZE if empty */
  int z;           /* index of EOF, or UBSIZE if EOF not yet in buffer */
};

typedef struct utfio_s utfio;

utfio *uioinit(int, utfio *);
utfio *uioinitrd(int, utfio *);
int uiofill(utfio *);
int uioflush(utfio *);
int uioread(utfio *,int *,int);
int uiowrite(utfio *,int *,int);

/*
 * unicode string literals
 */

extern int utfstr_nl[];
extern int utfstr_nul[];

extern int utfstr_edhup[];

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

/* types 
 *
 * addr_t : the address cookie
 * addr_i : a pointer/index to an addr_t
 *
 */

typedef int addr_t; 
typedef int addr_i; 

typedef unsigned char byte;

#endif
