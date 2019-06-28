/*
** etb.c -- replace blanks with tabs and blanks
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXLIN1 (MAXLINE+1)
main(argc, argv) int argc, *argv; {
  char c, tabs[MAXLIN1];
  int col, newcol;
  auxbuf(stdin, 4096);
  if(settab(tabs, argc, argv)==ERR) {
    fputs("usage: ETB [#]... [+#]\n", stderr);
    abort(7);
    }
  col=1;
  while(YES) {
    poll(YES);
    newcol=col;
    while((c=getchar())==' ') {
      ++newcol;
      if(tabpos(newcol, tabs)==YES) {
        cout('\t', stdout);
        col=newcol;
        }
      }
    if(c=='\t') {
      while(tabpos(newcol, tabs)==NO) ++newcol;
      cout('\t', stdout);
      col=newcol;
      continue;
      }
    while(col<newcol) {
      cout(' ', stdout);
      ++col;
      }
    if(c==EOF) break;
    cout(c, stdout);
    if(c=='\n') col=1;
    else if(c=='\b') --col; /* ok if it goes neg */
    else ++col;
    }
  fclose(stdout);
  }
#include "settab.c"
#include "tabpos.c"
#include "out.c"

