/*
** srt.c -- sort text lines
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define SHELL 1
#define QUICK 2
#define WRTMODE 2
#define MAXRUNS 99
#define LOGPTR 20
#define AVGLIN 28
#define RESERVE 2000
#define MERGEORDER 5
char *linbuf, outnam[MAXFN], tmpdrv;
char lastline[MAXLINE+1];
char *maxbuf, *maxlin;  /**** fake unsigned int ****/
char tmpout[]="X:sort00.$$$";
char tmpinp[]="X:sort00.$$$";
char tmpdel[]="X:sort00.$$$";
char delim;
int  field;
int tmpfd[MERGEORDER], *linptr, nlines;
int low, lim, high, outfil, output, t, order, unique, typesort;
main(argc, argv) int argc, *argv; {
  lastline[0]=outnam[0]=0;
  tmpdrv='X';
  doargs(argc, argv);
  if(tmpdrv == 'X') {
    strcpy(tmpout, tmpout+2);
    strcpy(tmpinp, tmpinp+2);
    strcpy(tmpdel, tmpdel+2);
    }
  else tmpout[0]=tmpinp[0]=tmpdel[0]=tmpdrv;
  output=stdout;
  if((lim=avail(YES))<0) lim=32767;
  maxlin=(lim-RESERVE)/(2+AVGLIN);
  linptr=malloc(2*maxlin);
  if((lim=avail(YES))<0) lim=32767;
  maxbuf=lim - RESERVE;
  linbuf=malloc(maxbuf);

  high=0;
  while(YES) {
    if(++high >= MAXRUNS) {
      fputs("file too large\n", stderr);
      abort(7);
      }
    t=gtext();

    sort(0, nlines-1);

    if(high==1) {
      if(t==NULL) {
        outfil=output;
        ptext();
        fclose(outfil);
        exit(0);
        }
      }
    maketmp();
    ptext();
    fclose(outfil);
    if(t==NULL) break;
    }


/*
** Must deallocate in reverse order from allocation.
** Will allocate input tmp file buffers/FCBs over this space;
** these must not reach end of linbuf where output tmp file
** space was allocated, since that space stays with that fd.
*/
  free(linbuf);
  free(linptr);

  linptr=malloc(2*(MERGEORDER+1));
  linbuf=malloc(MERGEORDER*(MAXLINE+1));
  lastline[0]=0;
  low=1;
  while(low < high) {               /*05*/
    lim=low+MERGEORDER-1;
    if(high < lim) lim=high;
    t=0;
    while(t <= (lim-low)) {
      bumptmp(tmpinp);
      if((tmpfd[t]=fopen(tmpinp, "r"))==NULL) cant(tmpinp);
      auxbuf(tmpfd[t++], 2048); /* redundant calls ignored */
      }
    if(lim==high) outfil=output;
    else maketmp();
    if(++high >= MAXRUNS) {
      fputs("file too large\n", stderr);
      abort(7);
      }
    merge(lim-low+1);
    fclose(outfil);
    t=0;
    while(t <= (lim-low)) {
      fclose(tmpfd[t++]);           /*02*/
      killtmp();
      }
    low=low+MERGEORDER;
    }
  }

doargs(argc, argv)  int argc, *argv;  {
  char arg[MAXFN], c;
  int i, error, len;
  field=0;
  delim=NULL;    /** indicates column number in field **/
  order=1;
  typesort=SHELL;
  unique=error=NO;
  i=0;
  while(getarg(++i, arg, MAXFN, argc, argv)!=EOF) {
    c=arg[1];
    if(arg[0]!='-') error=YES;
    else if(same(c, 't') &&
           (toupper(arg[2]) > 'A') &&
           (toupper(arg[2]) < 'G') &&
           (arg[3]==NULL))
           tmpdrv=arg[2];
    else if(same(c, 'c')) {
      delim=NULL;
      if(arg[utoi(arg+2, &field)+2] != NULL) error=YES;
      if(field) --field;
      }
    else if(same(c, 'f')) {
      if(arg[(len=utoi(arg+2, &field))+2] > ' ') {
        delim=arg[len+2];
        if(arg[len+3] != NULL) error=YES;
        }
      else delim=' ';
      if(field) --field;
      field = -field;
      }
    else if(arg[2]!=NULL) error=YES;
    else if(same(c, 'd')) order=-1;
    else if(same(c, 'u')) unique=YES;
    else if(same(c, 'q')) typesort=QUICK;
    else error=YES;
    if(error) {
      fputs("usage: SRT [-C#|-F#?] [-D] [-U] [-Tx] [-Q]\n",
             stderr);
      abort(7);
      }
    }
  }

gtext() {
  int len;
  char *lbp;
  lbp=1; /** leave space for first sort key offset **/
  nlines=0;
  while(YES) {
    poll(YES);
    if((len=readline(linbuf+lbp, stdin))==NULL) break;
    linptr[nlines++]=lbp;
    lbp=lbp+len;  /** has 2 bytes for NULL and next offset **/
    if(((lbp+1) >= (maxbuf-(MAXLINE+1)))||(nlines >= maxlin))
      break;
    }
  return len;
  }

ptext() {
  int i;
  char *lbp;
  i=0;
  while(i < nlines) {
    poll(YES);
    lbp=linbuf+linptr[i++];
    if(duptest(lbp)) continue;
    sout(lbp, outfil);
    }
  }

duptest(line) char *line; {
  int diff;
  if(!unique) return (NO);           /*03*/
  diff = lexcmp(lastline, line);
  strcpy(lastline, line);
  return (!diff);
  }

bumptmp(tmpname) char tmpname[]; {
  char *digit;
  digit = strchr(tmpname, '.') - 1;
  if(*digit == '9') {*digit = '0'; --digit;}
  ++*digit;
  }

maketmp() {
  bumptmp(tmpout);
  if((outfil=fopen(tmpout,"w"))==NULL) cant(tmpout);
  }

killtmp() {
  bumptmp(tmpdel);
  unlink(tmpdel);
  }

sort(lv, uv) int lv, uv; {
  if(typesort==QUICK) quick(lv, uv);
  else                shell(lv, uv);
  }

shell(lv, uv) int lv, uv; {
  int gap, i, j, jg;
  gap = (uv-lv+1) >> 1; /** divide by 2 **/
  while(gap > 0) {
    poll(YES);
    i = gap + lv;
    while(i <= uv) {
      j = i++ - gap;
      while(j >= lv) {
        jg = j + gap;
        if(compare(linptr[j], linptr[jg]) <= 0) break;
        exchange(j, jg);
        j = j - gap;
        }
      }
    gap = gap>>1; /** divide by 2 **/
    }
  }

quick(lv, uv) int lv, uv; {
  int i, j, pivlin;
  avail(YES);
  poll(YES);
  if(lv >= uv) return;  /** only one element **/
  i=lv-1;
  j=uv;
  pivlin=linptr[j];
  while(i < j) {
    ++i;
    while(compare(linptr[i], pivlin) < 0) ++i;
    --j;
    while(i < j) {
      if(compare(linptr[j], pivlin) > 0) --j;
      else break;
      }
    if(i < j) exchange(i, j);
    }
  j=uv;
  exchange(i, j);
  if((i-lv) < (uv-i)) {
    quick(lv, i-1);
    quick(i+1, uv);
    }
  else {
    quick(i+1, uv);
    quick(lv, i-1);
    }
  }

compare(p1, p2) int p1, p2; {
  char *ptr1, *ptr2;
  ptr1 = linbuf + (p1 - 1); ptr1 = ptr1 + *ptr1;
  ptr2 = linbuf + (p2 - 1); ptr2 = ptr2 + *ptr2;
  while(lexorder(*++ptr1, *++ptr2) == 0)
    if((*ptr1 == NULL)||(delimit(*ptr1))) return 0;
  if(delimit(*ptr1)) return -order;
  if(delimit(*ptr2)) return  order;
  if(lexorder(*ptr1, *ptr2) > 0) return order;
  return -order;
  }

delimit(c) char c; {
  if(c > delim)    return NO;
  if(delim == ' ') return YES;
  if(c < delim)    return NO;
  return YES;
  }

exchange(i, j) int i, j; {
  int k;
  k=linptr[i]; linptr[i]=linptr[j]; linptr[j]=k;
  }

merge(nfiles) int nfiles; {
  int i, inf, lbp, lp1, nf;
  char *ptr;
  lbp=1; /* leave space for first sort key offset **/
  nf=i=0;
  while(i < nfiles) {    /** get one line from each file **/
    if(readline((linbuf+lbp), tmpfd[i++])!=NULL) {
      linptr[++nf]=lbp;
      lbp=lbp+(MAXLINE+1);
      }
    }

  sort(1, nf);    /** make initial heap **/ /*04*/

  while(nf > 0) {
    poll(YES);
    lp1=linptr[1];
    ptr=linbuf+lp1;
    if(duptest(ptr)==NO) sout(ptr, outfil);
    inf=(lp1/(MAXLINE+1)); /** compute file index **/
    if(readline((linbuf+lp1), tmpfd[inf])==NULL)
      linptr[1]=linptr[nf--];
    reheap(nf);
    }
  }

reheap(nf) int nf; {
  int i, j;
  i=1;
  while((j=(i<<1)) <= nf) {
    if(j < nf) {      /** find smaller child **/
      if(compare(linptr[j], linptr[j+1]) > 0) ++j;
      }
    if(compare(linptr[i], linptr[j]) <= 0) break;
    exchange(i, j);     /** percolate **/
    i=j;
    }
  }

/*
** readline -- read next line, set its sort key offset,
**         and return its length
*/
readline(str, fd) char *str; int fd; {
  int fld;
  char *ptr, *offset;
  if(fgets(str, MAXLINE+1, fd)==NULL) return NULL;
  ptr=offset=str-1;     /** location of offset field **/
  fld=field;
  if(delim) {           /** must search for field'th field **/
    *offset = -1;
    while(*(++ptr)) {
      if(fld < 0) {
        if(delim == ' ') {
          if((*ptr > ' ')&(*(ptr+1) <= ' ')) ++fld;
          }
        else if(*ptr == delim) ++fld;
        }
      else if((fld == 0)&((delim != ' ')|(*ptr > ' '))) {
        *offset=(ptr-str);
        fld=1;
        }
      }
    if (*offset == -1) *offset=(ptr-str); /** end of line **/
    }
  else {  /** field is the column number of the sort key **/
    while(*(++ptr));
    if(field < (ptr-str)) *offset=field;
    else                  *offset=(ptr-str);
    }
  return (ptr-str+2); /** includes NULL and next offset **/
  }

#include "out.c"
#include "cant.c"
#include "same.c"

