/*
** mrg.c -- compares two sorted text files
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
int cmpr, fd1, fd2, i;
char *line1, *line2, column, arg[MAXFN];
main(argc, argv) int argc, *argv; {
  int error;
  column='0';
  fd1=fd2=stdin;
  line1=malloc(MAXLINE+1);
  line2=malloc(MAXLINE+1);
  error=NO;
  i=0;
  while(getarg(++i, arg, MAXFN, argc, argv)!=EOF) {
    if(arg[0]!='-') {
      if(fd1==stdin) {
        if((fd1=fopen(arg, "r"))==NULL) cant(arg);
        }
      else if(fd2==stdin) {
        if((fd2=fopen(arg, "r"))==NULL) cant(arg);
        }
      else error=YES;
      }
    else if(((arg[1] > '0')&(arg[1] < '4'))
            |(same(arg[1], 'f')))
      column=arg[1];
    else error=YES;
    }
  if((error)|(fd1==stdin)) {
    fputs("usage: MRG file [file] [-1|-2|-3|-F]\n", stderr);
    abort(7);
    }
  auxbuf(fd1, 4096);
  auxbuf(fd2, 4096);
  getline(line1, fd1);
  getline(line2, fd2);
  while(YES) {
    poll(YES);
    cmpr=lexcmp(line1, line2);
    if(cmpr < 0) {
      if(same(column, 'f')) fout("1) ", line1);
      else if((column=='0')|(column=='1')) sout(line1, stdout);
      getline(line1, fd1);
      continue;
      }
    else if(cmpr > 0) {
      if(same(column, 'f')) fout("  2) ", line2);
      else if((column=='0')|(column=='2')) sout(line2, stdout);
      getline(line2, fd2);
      continue;
      }
    if(line1[0] == 127) break;
    if(same(column, 'f'))   fout("    3) ", line1);
    else if((column=='0')|(column=='3')) sout(line1, stdout);
    getline(line1, fd1);
    getline(line2, fd2);
    }
  fclose(stdout);
  }

getline(line, fd) char *line; int fd; {
  if(fgets(line, MAXLINE+1, fd)==NULL) {
    line[0] = 127;
    line[1] = NULL;
    }
  }

/*
** fout -- formatted output of a line
*/
fout(header, data) char *header, *data; {
  sout(header, stdout);
  sout(data, stdout);
  }

#include "out.c"
#include "cant.c"
#include "same.c"

