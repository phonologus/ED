/*
 * Editor
 */

#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>  /* for wait() */
#include <fcntl.h>  /* for open() and symbols for flags */

#include <unistd.h>
#include <stdlib.h>

#include "utfio.h"

#define nelem(X) (sizeof(X) / sizeof(X[0]))

enum {
  FNSIZE = 512,
  LBSIZE = 4096,
  BLKSIZE = 4096,
  BLKBSIZE = (BLKSIZE << 2),    /* # of bytes in a BLK */
  ESIZE = 256,
  GBSIZE = 256,
  NBRA = 10,
  KSIZE = 9
};

enum {
  CBRA = 1,
  CCHR = 2,
  CDOT = 4,
  CCL = 6,
  NCCL = 8,
  CDOL = 10,
  CEFIL = 11,
  CKET = 12,
  CBACK = 14,
  CCIRC = 15,
  STAR = 01
};

int FMODE;

int  Q[]  = {'\0'};
int  T[]  = {'T','M','P','\0'};
int  M[]  = {'M','E','M','?','\0'};
int  F[]  = {'F','I','L','E','\0'};
int  STDOUT[]= {'/','d','e','v','/','s','t','d','o','u','t','\0'};
int  NAPPENDED[]= {'?','N',' ','a','p','p','e','n','d','e','d','\0'};

int NL[] = {'\n','\0'};
int  A[]  = {'a','\0'};
int  R[]  = {'r','\0'};
int  SHRIEK[]= {'!','\0'};
int  QUERY[]= {'?','\0'};

enum {
  EOF = -1,
  READ = 0,
  WRITE = 1
};

enum {
  LINELEN = 70,    /* max number of glyphs in list-mode line */
  BELL = 4         /* max number of glyphs needed for \ddd */
};

int  line[LINELEN];
int  *linp  = line;

byte utf8buff[utfbsize];
int ucbuff[ucbsize];

utfio _uio;
utfio *uio=&_uio;

int  peekc;
int  lastc;
int  savedfile[FNSIZE];
int  file[FNSIZE];
int  linebuf[LBSIZE];
int  rhsbuf[LBSIZE/2];
int  expbuf[ESIZE+4];
int  genbuf[LBSIZE];
long  *zero;
long  *dot;
long  *dol;
int  given;
long  *addr1;
long  *addr2;
long  count;
int  *linebp;
int  io;
int  pflag;
void  (*oldhup)(int);
void  (*oldquit)(int);
int  vflag  = 1;
int  oflag;
int  listf;
int  listn;
int  col;
int  *globp;
int  tfile  = -1;
int  tline;
char  tfname[]="/tmp/exxxxxx";
int  *loc1;
int  *loc2;
int  ibuff[BLKSIZE];
int  iblock  = -1;
int  obuff[BLKSIZE];
int  oblock  = -1;
int  ichanged;
int  nleft;
long  names[26];
int  anymarks;
int  *braslist[NBRA];
int  *braelist[NBRA];
int  nbra;
int  subnewa;
int  subolda;
int  fchange;
int  wrapp;
int  bpagesize = 20;
unsigned int nlall = 128;
jmp_buf  savej;

long*  address(void);
int  getchr(void);
int*  getline(long);
int*  getblock(long, int);
int*  place(int*, int*, int*);
void  commands(void);
void  quit(void);
void  printcom(void);
void  bye(int);
void  error(int*);
void  nonzero(void);
void  newline(void);
void  rdelete(long*, long*);
void  setnoaddr(void);
void  init(void);
void  putst(int*);
void  putst(int*);
void  squeeze(int);
void  setwide(void);
void  putfile(void);
void  putd(void);
void  callunix(void);
void  dosub(void);
void  reverse(long*, long*);
void  compile(int);
void  putchr(int);
long  append(int(*)(void), long*);
void  add(int);
void  browse(void);
void  filename(int);
void  global(int);
void  join(void);
void  move(int);
void  exfile(void);
void  substitute(int);
int  getfile(void);
int  gettty(void);
int  compsub(void);
int  getsub(void);
int  getcopy(void);
int  getnum(void);
int  execute(long*);
int  putline(void);
int  advance(int*, int*);
int  cclass(int*, int, int);
int  backref(int, int*);

void  onintr(int);
void  onquit(int);
void  onhup(int);

int
main(int argc, char *argv[])
{
  int *p1, *p2;
  void (*oldintr)(int);

  oldquit = signal(SIGQUIT, SIG_IGN);
  oldhup = signal(SIGHUP, SIG_IGN);
  oldintr = signal(SIGINT, SIG_IGN);
  if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
    signal(SIGTERM, onquit);
  argv++;
  while (argc > 1 && **argv=='-') {
    switch((*argv)[1]) {

    case '\0':
      vflag = 0;
      break;

    case 'q':
      signal(SIGQUIT, SIG_DFL);
      vflag = 1;
      break;

    case 'o':
      oflag = 1;
      break;
    }
    argv++;
    argc--;
  }
  if (oflag) {
    p1 = STDOUT;
    p2 = savedfile;
    while ((*p2++ = *p1++))
      ;
    globp = A;
  } else if (*argv) {
    p1 = ucode((byte *)*argv);
    p2 = savedfile;
    while ((*p2++ = *p1++))
      if (p2 >= &savedfile[nelem(savedfile)])
        p2--;
    globp = R;
  }
  if((zero = (long*)malloc(nlall*sizeof(long)))==NULL) {
    error(M);
    onhup(0);
  }
  init();
  if (oldintr != SIG_IGN)
    signal(SIGINT, onintr);
  if (oldhup != SIG_IGN)
    signal(SIGHUP, onhup);
  setjmp(savej);
  commands();
  quit();
  return -1;  /* not reached */
}

void
commands(void)
{
  long *a1;
        int c, temp;
  int lastsep;

  for (;;) {
  if (pflag) {
    pflag = 0;
    addr1 = addr2 = dot;
    printcom();
  }
  c = '\n';
  for (addr1 = 0;;) {
    lastsep = c;
    a1 = address();
    c = getchr();
    if (c!=',' && c!=';')
      break;
    if (lastsep==',')
      error(Q);
    if (a1==0) {
      a1 = zero+1;
      if (a1>dol)
        a1--;
    }
    addr1 = a1;
    if (c==';')
      dot = a1;
  }
  if (lastsep!='\n' && a1==0)
    a1 = dol;
  if ((addr2=a1)==0) {
    given = 0;
    addr2 = dot;  
  }
  else
    given = 1;
  if (addr1==0)
    addr1 = addr2;
  switch(c) {

  case 'a':
    add(0);
    continue;

  case 'b':
    nonzero();
    browse();
    continue;

  case 'c':
    nonzero();
    newline();
    rdelete(addr1, addr2);
    append(gettty, addr1-1);
    continue;

  case 'd':
    nonzero();
    newline();
    rdelete(addr1, addr2);
    continue;

  case 'E':
    fchange = 0;
    c = 'e';
  case 'e':
    setnoaddr();
    if (vflag && fchange) {
      fchange = 0;
      error(Q);
    }
    filename(c);
    init();
    addr2 = zero;
    goto caseread;

  case 'f':
    setnoaddr();
    filename(c);
    putst(savedfile);
    continue;

  case 'g':
    global(1);
    continue;

  case 'i':
    add(-1);
    continue;


  case 'j':
    if (!given)
      addr2++;
    newline();
    join();
    continue;

  case 'k':
    nonzero();
    if ((c = getchr()) < 'a' || c > 'z')
      error(Q);
    newline();
    names[c-'a'] = unmark(*addr2);
    anymarks = mark(anymarks);
    continue;

  case 'm':
    move(0);
    continue;

  case 'n':
    listn++;
    newline();
    printcom();
    continue;

  case '\n':
    if (a1==0) {
      a1 = dot+1;
      addr2 = a1;
      addr1 = a1;
    }
    if (lastsep==';')
      addr1 = a1;
    printcom();
    continue;

  case 'l':
    listf++;
  case 'p':
  case 'P':
    newline();
    printcom();
    continue;

  case 'Q':
    fchange = 0;
  case 'q':
    setnoaddr();
    newline();
    quit();

  case 'r':
    filename(c);
  caseread:
    if ((io = open((char *)utf8(file), O_RDONLY)) < 0) {
      lastc = '\n';
      error(file);
    }
    setwide();
    squeeze(0);
    uioinitrd(io,uio);
    c = zero != dol;
    append(getfile, addr2);
    exfile();
    fchange = c;
    continue;

  case 's':
    nonzero();
    substitute(globp!=0);
    continue;

  case 't':
    move(1);
    continue;

  case 'u':
    nonzero();
    newline();
    if ((*addr2&~01) != subnewa)
      error(Q);
    *addr2 = subolda;
    dot = addr2;
    continue;

  case 'v':
    global(0);
    continue;

  case 'W':
    wrapp++;
  case 'w':
    setwide();
    squeeze(dol>zero);
    if ((temp = getchr()) != 'q' && temp != 'Q') {
      peekc = temp;
      temp = 0;
    }
    filename(c);
    FMODE= O_WRONLY | O_CREAT;
    if(wrapp)
      FMODE |= O_APPEND;
    else
      FMODE |= O_TRUNC;
    if((io=open((char*)utf8(file), FMODE, 0666)) == -1)
      error(file);
    uioinit(io,uio);
    wrapp = 0;
    if (dol > zero)
      putfile();
    exfile();
    if (addr1<=zero+1 && addr2==dol)
      fchange = 0;
    if (temp == 'Q')
      fchange = 0;
    if (temp)
      quit();
    continue;

  case '=':
    setwide();
    squeeze(0);
    newline();
    count = addr2 - zero;
    putd();
    putchr('\n');
    continue;

  case '!':
    callunix();
    continue;

  case EOF:
    return;

  }
  error(Q);
  }
}

void
printcom(void)
{
  long *a1;

  nonzero();
  a1 = addr1;
  do {
    if (listn) {
      count = a1-zero;
      putd();
      putchr('\t');
    }
    putst(getline(*a1++));
  } while (a1 <= addr2);
  dot = addr2;
  listf = 0;
  listn = 0;
  pflag = 0;
}

long*
address(void)
{
  int sign, opcnt, nextopand, c;
  long *a, *b;

  nextopand = -1;
  sign = 1;
  opcnt = 0;
  a = dot;
  do {
    do c = getchr(); while (c==' ' || c=='\t');
    if ('0'<=c && c<='9') {
      peekc = c;
      if (!opcnt)
        a = zero;
      a += sign*getnum();
    } else switch (c) {
    case '$':
      a = dol;
      /* fall through */
    case '.':
      if (opcnt)
        error(Q);
      break;
    case '\'':
      c = getchr();
      if (opcnt || c<'a' || 'z'<c)
        error(Q);
      a = zero;
      do a++; while (a<=dol && names[c-'a']!=(*a&~01));
      break;
    case '?':
      sign = -sign;
      /* fall through */
    case '/':
      compile(c);
      b = a;
      for (;;) {
        a += sign;
        if (a<=zero)
          a = dol;
        if (a>dol)
          a = zero;
        if (execute(a))
          break;
        if (a==b)
          error(Q);
      }
      break;
    default:
      if (nextopand == opcnt) {
        a += sign;
        if (a<zero || dol<a)
          continue;       /* error(Q); */
      }
      if (c!='+' && c!='-' && c!='^') {
        peekc = c;
        if (opcnt==0)
          a = 0;
        return a;
      }
      sign = 1;
      if (c!='+')
        sign = -sign;
      nextopand = ++opcnt;
      continue;
    }
    sign = 1;
    opcnt++;
  } while (zero<=a && a<=dol);
  error(Q);
  return 0;
}

int
getnum(void)
{
  int r, c;

  r = 0;
  while ((c=getchr())>='0' && c<='9')
    r = r*10 + c - '0';
  peekc = c;
  return r;
}

void
setwide(void)
{
  if (!given) {
    addr1 = zero + (dol>zero);
    addr2 = dol;
  }
}

void
setnoaddr(void)
{
  if (given)
    error(Q);
}

void
nonzero(void)
{
  squeeze(1);
}

void
squeeze(int i)
{
  if (addr1<zero+i || addr2>dol || addr1>addr2)
    error(Q);
}

void
newline(void)
{
  int c;

  if ((c = getchr()) == '\n' || c == EOF)
    return;
  if (c=='p' || c=='l' || c=='n') {
    pflag++;
    if (c=='l')
      listf++;
    else if (c=='n')
      listn++;
    if (getchr()=='\n')
      return;
  }
  error(Q);
}

void
filename(int comm)
{
  int *p1, *p2;
  int c;

  count = 0;
  c = getchr();
  if (c=='\n' || c==EOF) {
    p1 = savedfile;
    if (*p1==0 && comm!='f')
      error(Q);
    p2 = file;
    while ((*p2++ = *p1++))
      ;
    return;
  }
  if (c!=' ')
    error(Q);
  while ((c = getchr()) == ' ')
    ;
  if (c=='\n')
    error(Q);
  p1 = file;
  do {
    if (p1 >= &file[nelem(file)-1] || c==' ' || c==EOF)
      error(Q);
    *p1++ = c;
  } while ((c = getchr()) != '\n');
  *p1++ = 0;
  if (savedfile[0]==0 || comm=='e' || comm=='f') {
    p1 = savedfile;
    p2 = file;
    while ((*p1++ = *p2++))
      ;
  }
}

void
exfile(void)
{
  close(io);
  io = -1;
  if (vflag) {
    putd();
    putchr('\n');
  }
}

void
onintr(int sig)
{
  signal(SIGINT, onintr);
  putchr('\n');
  lastc = '\n';
  error(Q);
}

void
onhup(int sig)
{
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  if (dol > zero) {
    addr1 = zero+1;
    addr2 = dol;
    io = open("ed.hup", O_WRONLY | O_TRUNC | O_CREAT, 0600);
    if (io > 0) {
      uioinit(io,uio);
      putfile();
    }
  }
  fchange = 0;
  quit();
}

void
error(int *s)
{
  int c;

  wrapp = 0;
  listf = 0;
  listn = 0;
  count = 0;
  lseek(0, 0, SEEK_END);
  pflag = 0;
  if (globp)
    lastc = '\n';
  globp = 0;
  peekc = lastc;
  if(lastc)
    while ((c = getchr()) != '\n' && c != EOF)
      ;
  if (io > 0) {
    close(io);
    io = -1;
  }
  putchr('?');
  putst(s);
  longjmp(savej, 1);
}

int
getchr(void)
{
  byte b[utfbytes];
  byte *c;

  c=b;

  if ((lastc = peekc)) {
    peekc = 0;
    return lastc;
  }
  if (globp) {
    if ((lastc = *globp++) != 0)
      return lastc;
    globp = 0;
    return EOF;
  }

  if(read(0,c,1)<= 0)
    return lastc = EOF;

  if(convnutf(c,&lastc,1))
    return lastc;    /* unicode ascii */

  /*
   * lastc should now hold the number of extra bytes needed,
   * or 0 if there was a decoding error.
   */

  if(0==lastc)
    return lastc=utfeof;  /* decoding error */

  if(read(0,++c,lastc)<=0)  /* get required extra bytes */
    return lastc=utfeof;

  if(convutf(--c,&lastc))   /* convert to codepoint */
    return lastc;

  return lastc = utfeof;    /* decoding error */
}

int
gety(void)
{
  int c;
  int *gf;
  int *p;

  p = linebuf;
  gf = globp;
  while ((c = getchr()) != '\n') {
    if (c==EOF) {
      if (gf)
        peekc = c;
      return c;
    }
    if (c == 0)
      continue;
    *p++ = c;
    if (p >= &linebuf[LBSIZE-2])
      error(Q);
  }

  *p = 0;
  return 0;
}

int
gettty(void)
{
  int rc;

  if ((rc = gety()))
    return rc;
  if (linebuf[0]=='.' && linebuf[1]==0)
    return EOF;
  return 0;
}

int
getfile(void)
{
  int c;
  int *lp;

  lp = linebuf;
  do {
    if(uioread(uio,&c,1)<0)
      error(Q);
    if(c==utfeof) {
      if (lp>linebuf) {
        putst(NAPPENDED);
        c = '\n';
      } else
        return EOF;
    }
    if (c == '\0')
      continue;
    if (lp >= &linebuf[LBSIZE]) {
      lastc = '\n';
      error(Q);
    }
    *lp++ = c;
    count++;
  } while (c != '\n');
  *--lp = 0;
  return 0;
}

void
putfile(void)
{
  long *a1;
  int c;
  int *lp;

  a1 = addr1;
  do {
    lp = getline(*a1++);
    for (;;) {
      count++;
      if ((c = *lp++) == 0) {
        if(uiowrite(uio,NL,1)<0)
          error(Q);
        break;
      }
      if(uiowrite(uio,&c,1)<0)
        error(Q);
    }
  } while (a1 <= addr2);
  if(uioflush(uio)<0)  /* don't forget to flush! */
    error(Q);
}

long
append(int (*f)(void), long *a)
{
  long *a1, *a2, *rdot, *ozero, nline, tl;

  nline = 0;
  dot = a;
  while ((*f)() == 0) {
    if ((dol-zero)+1 >= nlall) {
      ozero = zero;
      nlall += 1024;
      if ((zero = (long *)realloc((void *)zero, nlall*sizeof(long)))==NULL) {
        error(M);
        onhup(0);
      }
      dot += zero - ozero;
      dol += zero - ozero;
    }
    tl = putline();
    nline++;
    a1 = ++dol;
    a2 = a1+1;
    rdot = ++dot;
    while (a1 > rdot)
      *--a2 = *--a1;
    *rdot = tl;
  }
  return nline;
}

void
add(int i)
{
  if (i && (given || dol>zero)) {
    addr1--;
    addr2--;
  }
  squeeze(0);
  newline();
  append(gettty, addr2);
}

void
browse(void)
{
  int forward, n;
  static int bformat, bnum; /* 0 */

  forward = 1;
  if((peekc=getchr()) != '\n'){
    if (peekc=='-' || peekc=='+') {
      if(peekc == '-')
        forward=0;
      getchr();
    }
    if((n=getnum()) > 0)
      bpagesize = n;
  }
  newline();
  if (pflag) {
    bformat = listf;
    bnum = listn;
  } else {
    listf = bformat;
    listn = bnum;
  }
  if (forward) {
    addr1 = addr2;
    if((addr2+=bpagesize) > dol)
      addr2 = dol;
  } else {
    if((addr1=addr2-bpagesize) <= zero)
      addr1 = zero+1;
  }
  printcom();
}

void
callunix(void)
{
  pid_t pid, rpid;
  void (*savint)(int);
  int retcode;

  setnoaddr();
  if ((pid = fork()) == 0) {
    signal(SIGHUP, oldhup);
    signal(SIGQUIT, oldquit);
    execl("/bin/sh", "sh", "-t", (char *)NULL);
    bye(0100);
  }
  savint = signal(SIGINT, SIG_IGN);
  while ((rpid = wait(&retcode)) != pid && rpid != -1)
    ;
  signal(SIGINT, savint);
  if (vflag) {
    putst(SHRIEK);
  }
}

void
quit(void)
{
  if (vflag && fchange && dol!=zero) {
    fchange = 0;
    error(Q);
  }
  bye(0);
}

void
bye(int e)
{
  unlink(tfname);
  free(zero);
  exit(e);
}

void
onquit(int sig)
{
  quit();
}

void
rdelete(long *ad1, long *ad2)
{
  long *a1, *a2, *a3;

  a1 = ad1;
  a2 = ad2+1;
  a3 = dol;
  dol -= a2 - a1;
  do {
    *a1++ = *a2++;
  } while (a2 <= a3);
  a1 = ad1;
  if (a1 > dol)
    a1 = dol;
  dot = a1;
  fchange = 1;
}

void
gdelete(void)
{
  long *a1, *a2, *a3;

  a3 = dol;
  for (a1=zero; (*a1&01)==0; a1++)
    if (a1>=a3)
      return;
  for (a2=a1+1; a2<=a3;) {
    if (*a2&01) {
      a2++;
      dot = a1;
    } else
      *a1++ = *a2++;
  }
  dol = a1-1;
  if (dot>dol)
    dot = dol;
  fchange = 1;
}

int*
getline(long tl)
{
  int *bp;
  int *lp;
  long nl;

  lp = linebuf;
  bp = getblock(tl, READ);
  nl = nleft;
  tl &= ~((BLKSIZE/2)-1);
  while ((*lp++ = *bp++))
    if (--nl == 0) {
      bp = getblock(tl+=(BLKSIZE/2), READ);
      nl = nleft;
    }
  return linebuf;
}

int
putline(void)
{
  int *bp;
  int *lp;
  long nl, tl;

  fchange = 1;
  lp = linebuf;
  tl = tline;
  bp = getblock(tl, WRITE);
  nl = nleft;
  tl &= ~((BLKSIZE/2)-1);
  while ((*bp = *lp++)) {
    if (*bp++ == '\n') {
      *--bp = 0;
      linebp = lp;
      break;
    }
    if (--nl == 0) {
      bp = getblock(tl+=(BLKSIZE/2), WRITE);
      nl = nleft;
    }
  }
  nl = tline;
  tline += (((lp-linebuf)+03)>>1)&~01;
  return nl;
}

void
blkio(int b, int *buf, int rw)
{
  if(lseek(tfile, (long)b*(long)BLKBSIZE, SEEK_SET)<0L)
    error(T);
  if(rw == READ) {
    if (read(tfile, buf, BLKBSIZE) != BLKBSIZE)
      error(T);
  } else if(rw == WRITE) {
    if (write(tfile, buf, BLKBSIZE) != BLKBSIZE)
      error(T);
  } else error(T);
     
}

int *
getblock(long atl, int iof)
{
  int bno, off;
  
  bno = (atl/(BLKSIZE/2));
  off = (atl<<1) & (BLKSIZE-1) & ~03;
  nleft = BLKSIZE - off;
  if (bno==iblock) {
    ichanged |= iof;
    return ibuff+off;
  }
  if (bno==oblock)
    return obuff+off;
  if (iof==READ) {
    if (ichanged)
      blkio(iblock, ibuff, WRITE);
    ichanged = 0;
    iblock = bno;
    blkio(bno, ibuff, READ);
    return ibuff+off;
  }
  if (oblock>=0)
    blkio(oblock, obuff, WRITE);
  oblock = bno;
  return obuff+off;
}

void
init(void)
{
  long *markp;
  char *p;
  int pid;

  close(tfile);

  pid=getpid();
  for(p=&tfname[12];p>&tfname[6];){
    *--p=(pid%10)+'0';
    pid/=10;
  }
  tline = 2;
  for (markp = names; markp < &names[26]; )
    *markp++ = 0;
  subnewa = 0;
  anymarks = 0;
  iblock = -1;
  oblock = -1;
  ichanged = 0;
  if((tfile = open(tfname, O_RDWR | O_TRUNC | O_CREAT,0600)) == -1)
    error(T);
  dot = dol = zero;
}

void
global(int k)
{
  int *gp, globuf[GBSIZE];
  int c;
  long *a1;

  if (globp)
    error(Q);
  setwide();
  squeeze(dol>zero);
  if ((c=getchr())=='\n')
    error(Q);
  compile(c);
  gp = globuf;
  while ((c = getchr()) != '\n') {
    if (c==EOF)
      error(Q);
    if (c=='\\') {
      c = getchr();
      if (c!='\n')
        *gp++ = '\\';
    }
    *gp++ = c;
    if (gp >= &globuf[GBSIZE-2])
      error(Q);
  }
  if (gp == globuf)
    *gp++ = 'p';
  *gp++ = '\n';
  *gp = 0;
  for (a1=zero; a1<=dol; a1++) {
    *a1 = unmark(*a1);
    if (a1>=addr1 && a1<=addr2 && execute(a1)==k)
      *a1 = mark(*a1);
  }
  /*
   * Special case: g/.../d (avoid n^2 algorithm)
   */
  if (globuf[0]=='d' && globuf[1]=='\n' && globuf[2]=='\0') {
    gdelete();
    return;
  }
  for (a1=zero; a1<=dol; a1++) {
    if (marked(*a1)) {
      *a1 = unmark(*a1);
      dot = a1;
      globp = globuf;
      commands();
      a1 = zero;
    }
  }
}

void
join(void)
{
  int *gp, *lp;
  long *a1;

  nonzero();
  gp = genbuf;
  for (a1=addr1; a1<=addr2; a1++) {
    lp = getline(*a1);
    while ((*gp = *lp++))
      if (gp++ >= &genbuf[LBSIZE-2])
        error(Q);
  }
  lp = linebuf;
  gp = genbuf;
  while ((*lp++ = *gp++))
    ;
  *addr1 = putline();
  if (addr1 < addr2)
    rdelete(addr1+1, addr2);
  dot = addr1;
}

void
substitute(int inglob)
{
  long *mp, *a1, nl, *ozero; 
  int gsubf, m, n;

  n = getnum();  /* OK even if n==0 */
  gsubf = compsub();
  for (a1 = addr1; a1 <= addr2; a1++) {
    if (execute(a1)){
      m = n;
      do {
        int span = loc2-loc1;

        if (--m <= 0) {
          dosub();
          if (!gsubf)
            break;
          if (span == 0) {  /* null RE match */
            if (*loc2=='\0')
              break;
            loc2++;
          }
        }
      } while (execute((long*)0));
      if (m <= 0) {
        inglob = mark(inglob);
        subnewa = putline();
        *a1 = unmark(*a1);
        if (anymarks) {
          for (mp = names; mp < &names[26]; mp++)
            if (*mp == *a1)
              *mp = subnewa;
        }
        subolda = *a1;
        *a1 = subnewa;
        ozero = zero;
        nl = append(getsub, a1);
        nl += zero-ozero;
        a1 += nl;
        addr2 += nl;
      }
    }
  }
  if (inglob==0)
    error(Q);
}

int
compsub(void)
{
  int seof, c;
  int *p;

  if ((seof = getchr()) == '\n' || seof == ' ')
    error(Q);
  compile(seof);
  p = rhsbuf;
  for (;;) {
    c = getchr();
    if (c=='\\')
      c = escape(getchr());
    if (c=='\n') {
      if (globp && globp[0])  /* last '\n' does not count */
        c = escape(c);
      else {
        peekc = c;
        pflag++;
        break;
      }
    }
    if (c == seof)
      break;
    *p++ = c;
    if (p >= &rhsbuf[LBSIZE/2])
      error(Q);
  }
  *p = 0;
  if ((peekc = getchr()) == 'g') {
    peekc = 0;
    newline();
    return 1;
  }
  newline();
  return 0;
}

int
getsub(void)
{
  int *p1, *p2;

  p1 = linebuf;
  if ((p2 = linebp) == 0)
    return EOF;
  while ((*p1++ = *p2++))
    ;
  linebp = 0;
  return 0;
}

void
dosub(void)
{
  int *lp, *sp, *rp;
  int c;

  lp = linebuf;
  sp = genbuf;
  rp = rhsbuf;
  while (lp < loc1)
    *sp++ = *lp++;
  while ((c = *rp++)) {
    if (c=='&') {
      sp = place(sp, loc1, loc2);
      continue;
    } else if (escaped(c) && (c=unescape(c)) >='1' &&  c < nbra+'1') {
      sp = place(sp, braslist[c-'1'], braelist[c-'1']);
      continue;
    }
    *sp++ = c;
    if (sp >= &genbuf[LBSIZE])
      error(Q);
  }
  lp = loc2;
  loc2 = sp - genbuf + linebuf;
  while ((*sp++ = *lp++))
    if (sp >= &genbuf[LBSIZE])
      error(Q);
  lp = linebuf;
  sp = genbuf;
  while ((*lp++ = *sp++))
    ;
}

int*
place(int *sp, int *l1, int *l2)
{

  while (l1 < l2) {
    *sp++ = *l1++;
    if (sp >= &genbuf[LBSIZE])
      error(Q);
  }
  return sp;
}

void
move(int cflag)
{
  long *adt, *ad1, *ad2, *ozero, delta;

  nonzero();
  if ((adt = address())==0)  /* address() guarantees addr is in range */
    error(Q);
  newline();
  if (cflag) {
    ad1 = dol;
    ozero = zero;
    append(getcopy, ad1++);
    ad2 = dol;
    delta = zero - ozero;
    ad1 += delta;
    adt += delta;
  } else {
    ad2 = addr2;
    for (ad1 = addr1; ad1 <= ad2;) {
      *ad1=unmark(*ad1);
      ad1++;
    }
    ad1 = addr1;
  }
  ad2++;
  if (adt<ad1) {
    dot = adt + (ad2-ad1);
    if ((++adt)==ad1)
      return;
    reverse(adt, ad1);
    reverse(ad1, ad2);
    reverse(adt, ad2);
  } else if (adt >= ad2) {
    dot = adt++;
    reverse(ad1, ad2);
    reverse(ad2, adt);
    reverse(ad1, adt);
  } else
    error(Q);
  fchange = 1;
}

void
reverse(long *a1, long *a2)
{
  long t;

  for (;;) {
    t = *--a2;
    if (a2 <= a1)
      return;
    *a2 = *a1;
    *a1++ = t;
  }
}

int
getcopy(void)
{
  if (addr1 > addr2)
    return EOF;
  getline(*addr1++);
  return 0;
}

void
compile(int eof)
{
  int c, cclcnt;
  int *ep, *lastep, bracket[NBRA], *bracketp;

  ep = expbuf;
  bracketp = bracket;
  if ((c = getchr()) == '\n') {
    peekc = c;
    c = eof;
  }
  if (c == eof) {
    if (*ep==0)
      error(Q);
    return;
  }
  nbra = 0;
  if (c=='^') {
    c = getchr();
    *ep++ = CCIRC;
  }
  peekc = c;
  lastep = 0;
  for (;;) {
    if (ep >= &expbuf[ESIZE])
      goto cerror;
    c = getchr();
    if (c == '\n') {
      peekc = c;
      c = eof;
    }
    if (c==eof) {
      if (bracketp != bracket)
        goto cerror;
      *ep = CEFIL;
      return;
    }
    if (c!='*')
      lastep = ep;
    switch (c) {

    case '\\':
      if ((c = getchr())=='(') {
        if (nbra >= NBRA)
          goto cerror;
        *bracketp++ = nbra;
        *ep++ = CBRA;
        *ep++ = nbra++;
        continue;
      }
      if (c == ')') {
        if (bracketp <= bracket)
          goto cerror;
        *ep++ = CKET;
        *ep++ = *--bracketp;
        continue;
      }
      if (c>='1' && c<'1'+NBRA) {
        *ep++ = CBACK;
        *ep++ = c-'1';
        continue;
      }
      *ep++ = CCHR;
      if (c=='\n')
        goto cerror;
      *ep++ = c;
      continue;

    case '.':
      *ep++ = CDOT;
      continue;

    case '\n':
      goto cerror;

    case '*':
      if (lastep==0 || *lastep==CBRA || *lastep==CKET)
        goto defchar;
      *lastep |= STAR;
      continue;

    case '$':
      if ((peekc=getchr()) != eof && peekc!='\n')
        goto defchar;
      *ep++ = CDOL;
      continue;

    case '[':
      *ep++ = CCL;
      *ep++ = 0;
      cclcnt = 1;
      if ((c=getchr()) == '^') {
        c = getchr();
        ep[-2] = NCCL;
      }
      do {
        if (c=='\n')
          goto cerror;
        if (c=='-' && ep[-1]!=0) {
          if ((c=getchr())==']') {
            *ep++ = '-';
            cclcnt++;
            if (ep >= &expbuf[ESIZE])
              goto cerror;
            *ep++ = '-';
            cclcnt++;
            if (ep >= &expbuf[ESIZE])
              goto cerror;
            break;
          }
          ep[-1]=c;
          continue;
        }
        *ep++ = c;
        cclcnt++;
        if (ep >= &expbuf[ESIZE])
          goto cerror;
        *ep++ = c;
        cclcnt++;
        if (ep >= &expbuf[ESIZE])
          goto cerror;
      } while ((c = getchr()) != ']');
      lastep[1] = cclcnt;
      continue;

    defchar:
    default:
      *ep++ = CCHR;
      *ep++ = c;
    }
  }
   cerror:
  expbuf[0] = 0;
  nbra = 0;
  error(Q);
}

int
execute(long *addr)
{
  int *p1, *p2;
  int c;

  for (c=0; c<NBRA; c++) {
    braslist[c] = 0;
    braelist[c] = 0;
  }
  p2 = expbuf;
  if (addr == (long*)0) {
    if (*p2 == CCIRC)
      return 0;
    p1 = loc2;
  } else {
    if (addr == zero)
      return 0;
    p1 = getline(*addr);
  }
  if (*p2 == CCIRC) {
    loc1 = p1;
    return advance(p1, p2+1);
  }
  /* fast check for first character */
  if (*p2 == CCHR) {
    c = p2[1];
    do {
      if (*p1!=c)
        continue;
      if (advance(p1, p2)) {
        loc1 = p1;
        return 1;
      }
    } while (*p1++);
    return 0;
  }
  /* regular algorithm */
  do {
    if (advance(p1, p2)) {
      loc1 = p1;
      return 1;
    }
  } while (*p1++);
  return 0;
}

int
advance(int *lp, int *ep)
{
  int *curlp;
  int i;

  for (;;)
  switch (*ep++) {

  case CCHR:
    if (*ep++ == *lp++)
      continue;
    return 0;

  case CDOT:
    if (*lp++)
      continue;
    return 0;

  case CDOL:
    if (*lp==0)
      continue;
    return 0;

  case CEFIL:
    loc2 = lp;
    return 1;

  case CCL:
    if (cclass(ep, *lp++, 1)) {
      ep += *ep;
      continue;
    }
    return 0;

  case NCCL:
    if (cclass(ep, *lp++, 0)) {
      ep += *ep;
      continue;
    }
    return 0;

  case CBRA:
    braslist[*ep++] = lp;
    continue;

  case CKET:
    braelist[*ep++] = lp;
    continue;

  case CBACK:
    if (braelist[i = *ep++]==0)
      error(Q);
    if (backref(i, lp)) {
      lp += braelist[i] - braslist[i];
      continue;
    }
    return 0;

  case CBACK|STAR:
    if (braelist[i = *ep++] == 0)
      error(Q);
    curlp = lp;
    while (backref(i, lp))
      lp += braelist[i] - braslist[i];
    while (lp >= curlp) {
      if (advance(lp, ep))
        return 1;
      lp -= braelist[i] - braslist[i];
    }
    continue;

  case CDOT|STAR:
    curlp = lp;
    while (*lp++)
      ;
    goto star;

  case CCHR|STAR:
    curlp = lp;
    while (*lp++ == *ep)
      ;
    ep++;
    goto star;

  case CCL|STAR:
  case NCCL|STAR:
    curlp = lp;
    while (cclass(ep, *lp++, ep[-1]==(CCL|STAR)))
      ;
    ep += *ep;
    goto star;

  star:
    do {
      lp--;
      if (advance(lp, ep))
        return 1;
    } while (lp > curlp);
    return 0;

  default:
    error(Q);
  }
}

int
backref(int i, int *lp)
{
  int *bp;

  bp = braslist[i];
  while (*bp++ == *lp++)
    if (bp >= braelist[i])
      return 1;
  return 0;
}

int
cclass(int *set, int c, int af)
{
  int n;

  if (c==0)
    return 0;
  n = *set++;
  while (--n) {
    if (c >= *set++) {
      if(c <= *set++)
        return af;
    } else
      ++set;
  }
  return !af;
}

void
putd(void)
{
  int r;

  r = count%10;
  count /= 10;
  if (count)
    putd();
  putchr(r + '0');
}

void
putst(int *sp)
{
  col = 0;
  while (*sp)
    putchr(*sp++);
  putchr('\n');
}

void
putchr(int c)
{
  int *lp;
  byte utf[utfbytes+1];
  byte a, *b;
  int n;

  lp = linp;
  if (listf) {
    if (c=='\n') {
      if (linp!=line && linp[-1]==' ') {
        *lp++ = '\\';
        *lp++ = 'n';
      }
      *lp++ = c;
    } else {
      if (col >= (LINELEN-BELL)) {
        col = 8;
        *lp++ = '\\';
        *lp++ = '\n';
        *lp++ = '\t';
      }
      col++;
      if (c=='\b' || c=='\t' || c=='\\') {
        *lp++ = '\\';
        if (c=='\b')
          c = 'b';
        else if (c=='\t')
          c = 't';
        *lp++ = c;
        col++;
      } else if (c<' ' || c>='\177') {
        convnucode(c,utf,utfbytes);
        b=utf;
        while((a=*b++)) {
          *lp++ = '\\';
          *lp++ = ((a>>6)&03) + '0';
          *lp++ = ((a>>3)&07) + '0';
          *lp++ = (a & 07) + '0';
          col += 4;
        }
      } else
        *lp++ = c;
    }
  } else
    *lp++ = c;
  if(c == '\n' || lp >= &line[LINELEN-BELL]) {
    linp=line;
    n=utf8nstring(line,utf8buff,lp-line);
    write(oflag?2:1, utf8buff, n);
    return;
  }
  linp = lp;
}
