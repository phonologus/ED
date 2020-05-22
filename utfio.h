#ifndef UTFIOH
#define UTFIOH

/*
 * utfio
 */

#include <unistd.h>	/* read(), write() */

#include "utf.h"

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

#endif
