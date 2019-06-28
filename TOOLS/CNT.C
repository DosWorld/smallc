
/*
** cnt.c -- count characters, words, and/or lines
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
char strc[6], strw[6], strl[6];
main(argc, argv) int argc, *argv; {
  char arg[MAXFN], *nc, *nl, *nw;
  int c, f, i, fd, inword;
  fd=stdin;
  i=f=0;
  while(getarg(++i, arg, MAXFN, argc, argv) != EOF) {
    if(arg[0] != '-') {
      if((fd = fopen(arg, "r")) == 0) cant(arg);
      continue;
      }
    switch(f = tolower(arg[1])) {
      case 'c': case 'w': case 'l': continue;
      default:
        fputs("usage: CNT [file] [-C|-W|-L]\n", stderr);
        abort(7);
      }
    }
  nc=nl=nw=0;
  inword=NO;
  while((c=fgetc(fd))!=EOF) {
    poll(YES);
    if(c=='\n') {
      ++nl;
      }
    else ++nc;
    if(isspace(c)) inword=NO;
    else if(inword==NO) {
      inword=YES;
      ++nw;
      }
    }
  itou(nc, strc, 6);
  itou(nw, strw, 6);
  itou(nl, strl, 6);
  switch(f) {
    case 'c': lout(strc, stdout); break;
    case 'w': lout(strw, stdout); break;
    case 'l': lout(strl, stdout); break;
    default:
      sout(strc, stdout); lout(" characters", stdout);
      sout(strw, stdout); lout(" words", stdout);
      sout(strl, stdout); lout(" lines", stdout);
    }
  fclose(stdout);
  }
#include "cant.c"
#include "out.c"

