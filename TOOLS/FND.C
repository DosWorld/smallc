/*
** fnd.c -- find patterns in text file
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXARG 49
#define MAXLIN1 (MAXLINE+1)
char arg[MAXARG], lin[MAXLIN1], pat[MAXPAT];
int err, argi;
main(argc, argv) int argc, *argv; {
  auxbuf(stdin, 4096);
  err=NO;
  if((getarg(1, arg, MAXARG, argc, argv)==EOF)|(arg[0]=='-'))
    error("usage: FND [~]pattern\n");
  if(arg[0]==NOT) argi=1; else argi=0;
  if(getpat(arg+argi, pat)==ERR) error("pattern too long\n");
  while(fgets(lin, MAXLIN1, stdin)!=NULL) {
    poll(YES);
    trim(lin);
    if(match(lin, pat)) {
      if(argi==0) lout(lin, stdout);
      }
    else if(argi==1) lout(lin, stdout);
    }
  fclose(stdout);
  }

getpat(arg, pat) char arg[], pat[]; {
  return(makpat(arg, 0, NULL, pat));
  }

#include "pat.c"
#include "error.c"
#include "index.c"
#include "out.c"
#include "trim.c"

