/*
** fmt.c -- text formatter
**
** Copyrignt 1982 J. E. Hendrix.  All rights reserved.
*/

#include <stdio.h>
#include "tools.h"
#define NOCCARGC
#define KILL 14  /* control-N */
#define INSIZE 396
#define MAXOUT 579
#define COMMAND '.'
#define PAGENUM '#'
#define TTLDELIM '/'
#define MRGCHAR '|'
#define SUBCHAR '/'
#define SUPCHAR '\\'
#define BLANK '~'
#define DEFM1    1
#define DEFM2    2
#define DEFM3    2
#define DEFM4    9
#define DEFLM    9      /* LM is effectively one greater */
#define DEFRM   73
#define DEFMP    2
#define PAGELEN 66
#define HUGE 32000
#define NDIM     5
#define NWIDTH   4

int
  ascii,   /* pure ascii output mode */
  ttymode, /* tty mode if YES */
  itsub,   /* italic subs for underlines */
  ulsub,   /* underline subs for italics */
  pass,    /* pass counter */
  fill,    /* fill if YES */
  fillset, /* true if -NF given */
  just,    /* justify if YES */
  justset, /* true if -NJ given */
  lsval,   /* line spacing */
  inval,   /* indent value */
  lmval,   /* left margin value */
  lmset,   /* true if -LM# given */
  rmval,   /* right margin value */
  rmset,   /* true if -RM# given */
  pagekill,/* kill page if YES */
  pospec,  /* page offset specification */
  poval,   /* page offset for current page */
  tival,   /* temporary indent */
  ceval,   /* number of lines to center */
  ulval,   /* number of lines to underline */
  bfval,   /* number of lines to boldface */
  itval,   /* number of lines to italicize */
  dwval,   /* number of lines to double-wide */
  dwact,   /* double-wide is active */
  bfstrikes,  /* number of boldface strikes */
  cuact,   /* continuous underline active if > 0 */
  mpcnt,   /* lines printed since last .mp page break */
  mpval,   /* minimum lines required for paragraphs */
  sqval,   /* squeeze indents */
  bline,   /* blank line flag */
  curpag,  /* current page number */
  newpag,  /* next page number */
  lineno,  /* next line to be printed */
  plval,   /* page length in lines */
  m1val,   /* margin before and including header */
  m2val,   /* margin after header */
  m3val,   /* margin after last text line */
  m4val,   /* bottom margin including footer */
  bottom,  /* last live line on page */
  outp,    /* last character position in outbuf */
  outw,    /* width of text in outbuf */
  outwds,  /* number of words in outbuf */
  pause,   /* pause on page breaks if YES */
  ready,   /* prompt "ready printer..." if YES */
  pbreak,  /* page breaks if YES */
  pheader, /* page headers if YES */
  pfooter, /* page footers if YES */
  show,    /* show source file names */
  begin,   /* beginning page to print */
  end,     /* ending page to print */
  bcopy,   /* beginning copy */
  ecopy,   /* ending copy */
  mrgfd,   /* merge file fd */
  sentend, /* YES for plausible end of sentence */
  dir;     /* direction for spreading blanks */

char
       sbpref[]={27,'S',49,1},  /* nontty subscript prefix */
       sbsuff[]={27,'T', 1,0},  /* nontty subscript suffix */
       sppref[]={27,'S',48,1},  /* nontty superscript prefix */
       spsuff[]={27,'T', 1,0},  /* nontty superscript suffix */
      *ulon, *uloff,
       ulpref[]={27,'-',49,1},  /* nontty underline prefix */
       ulsuff[]={27,'-',48,1},  /* nontty underline suffix */
      *bfon, *bfoff,
       bfpref[]={27,'E',1,0},   /* nontty boldface prefix */
       bfsuff[]={27,'F',1,0},   /* nontty boldface suffix */
      *dson, *dsoff,
       dspref[]={27,'G',1,0},   /* nontty dbl strike prefix */
       dssuff[]={27,'H',1,0},   /* nontty dbl strike suffix */
      *iton, *itoff,
       itpref[]={27,'4',1,0},   /* nontty italics prefix */
       itsuff[]={27,'5',1,0},   /* nontty italics suffix */
      *dwon, *dwoff,
       dwpref[]={27,'W',49,1},  /* nontty dbl wide prefix */
       dwsuff[]={27,'W',48,1};  /* nontty dbl wide suffix */

char
   cmd,      /* command character */
   mrg,      /* merge field delimiter */
   blank,    /* literal blank character */
  *ulskips,  /* specific characters to skip underlining */
  *bfskips,  /* specific characters to skip bolding */
  *oheader,  /* odd page header */
  *eheader,  /* even page header */
  *ofooter,  /* odd page footer */
  *efooter,  /* even page footer */
  *outbuf,   /* output buffer */
  *ttlbuf,   /* buffer for titles */
  *mrgbuf,   /* buffer for merged text */
  *inbuf,    /* input buffer */
  *wrdbuf,   /* space for one word */
  *nbrstr;   /* number string */

main(argc, argv) int argc, *argv; {
  oheader=malloc((MAXLINE+1));
  eheader=malloc((MAXLINE+1));
  ofooter=malloc((MAXLINE+1));
  efooter=malloc((MAXLINE+1));
  mrgbuf=malloc((MAXLINE+1));
  outbuf=malloc(MAXOUT);
  ttlbuf=malloc(MAXOUT);
  wrdbuf=malloc(MAXOUT);
  inbuf=malloc(INSIZE);
  nbrstr=malloc(NDIM);
  nbrstr[NWIDTH]=NULL;
  doargs(argc, argv);
  mpcnt=99;
  pass=0;
  if(mrgfd) {
    while(fgets(mrgbuf, (MAXLINE+1), mrgfd) && (pass < ecopy)) {
      if(++pass < bcopy) continue;
      trim(mrgbuf);
      nextpass();
      }
    }
  else if(ecopy == HUGE) nextpass();
  else for(pass = bcopy; pass <= ecopy; ++pass) nextpass();
  fclose(stdout);
  }

/*
** nextpass -- make next pass on stdin text
*/
nextpass() {
  char c, str[6];
  init();
  if(mrgfd || ecopy != HUGE) {
    cseek(stdin, 0, 0);
    left(itod(pass, str, 6));
    fputs("copy ", stderr);
    fputs(str, stderr);
    fputs("   ", stderr);
    }
  c=NULL;
  if(((pass <= bcopy) || (pause==YES)) &&
     ready && isatty(stdout)) {
    fputs("ready printer... ", stderr);
    c=fgetc(stderr);
    }
  if(c!=KILL) {
    process(stdin);
    if((lineno > 0)|(outp > -1)) space(HUGE);
    }
  else fputc('\n', stderr);
  }

/*
** process -- process text from fd
*/
process(fd) int fd; {
  while(fgets(inbuf, INSIZE, fd)!=NULL) {
    poll(YES);
    trim(inbuf);
    if((fd==stderr)&(inbuf[0]==cmd)&(inbuf[1]==NULL))
      return;
    if(inbuf[0]==cmd) command(inbuf);
    else {
      if(merge()==NO) continue;
      text(inbuf);
      }
    }
  }

/*
** include -- process included text (nexting allowed)
*/
include() {
  int i, fd;
  char fname[INSIZE];
  i=0;
  while((inbuf[i]!=' ')&(inbuf[i]!=NULL)) ++i;
  getwrd(inbuf, &i, fname);
  if((fd=fopen(fname, "r"))==NULL) cant(fname);
  lout(fname, stderr);
  if(show) {
    brk();
    put(fname);
    brk();
   }
  process(fd);
  fclose(fd);
  }

/*
** prompt -- prompt operator for input
*/
prompt() {
  fputs("\7enter: ", stderr);
  fputs(inbuf+3, stderr);
  fputc('\n', stderr);
  process(stderr);
  }

/*
** init -- initialize parameters
*/
init() {
  cmd=COMMAND;
  mrg=MRGCHAR;
  blank=BLANK;
  sentend=NO;
  if(!fillset) fill=YES;
  if(!justset) just=YES;
  lsval=1;
  inval=tival=sqval=0;
  if(!lmset) lmval=DEFLM;
  if(!rmset) rmval=DEFRM;
  bfval=itval=ceval=ulval=cuact=0;
  curpag=0;
  newpag=1;
  lineno=0;
  mpval=DEFMP;
  bline=NO;       /* prevents a blank first page */
  plval=PAGELEN;
  m1val=DEFM1;
  m2val=DEFM2;
  m3val=DEFM3;
  m4val=DEFM4;
  bottom=plval-m3val-m4val;
  oheader[0]=eheader[0]=NULL;
  ofooter[0]=efooter[0]=NULL;
  outp=-1;
  outw=outwds=0;
  dir=0;
  }

/*
** command -- process commands
*/
command(buf) char buf[]; {
  int argtyp, spval, val;
  if(buf[1] == cmd) return;   /* comment */
  val=getval(buf, &argtyp);
  switch((toupper(buf[1]) << 8) + toupper(buf[2])) {
    default: {
      fputs("\7note: ", stderr);
      fputs(buf, stderr);
      fputc('\n', stderr);
      return;
      }
    case 'FI': {
      brk();
      fill=YES;
      break;
      }
    case 'NF': {
      brk();
      fill=NO;
      break;
      }
    case 'JU': {
      brk();
      just=YES;
      break;
      }
    case 'NJ': {
      brk();
      just=NO;
      break;
      }
    case 'BR':{
      brk();
      break;
      }
    case 'LS': {
      setvalue(&lsval, val, argtyp, 1, 1, HUGE);
      break;
      }
    case 'HE': {
      gettl(buf, oheader);
      gettl(buf, eheader);
      break;
      }
    case 'OH': {
      gettl(buf, oheader);
      break;
      }
    case 'EH': {
      gettl(buf, eheader);
      break;
      }
    case 'FO': {
      gettl(buf, ofooter);
      gettl(buf, efooter);
      break;
      }
    case 'OF': {
      gettl(buf, ofooter);
      break;
      }
    case 'EF': {
      gettl(buf, efooter);
      break;
      }
    case 'SP': {
      setvalue(&spval, val, argtyp, 1, 0, HUGE);
      space(spval);
      bline=YES;
      break;
      }
    case 'BP': {
      if((lineno > 0)|(outp > -1)) space(HUGE);
      setvalue(&curpag, val, argtyp, newpag, -HUGE, HUGE);
      newpag=curpag;
      break;
      }
    case 'PL': {
      if(val==0) {
        lineno=1;
        plval=0;
        }
      else {
        setvalue(&plval, val, argtyp,
            PAGELEN, m1val+m2val+m3val+m4val+1, HUGE);
        bottom=plval-m3val-m4val;
        }
      break;
      }
    case 'IN': {
      brk();
      setvalue(&inval, val, argtyp, 0, -lmval, rmval-lmval-1);
      break;
      }
    case 'LM': {
      brk();
      setvalue(&lmval, val-1, argtyp, DEFLM, 0, rmval-1);
      break;
      }
    case 'RM': {
      setvalue(&rmval, val, argtyp, DEFRM, lmval+1, HUGE);
      break;
      }
    case 'TI': {
      brk();
      setvalue(&tival, val, argtyp, 0,
          -(lmval+inval), rmval-(lmval+inval)-1);
      break;
      }
    case 'SQ': {
      brk();
      setvalue(&sqval, val, argtyp, 0, 0, HUGE);
      break;
      }
    case 'CE': {
      brk();
      setvalue(&ceval, val, argtyp, 1, 0, HUGE);
      break;
      }
    case 'BF': {
      if(!ascii) {
        setvalue(&bfval, val, argtyp, 1, 0, HUGE);
        bfskips="(),.;:?";
        ulval=0;
        }
      break;
      }
    case 'CB': {
      if(!ascii) {
        setvalue(&bfval, val, argtyp, 1, 0, HUGE);
        bfskips="";
        ulval=0;
        }
      break;
      }
    case 'IT': {
      if(!ascii)
        setvalue(&itval, val, argtyp, 1, 0, HUGE);
      break;
      }
    case 'DW': {
      if(!ascii)
        setvalue(&dwval, val, argtyp, 1, 0, HUGE);
      break;
      }
    case 'UL': {
      if(!ascii) {
        setvalue(&ulval, val, argtyp, 1, 0, HUGE);
        cuact=0;
        ulskips=" ()[]{}\t\b,.;:?!_'\"";
        bfval=0;
        }
      break;
      }
    case 'CU': {
      if(!ascii) {
        setvalue(&ulval, val, argtyp, 1, 0, HUGE);
        if(ulval==0) cuact=0;
        else cuact=1;    /* becomes 2 after next putwrd */
        if(ttymode) ulskips=" \t\b";
        else        ulskips= "\t\b";
        }
      break;
      }
    case 'NU': {
      ulval=cuact=0;
      break;
      }
    case 'MP': {
      setvalue(&mpval, val, argtyp, DEFMP, 0, HUGE);
      break;
      }
    case 'NE': {
      if(((bottom-lineno) < val) && (lineno > 0) && pbreak)
        space(HUGE);
      break;
      }
    case 'RS': {
      if(((bottom-lineno) < val) && (lineno > 0))
        space(HUGE);
      space(val);
      break;
      }
    case 'SO': {
      include();
      break;
      }
    case 'PR': {
      prompt();
      break;
      }
    case 'CC': {
      val=0;
      getwrd(buf, &val, wrdbuf);
      if(getwrd(buf, &val, wrdbuf)==1) cmd=wrdbuf[0];
      break;
      }
    case 'MC': {
      val=0;
      getwrd(buf, &val, wrdbuf);
      if(getwrd(buf, &val, wrdbuf)==1) mrg=wrdbuf[0];
      break;
      }
    case 'BC': {
      val=0;
      getwrd(buf, &val, wrdbuf);
      if(getwrd(buf, &val, wrdbuf)==1) blank=wrdbuf[0];
      break;
      }
    case 'M1': {
      setvalue(&m1val, val, argtyp, DEFM1, 0, HUGE);
      break;
      }
    case 'M2': {
      setvalue(&m2val, val, argtyp, DEFM2, 0, HUGE);
      break;
      }
    case 'M3': {
      setvalue(&m3val, val, argtyp, DEFM3, 0, HUGE);
      bottom=plval-m3val-m4val;
      break;
      }
    case 'M4': {
      setvalue(&m4val, val, argtyp, DEFM4, 0, HUGE);
      bottom=plval-m3val-m4val;
      break;
      }
    case 'PO': {
      setvalue(&pospec, val, argtyp, 0, 0, HUGE);
      break;
      }
    }
  }

#include "fmt2.c"
#include "fmt3.c"
#include "scopy.c"
#include "getwrd.c"
#include "cant.c"
#include "page.c"
#include "index.c"
#include "same.c"
#include "trim.c"
#include "out.c"

