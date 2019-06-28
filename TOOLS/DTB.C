
/*
** dtb.c -- convert tabs to equivalent blanks
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXLIN1 (MAXLINE+1)
main(argc,argv) int argc, *argv; {
  char c, tabs[MAXLIN1];
  int col, i;
  auxbuf(stdin, 4096);
  if(settab(tabs, argc, argv)==ERR) {
    fputs("usage: DTB [#]... [+#]\n", stderr);
    abort(7);
    }
  col=1;
  while((c=getchar())!=EOF) {
    poll(YES);
    if(c=='\t')
      while(YES) {
        cout(' ', stdout);
        ++col;
        if(tabpos(col, tabs)==YES) break;
        }
    else if(c=='\n') {
      cout('\n', stdout);
      col=1;
      }
    else {
      cout(c, stdout);
      ++col;
      }
    }
  fclose(stdout);
  }
#include "settab.c"
#include "tabpos.c"
#include "out.c"

