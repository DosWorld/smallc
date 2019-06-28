/*
** prt.c -- print files with optional page skips, headings, and line numbers.
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define MARGIN1 0
#define MARGIN2 1
char name[MAXFN], inclext[MAXFN];
int
  i,
  fin,
  files,
  headings,
  skips,
  numbers,
  begin,
  end,
  ready,
  pause;
int lineno, pageno, margin;
main(argc, argv) int argc, *argv; {
  headings=skips=numbers=YES;
  doargs(argc, argv);
  if(iscons(stdout)) freopen("PRN", "w", stdout);
  if(ready && isatty(stdout)) {
    fputs("ready printer... ", stderr);
    fgetc(stderr);
    }
  files=NO;
  i=0;
  while(getarg(++i, name, MAXFN, argc, argv)!=EOF) {
    if((name[0]=='-')|(name[0]==EXTMARK)) continue;
    files=YES;
    fin=fopen(name, "r");
    if(fin==0) cant(name);
    pageno=lineno=0;
    fprnt(name, fin);
    fclose(fin);
    if((lineno > 0)&(skips)) skip(PTRHIGH-lineno);
    }
  if(files==NO) {
    pageno=lineno=0;
    fprnt("<stdin>", stdin);
    if((lineno > 0)&(skips)) skip(PTRHIGH-lineno);
    }
  fclose(stdout);
  exit(0);
  }

doargs(argc, argv) int argc, *argv; {
  int i, j;
  inclext[0]=NULL;
  begin=0;
  end=32767;
  ready=YES;
  pause=NO;
  margin=0;
  i=0;
  while(getarg(++i, name, MAXFN, argc, argv)!=EOF) {
    if(name[0]==EXTMARK) {
      j=0;
      while(inclext[j]=toupper(name[j])) ++j;
      continue;
      }
    if(name[0]!='-') continue;
    if((same(name[1], 'b'))&(same(name[2], 'p'))) {
      utoi(name+3, &begin);
      continue;
      }
    if((same(name[1], 'e'))&(same(name[2], 'p'))) {
      utoi(name+3, &end);
      continue;
      }
    if((same(name[1], 'l'))&(same(name[2], 'm'))) {
      utoi(name+3, &margin);
      continue;
      }
    if((same(name[1], 'n'))&(same(name[2], 'r'))) {
      ready=NO;
      continue;
      }
    if(same(name[1], 'p')) {
      pause=YES;
      continue;
      }
    if(same(name[1], 'n')) {
      if(same(name[2], 'h')) {headings=NO; continue;}
      if(same(name[2], 's')) {   skips=NO; continue;}
      if(same(name[2], 'n')) { numbers=NO; continue;}
      }
    fputs("usage: PRT [file]... [.?] [-NN] [-NH|-NS]\n",
          stderr);
    fputs("     [-LM#] [-BP#] [-EP#] [-P] [-NR]\n", stderr);
    abort(7);
    }
  }

fprnt(name, fin)
  char name[]; int fin; {
  char line[MAXLINE+1], word[MAXLINE+1];
  int textline, fin2, i, temp;
  textline=0;
  while(fgets(line, MAXLINE+1, fin)!=NULL) {
    poll(YES);
    if((lineno==0)&(skips)) {
      if(++pageno > end) exit(0);
      if(pageno >= begin) {
        page(pageno, pause, stdout);
        skip(MARGIN1);
        if(headings) {
          temp=margin;
          while(temp--) printf(" ", 0);
          if(numbers) printf("      ", 0);
          printf("File: %s    Page: %d\n", name, pageno, 2);
          }
        else printf("\n",0);
        ++lineno;
        skip(MARGIN2);
        }
      lineno=lineno+MARGIN1+MARGIN2;
      }
    ++textline;
    ++lineno;
    if(pageno >= begin) {
      temp=margin;
      while(temp--) printf(" ", 0);
      if(numbers)
        printf("%5d %s", textline, line, 2);
      else printf("%s", line, 1);
      }
    if((lineno >= (PTRHIGH-PTRSKIP))&(skips)) {
      skip(PTRHIGH-lineno);
      lineno=0;
      }
    if(inclext[0]!=EXTMARK) continue;
    i=0;
    getwrd(line, &i, word);
    if((lexcmp(word, "#include")==0) ||
       (lexcmp(word, ".so")==0)) {
      if(getwrd(line, &i, word) >= MAXFN) cant(word);
      strip(word);
      i=0;
      while((word[i]!=EXTMARK) && (word[i])) ++i;
      if((inclext[1]) &&
         (lexcmp(word+i, inclext)!=0)) continue;
      if((fin2=fopen(word, "r"))==NULL) cant(word);
      fprnt(name, fin2);
      fclose(fin2);
      }
    }
  }

skip(n) int n; {
  int i;
  if(pageno < begin) return;
  i=0;
  while(++i <= n) {
    poll(YES);
    fputc('\n', stdout);
    }
  }
#include "cant.c"
#include "page.c"
#include "same.c"
#include "strip.c"
#include "getwrd.c"

