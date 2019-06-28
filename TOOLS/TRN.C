/*
** trn.c -- map characters
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXARG 80
#define MAXSET 127
main(argc, argv) int argc, *argv; {
  char arg[MAXARG+1], c, from[MAXSET+1], to[MAXSET+1];
  int allbut, collap, i, lastto, error;
  auxbuf(stdin, 4096);
  error=NO;
  if(getarg(1, arg, MAXARG, argc, argv)==EOF) error=YES;
  if(((arg[0]=='-')&(arg[1]<=' '))|(error==YES)) {
    fputs("usage: TRN [~]from [to]\n", stderr);
    abort(7);
    }
  if(arg[0]==NOT) {
    allbut=YES;
    if(makset(arg, 1, from, MAXSET)==NO)
      error("from-list too large");
    }
  else {
    allbut=NO;
    if(makset(arg, 0, from, MAXSET)==NO)
      error("from-list too large");
    }
  if(getarg(2, arg, MAXARG, argc, argv)==EOF) to[0]=NULL;
  else if(makset(arg, 0, to, MAXSET)==NO)
    error("to-list too large");
  lastto=strlen(to)-1;
  if((strlen(from)>(lastto+1))|(allbut==YES)) collap=YES;
  else collap=NO;
  while(1) {
    poll(YES);
    i=xindex(from, c=fgetc(stdin), allbut, lastto);
    if((collap==YES)&(i>=lastto)&(lastto>-1)) { /* collapse */
      if(fputc(to[lastto], stdout)==EOF)
        error("output error");
      while(1) {
        i=xindex(from, c=fgetc(stdin), allbut, lastto);
        if(i<lastto) break;
        }
      }
    if(c==EOF) break;
    if((i>-1)&(lastto>-1)) {            /* translate */
      if(fputc(to[i], stdout)==EOF)
        error("output error");
      }
    else if(i<0) {                       /* copy */
      if(fputc(c, stdout)==EOF)
        error("output error");
      }
    }                                    /* delete */
  fclose(stdout);
  }
#include "pat.c"
#include "error.c"
#include "index.c"
#include "xindex.c"
#include "makset.c"

