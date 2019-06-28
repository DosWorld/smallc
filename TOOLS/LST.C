/*
** lst.c -- list text in columns on pages
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
int
  fd,     /* file descriptor for input */
  eof,    /* end of file if YES */
  cols,   /* number of cols */
  cwidth, /* col width */
  pwidth, /* page width */
  plength,/* page length */
  blanks, /* blank lines processed if YES */
  numbers,/* number lines if > 0 */
  pause;  /* pause before each page if YES */
main(argc, argv) int argc, *argv; {
  if(!isatty(stdout)) {        /* set default dimensions */
    pause=NO;
    pwidth=PTRWIDE-1;
    plength=PTRHIGH-PTRSKIP-PTRHDR;  /* page body height */
    }
  else {
    pause=YES;
    pwidth=CRTWIDE-1;
    plength=CRTHIGH-1;               /* allow for prompts */
    }
  blanks=YES;
  eof=NO;
  cols=1;
  numbers=1;
  fd=stdin;
  doargs(argc, argv);
  cwidth=pwidth/cols;
  while(eof==NO) {
    if(pause) {
      fputs("waiting... ", stderr);
      fgetc(stderr);
      }
    eof=column(fd, stdout, cwidth, cols, plength, &numbers);
    }
  fclose(stdout);
  }
doargs(argc, argv) int argc, *argv; {
  int i, j, k, err;
  char arg[MAXFN];
  err=NO;
  i=0;
  while(getarg(++i, arg, MAXFN, argc, argv) != EOF) {
    if(arg[0] != '-') {
      if(!(fd=fopen(arg, "r"))) {err=YES; break;}
      continue;
      }
    if(same(arg[1], 'c')) {
      if((j=utoi(arg+2, &cols)) < 1) {err=YES; break;}
      if((cols < 1)|(arg[j+2] > ' ')) {err=YES; break;}
      continue;
      }
    if(same(arg[1], 'n')) {
      if(arg[3] <= ' ') {
        if(same(arg[2], 'b')) {
          blanks=NO;
          continue;
          }
        if(same(arg[2], 'p')) {
          pause=NO;
          continue;
          }
        if(same(arg[2], 'n')) {
          numbers=0;
          continue;
          }
        err=YES;
        }
      }
    if(same(arg[1], 'p')) {
      if((j=utoi(arg+3, &k)) > 0) {
        if((k > 0)&(arg[j+3] <= ' ')) {
          if(same(arg[2], 'l')) {
            plength=k;
            continue;
            }
          if(same(arg[2], 'w')) {
            pwidth=k;
            continue;
            }
          }
        }
      }
    err=YES;
    }
  if(err) {
    fputs("usage: LST [file] [-C#] [-PW#] [-PL#] [-NB] [-NN] [-NP]\n",
          stderr);
    abort(7);
    }
  }
column(in, out, cwidth, cols, plength, number)
   int in, out, cwidth, cols, plength, *number; {
   int eof, lines, colcnt, lwidth, i, bufsz, linecnt;
  char *ptr, *stop, *eptr, *nexteptr, *buf, *bend;
  bufsz=cols*(cwidth+1)*plength;
  ptr=buf=malloc(bufsz);
  bend=buf+bufsz;
  eof=NO;
  while(ptr < bend) {
    poll(YES);
    if(*number) {
      i=4;
      itou(*number, ptr, -i);
      ptr[i++]=' ';
      }
    else i=0;
    if(fgets(ptr+i, cwidth+1-i, in)==NULL) {
      eof=YES;
      break;
      }
    trim(ptr+i);
    if((blanks==NO) && (ptr[i]==NULL)) continue;
    if(*number) ++(*number);
    ptr=ptr+cwidth+1;
    }
  stop=ptr;
  ptr=buf;
  lwidth=cols*(cwidth+1);
  lines=(stop-buf)/lwidth;
  if((stop-buf)%lwidth) ++lines;
  if(lines==0) return eof;
  linecnt=lines;
  while(linecnt--) {
    poll(YES);
    eptr=ptr;
    colcnt=cols;
    while(colcnt--) {
      sout(eptr, out);
      if((nexteptr=eptr+(lines*(cwidth+1))) >= stop) break;
      if(colcnt > 0) {
        if((i=strlen(eptr)) < cwidth) {
          i=cwidth-i;
          while(i--) cout(' ', out);
          }
        }
      eptr=nexteptr;
      }
    cout('\n', out);
    ptr=ptr+cwidth+1;
    }
  free(buf);
  return eof;
  }
#include "out.c"
#include "same.c"
#include "trim.c"

