
/*
** chg.c -- change occurrences of "from" to "to"
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXARG 49
#define MAXLIN1 (MAXLINE+1)
char lin[MAXLIN1], new[MAXLIN1], pat[MAXPAT], sub[MAXPAT];
char arg[MAXARG];
int i, k, lastn, n;
main(argc, argv) int argc, *argv; {
  if((getarg(1, arg, MAXARG, argc, argv)==EOF)
    |((arg[0]=='-')&(arg[1]==0)))
    error("usage: CHG pattern [replacement]\n");
  if(makpat(arg, 0, NULL, pat)==ERR)
    error("pattern too long\n");
  if(getarg(2, arg, MAXARG, argc, argv)==EOF)
    arg[0]=NULL;
  if(maksub(arg, 0, NULL, sub)==ERR)
    error("replacement too long\n");
  auxbuf(stdin, 4096);  /** alloc aux buffer to stdin **/
  while(fgets(lin, MAXLIN1, stdin)!=NULL) {
    poll(YES);
    lastn = -1;
    i=0;  k=0;
    trim(lin);
    while(YES) {
      n=amatch(lin, i, pat);
      if((n>=0)&(lastn!=n)) {   /** replace matched text **/
        catsub(lin, i, n, sub, new, &k, MAXLIN1);
        lastn=n;
        }
      if(lin[i]==NULL) break;
      if((n==-1)|(n==i)) {      /** no match or null match **/
        addset(lin[i], new, &k, MAXLIN1);
        ++i;
        }
      else i=n;                 /** skip matched text **/
      }
    if(addset(NULL, new, &k, MAXLIN1)==NO) {
      k=MAXLIN1-1;
      addset(NULL, new, &k, MAXLIN1);
      sout("\7line truncated: ", stderr);
      lout(new, stderr);
      }
    lout(new, stdout);
    }
  fclose(stdout);
  }

#include "pat.c"
#include "maksub.c"
#include "catsub.c"
#include "index.c"
#include "error.c"
#include "out.c"
#include "trim.c"

