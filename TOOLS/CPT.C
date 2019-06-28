/*
** cpt.c -- encrypt or decrypt ASCII or binary files
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#define NOCCARGC
#define MAXKEY 81
#define CTLZ   26
main(argc, argv) int argc, *argv; {
  char c, key[MAXKEY];
  int i, keylen;
  auxbuf(stdin, 4096);
  keylen=getarg(1, key, MAXKEY, argc, argv);
  if((keylen==EOF)|(key[0]=='-')) {
    fputs("usage: CPT key\n", stderr);
    abort(7);
    }
  i=1;
  while(read(stdin, &c, 1) > 0) {
    poll(YES);
    if(isatty(stdin) && (c==CTLZ)) break;
    c=c^key[i-1];
    if(write(stdout, &c, 1) !=1 ) {
      fputs("output error\n", stderr);
      abort(7);
      }
    i=(i%keylen)+1;
    }
  }

