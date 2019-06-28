
/*
** cpy.c -- copy named files to standard output
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define MAXARG 12
int fin, i, j;
int status;
int binary, striplf, stripcr, from, to;
char name[MAXFN], inclext[MAXFN];
main(argc, argv) int argc, *argv; {
  auxbuf(stdout, 4096);
  doargs(argc, argv);
  if((binary==YES)&(inclext[0]!=NULL)) {
    fputs("cannot include files during binary copy", stderr);
    abort(7);
    }
  fin=99;
  i=0;
  while(getarg(++i, name, MAXFN, argc, argv)!=EOF) {
    if((name[0]=='-')|(name[0]==EXTMARK)) continue;
    if((fin=fopen(name, "r"))==NULL) cant(name);
    if(binary) bcopy(fin, stdout);
    else       fcopy(fin, stdout);
    fclose(fin);
    }
  if(fin==99) {
    if(binary) bcopy(stdin, stdout);
    else       fcopy(stdin, stdout);
    }
  fclose(stdout);
  }

doargs(argc, argv) int argc, *argv; {
  int len;
  char arg[MAXARG], error;
  inclext[0]=from=to=NULL;
  binary=striplf=stripcr=error=NO;
  i=0;
  while(getarg(++i, arg, MAXARG, argc, argv)!=EOF) {
    if(arg[0]==EXTMARK) {
      j=0;
      while(inclext[j]=arg[j]) ++j;
      continue;
      }
    if(arg[0]!='-') continue;
    if(arg[2]==NULL) {
      if(same(arg[1], 'b')) binary=YES;
      else error=YES;
      }
    else if(arg[4]==NULL) {
      if(same(arg[1], 'n')) {
        if(same(arg[2], 'c') & same(arg[3], 'r'))
          stripcr=binary=YES;
        else if(same(arg[2], 'l') & same(arg[3], 'f'))
          striplf=binary=YES;
        else error=YES;
        }
      else error=YES;
      }
    else if(same(arg[1], 't')) {
      binary=YES;
      len=utoi(arg+2, &from);
      if((len<1)|(arg[len+2]!=',')) error=YES;
      else {
        j=len+3;
        len=utoi(arg+j, &to);
        if((len<1)|(arg[len+j]!=NULL)) error=YES;
        }
      }
    else error=YES;
    if(error) {
      fputs("usage: CPY [file]... [.?] [-B] [-NCR] [-NLF] [-T#,#]\n", stderr);
      abort(7);
      }
    }
  }

bcopy(in, out) int in, out; {
  char c[1];
  while(YES) {
    status=read(in, c, 1);
    if(status==0) break;
    if(isatty(in)&(c[0]==4)) break;
    if(status < 1) {
      fputs("input error\n", stderr);
      fclose(out);
      abort(7);
      }
    if((c[0]==CR)&(stripcr)) continue;
    if((c[0]==LF)&(striplf)) continue;
    if((from!=to)&((c[0]&255)==from)) c[0]=to;
    status=write(out, c, 1);
    if(status < 1) {
      fputs("output error\n", stderr);
      fclose(out);
      abort(7);
      }
    poll(YES);
    }
  }

fcopy(in, out) int in, out; {
  int i, loc, in2;
  char buf[MAXLINE+1], str[MAXLINE+1];
  while(fgets(buf, MAXLINE+1, in)!=NULL) {
    poll(YES);
    if(inclext[0]==NULL) {
      sout(buf, out);
      continue;
      }
    loc=0;
    getwrd(buf, &loc, str);
    if((lexcmp(str, "#include")!=0) &&
       (lexcmp(str, ".so")!=0)) {
      sout(buf, out);
      continue;
      }
    getwrd(buf, &loc, str);
    strip(str);
    i=0;
    while((str[i]!=EXTMARK) && str[i]) ++i;
    if(inclext[1] && lexcmp(str+i, inclext)) {
      sout(buf, out);
      continue;
      }
    if((in2=fopen(str, "r"))==NULL) cant(str);
    fcopy(in2, out);
    fclose(in2);
    }
  }

#include "out.c"
#include "cant.c"
#include "same.c"
#include "strip.c"
#include "getwrd.c"

