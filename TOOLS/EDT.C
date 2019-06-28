/*
** edt.c -- edit text
**
** Copyright 1982 J. E. Hendrix.  All rights reserved.
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define OK 1
#define RESERVE 600
#define PREV 0
#define NEXT 2
#define MARK 4
#define TEXT 5
#define INTEGER 2
#define LONG 4
#define LINE0 0
#define NOSTATUS 0
#define CURLINE '.'
#define LASTLINE '|'
#define SCAN '/'
#define BACKSCAN '\\'
#define FORWARD 1
#define BACKWARD 0
#define PERIOD '.'
#define COMMA ','
#define SEMICOL ';'
#define PLUS '+'
#define MINUS '-'
#define BLANK ' '
#define TAB '\t'
#define ESC 27
#define PROMPT '#'
#define CLFLAG '*'

#define PRINT 'p'
#define LINE 'l'
#define GLOBAL 'g'
#define EXCLUDE 'x'
#define APPEND 'a'
#define CHANGE 'c'
#define DELETE 'd'
#define INSERT 'i'
#define JOIN 'j'
#define MOVE 'm'
#define SUBSTITUTE 's'
#define ENTER 'e'
#define FILE 'f'
#define READ 'r'
#define WRITE 'w'
#define QUIT 'q'
#define VIEW 'v'
#define ZIP 'z'

int
  line1,   /* first line number */
  line2,   /* second line number */
  nlines,  /* number of line numbers given */
  curln,   /* current line (value of dot) */
  lastln;  /* last line (value of $) */

char *buf;    /* buffer for pointers and text */
/*
** buf[k+0] PREV     (2 bytes)  previous line
** buf[k+2] NEXT     (2 bytes)  next line
** buf[k+4] MARK     (1 byte)   mark for global commands
** buf[k+5] TEXT
*/
int  lastbf;  /* last element used in buf */
char *txt,    /* text line for matching and output */
     *savfil, /* remembered file name */
     *file;
int
  scr,        /* scratch file id */
  scrend[2];  /* end of info on scratch file */
char *lin, *pat, *sub, updtflag, nbrstr[7];
int cursav, i, status, maxbuf, context, view;

main(argc, argv) int argc, *argv; {
  if(isatty(stdin)) view=1; else view=0;
  txt=malloc(MAXLINE);
  lin=malloc(MAXLINE);
  pat=malloc(MAXPAT);
  sub=malloc(MAXPAT);
  file=malloc(MAXFN);
  savfil=malloc(MAXFN);
  maxbuf=avail(YES)-RESERVE;
  if(maxbuf < 0) maxbuf=32767;
  buf=malloc(maxbuf);
  setbuf();
  updtflag=NO;
  pat[0]=savfil[0]=nbrstr[6]=NULL;
  context=7;
  i=0;
  while(getarg(++i, txt, MAXFN, argc, argv)!=EOF) {
    if(txt[0]=='-') {
      if(same(txt[1], 'v')&(txt[2]==NULL)) view = 1 - view;
      else {
        fputs("usage: EDT [file] [-V]\n", stderr);
        abort(7);
        }
      }
    else scopy(txt, 0, savfil, 0);
    }
  if(*savfil) {
    if(enter(savfil)==ERR) {
      fputs("error\n", stderr);
      }
    }
  if(isatty(stdin))
    fputc(PROMPT, stderr);
  while(fgets(lin, MAXLINE, stdin)!=NULL) {
    poll(YES);
    trim(lin);
    i=0;
    cursav=curln;
    if(getlst()==OK) {
      if(ckglob()==OK)
        status=doglob();
      else if(status!= ERR)
        status=docmd(NO);
      /* else error, do nothing */
      }
    if(status==ERR) {
      fputs("\7error\n", stderr);
      curln=cursav;
      }
    else if(status==EOF) break;
    /* else OK, then loop */
    if(isatty(stdin)) fputc(PROMPT, stderr);
    }
  }

/*
** docmd -- handle all commands except globals
*/
docmd(glob) int glob; {
  int gflag, line3, pflag;
  pflag=NO;
  status=ERR;
  switch(tolower(lin[i])) {
    case APPEND:
      if(ckp(lin, i+1, &pflag)==OK)
        status=append(line2, glob);
      break;
    case CHANGE:
      if(ckp(lin, i+1, &pflag)==OK) {
        if(defalt(curln, curln)==OK) {
          if((status=append(line2, glob))!=ERR) {
            kill(line1, line2);
            ++curln;
            }
          }
        }
      break;
    case DELETE:
      if(ckp(lin, i+1, &pflag)==OK) {
        if(defalt(curln, curln)==OK) {
          kill(line1, line2);
          if(curln < lastln) curln=nextln(curln);
          }
        }
      break;
    case INSERT:
      if(ckp(lin, i+1, &pflag)==OK)
        status=append(prevln(line2), glob);
      break;
    case LINE:
      if(lin[i+1]==NULL) {
        itou(curln, nbrstr, 6);
        puts(nbrstr);
        status=OK;
        }
      break;
    case JOIN:
      if(ckp(lin, i+1, &pflag)==OK) {
        if(defalt(curln, curln+1)==OK) {
          if((status=join(line1, line2))==OK) {
            curln=line2;
            if((status=inject(txt))==OK)
              kill(line1, line2);
              ++curln;
            }
          }
        }
      break;
    case MOVE:
      ++i;
      if(getone(&line3)==EOF) status=ERR;
      if(status==OK) {
        if(ckp(lin, i, &pflag)==OK) {
          if(defalt(curln, curln)==OK) status=move(line3);
          }
        }
      break;
    case SUBSTITUTE:
      ++i;
      if(optpat()==OK) {
        if(getrhs(lin, &i, sub, &gflag)==OK) {
          if(ckp(lin, i+1, &pflag)==OK) {
            if(defalt(curln, curln)==OK)
              status=subst(sub, gflag);
            }
          }
        }
      break;
    case ENTER:
      if(chkupdt()==ERR) status=OK;
      else if(nlines==0) {
        if(getfn(lin, i, file, MAXFN)==OK) {
          scopy(file, 0, savfil, 0);
          clrbuf();
          setbuf();
          status=enter(file);
          }
        }
      break;
    case FILE:
      if(nlines==0) {
        if(getfn(lin, i, file, MAXFN)==OK) {
          scopy(file, 0, savfil, 0);
          puts(savfil);
          }
        itou(maxbuf-lastbf, nbrstr, 6);
        puts(nbrstr);
        status=OK;
        }
      break;
    case READ:
      if(getfn(lin, i, file, MAXFN)==OK)
        status=doread(line2, file);
      pflag=view;
      break;
    case WRITE:
      if(getfn(lin, i, file, MAXFN)==OK) {
        if(defalt(1, lastln)==OK)
          status=dowrit(line1, line2, file);
        }
      break;
    case ZIP:
      if(defalt(curln, lastln)==OK)
        status=doprnt(line1, line2, glob);
      break;
    case PRINT:
      if(defalt(curln, curln)==OK)
        status=doprnt(line1, line2, glob);
      break;
    case NULL:
      if((nlines==0)&(glob==NO)) line2=nextln(curln);
      if(view) status=doprnt(line2, line2, glob);
      else {
        curln=line2;
        status=OK;
        }
      break;
    case QUIT:
      if((lin[i+1]==NULL)&(nlines==0)&(glob==NO)) {
        if(chkupdt()==ERR) status=OK;
        else status=EOF;
        }
      break;
    case VIEW:
      view=1-view;
      status=OK;
    }
  /* else status is ERR */
  if(curln < 1) curln = nextln(0);
  if((status==OK)&(pflag==YES))
    status=doprnt(curln, curln, glob);
  return status;
  }

/*
** chkupdt -- warn if update not written to disk
*/
chkupdt() {
  if(updtflag) {
    fputs("didn't write to disk!\n", stderr);
    updtflag=NO;
    return ERR;
    }
  return OK;
  }

/*
** ctoi -- convert string at in[*i] to integer, bump *i
*/
ctoi(in, i) char in[]; int *i; {
  int dd, num;
  char *digits;
  digits="0123456789";
  while((in[*i]==BLANK)|(in[*i]==TAB)) *i = *i + 1;
  num=0;
  while(in[*i]!=NULL) {
    dd=index(digits, in[*i]);
    if(dd < 0) break;
    num = 10*num + dd;
    *i = *i + 1;
    }
  return num;
  }

/*
** skipbl -- skip blanks and tabs
*/
skipbl(lin, i) char lin[]; int *i; {
  while((lin[*i]==' ')|(lin[*i]=='\t')) *i = *i + 1;
  }

/*
** nextln -- get line after ln
*/
nextln(ln) int ln; {
  if(++ln > lastln) return 0;
  return ln;
  }

/*
** prevln -- get line before ln
*/
prevln(ln) int ln; {
  if(--ln < 0) return lastln;
  return ln;
  }

/*
** join -- put line1 thru line2 together into txt
*/
join(ln1, ln2) int ln1, ln2; {
  int i, j;
  j=0;
  while(ln1 <= ln2) {
    i=getind(ln1++)+TEXT;
    while(txt[j++]=buf[i++])
      if(j >= MAXLINE) return ERR;
    --j;
    }
  return OK;
  }

/*
** doread -- read "file" after "line"
*/
doread(line, file) int line; char file[]; {
  int fd, stat;
  if((fd=fopen(file, "r"))==NULL) {
    fputs("open ", stderr);
    return ERR;
    }
  curln=line;
  stat=input(fd);
  fclose(fd);
  return stat;
  }

/*
** getlst -- collect line numbers (if any) at lin[i], bump i
*/
getlst() {
  int num;
  line2=0;
  nlines=0;
  while(getone(&num)==OK) {
    line1=line2;
    line2=num;
    ++nlines;
    if((lin[i]!=COMMA)&(lin[i]!=SEMICOL)) break;
    if(lin[i]==SEMICOL) curln=num;
    ++i;
    }
  if(nlines>2) nlines=2;
  if(nlines==0) line2=curln;
  if(nlines<=1) line1=line2;
  if(status!=ERR) status=OK;
  return status;
  }

/*
** getone -- evaluate one line number expression
*/
getone(num) int *num; {
  int istart, mul, pnum;
  skipbl(lin, &i);
  istart=i;
  if((lin[i]==PLUS)|(lin[i]==MINUS)) *num=curln;
  else *num=0;
  if(getnum(num)==OK)
    while(YES) {
      skipbl(lin, &i);
      if((lin[i]!=PLUS)&(lin[i]!=MINUS)) {
        status=EOF;
        break;
        }
      if(lin[i]==PLUS) mul = 1;
      else mul = -1;
      ++i;
      skipbl(lin, &i);
      pnum=1;
      getnum(&pnum);
      *num = *num + mul*pnum;
      if(status==EOF) status=ERR;
      if(status!=OK) break;
      }
  if((*num<0)|(*num>lastln)) return (status=ERR);
  if(i<=istart) return (status=EOF);
  return (status=OK);
  }

#include "edt2.c"
#include "same.c"
#include "pat.c"
#include "buf.c"
#include "error.c"
#include "index.c"
#include "maksub.c"
#include "catsub.c"
#include "scopy.c"
#include "trim.c"

