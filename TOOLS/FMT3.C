/*
** fmt3.c -- text formatter part 3
*/

/*
** put -- put out line with proper spacing and indenting
*/
put(buf) char buf[]; {
  int i;
  if((buf[0]==NULL)|(buf[0]==' ')) bline=YES;
  else {
    if(((bottom-lineno+1) < (mpval*lsval)) &&
       (bline==YES) &&
       (mpcnt >= mpval)) {
      mpcnt=0;
      skip(bottom-lineno+1);
      lineno=bottom+1;   /* force footing */
      }
    bline=NO;
    }
  ++mpcnt;
  if(lineno>bottom) pfoot();
  if(lineno==0) phead();
  i=poval;  /** minus on odd pages **/
  while(++i <= (lmval+tival+inval+sqval))
    print(" ", NO);
  tival=0;
  print(buf, NO);
  if((lsval-1) < (bottom-lineno)) skip(lsval);
  else skip(bottom-lineno+1);
  if(plval > 0) lineno=lineno+lsval;
  if(lineno > bottom) pfoot();
  }

/*
** print -- print output to stdout if not skipping pages
*/
print(str, title) char *str; int title; {
  while(*str) {
    if(*str == 1) {
      if(!title) onoff(*(str-1));
      }
    else {
      /* skip output of control sequence terminator */
      if((plval==0)|((curpag >= begin)&(pagekill==NO))) {
        begin=1;  /** future copies begin on page 1 **/
        if(*str != blank) cout(*str, stdout);
        else cout(' ', stdout);
        }
      else if(*str == 27) {
        while(*str != 1) cout(*str++, stdout);
        continue;
        }
      }
    ++str;
    }
  }

/*
** onoff -- set and clear xxon and xxoff pointers
*/
onoff(ch) int ch; {
       if(ch == ulpref[2]) {ulon=ulpref; uloff=ulsuff;}
  else if(ch == ulsuff[2]) ulon=uloff=0;
  else if(ch == bfpref[1]) {bfon=bfpref; bfoff=bfsuff;}
  else if(ch == bfsuff[1]) bfon=bfoff=0;
  else if(ch == dspref[1]) {dson=dspref; dsoff=dssuff;}
  else if(ch == dssuff[1]) dson=dsoff=0;
  else if(ch == itpref[1]) {iton=itpref; itoff=itsuff;}
  else if(ch == itsuff[1]) iton=itoff=0;
  else if(ch == dwpref[1]) {dwon=dwpref; dwoff=dwsuff;}
  else if(ch == dwsuff[1]) dwon=dwoff=0;
  }

/*
** skip -- output n blank lines
*/
skip(n) int n; {
  while((n--) > 0) print("\n", NO);
  }

/*
** phead -- put out header
*/
phead() {
  pagekill=NO;
  if((curpag=newpag++) >= begin) {
    if(page(curpag, pause, stdout)==KILL) {
      pagekill=YES;
      fputc('\n', stderr);
      }
    }
  if(curpag%2) poval = -pospec;
  else poval = pospec;
  if(pbreak) {
    if(m1val > 0) {
      if(pheader) {
        skip(m1val-1);
        if(curpag%2) puttl(oheader, curpag);
        else         puttl(eheader, curpag);
        }
      else skip(m1val);
      }
    skip(m2val);
    }
  lineno = m1val + m2val + 1;
  }

/*
** pfoot -- put out footer
*/
pfoot() {
  if(pbreak) {
    skip(m3val);
    if(m4val > 0) {
      if(pfooter)
        if(curpag%2) puttl(ofooter, curpag);
        else         puttl(efooter, curpag);
      else           skip(1);
      }
    }
  if((curpag >= end) && ((ecopy == HUGE) || (ecopy == pass))) {
    fclose(stdout);
    exit(0);
    }
  if(pbreak && pause == NO) skip(m4val-1);
  lineno=0;
  }

/*
** puttl -- put out title line with optional page number
*/
puttl(buf, pageno) char buf[]; int pageno; {
  char str[2];
  int i, j, k ,parts;
  str[1]=NULL;
  i=poval;  /** minus on odd pages **/
  while(++i <= lmval) print(" ", YES);
  parts=1;
  i=k=0;
  while(buf[i]!=NULL) {
    if(buf[i]==PAGENUM) {
      itod(pageno, nbrstr, - NWIDTH);
      j=0;
      while(nbrstr[j]==' ') ++j;
      while(j < NWIDTH) {
        if(k < (MAXOUT-1)) ttlbuf[k++]=nbrstr[j];
        ++j;
        }
      }
    else {
      if(buf[i]==TTLDELIM) ++parts;
      if(k < (MAXOUT-1)) ttlbuf[k++]=buf[i];
      }
    ++i;
    }
  ttlbuf[k]=NULL;
  spread(ttlbuf, k+1, MAXOUT, rmval-lmval-k, parts, TTLDELIM);
  /* disable running ul, it, dw, ds, or bf */
  if(uloff) print(uloff, YES);
  if(itoff) print(itoff, YES);
  if(dsoff) print(dsoff, YES);
  if(bfoff) print(bfoff, YES);
  if(dwoff) print(dwoff, YES);
  print(ttlbuf, YES);
  /* reenable running ul, it, dw, ds, or bf */
  if(ulon) print(ulon, YES);
  if(iton) print(iton, YES);
  if(dson) print(dson, YES);
  if(bfon) print(bfon, YES);
  if(dwon) print(dwon, YES);
  print("\n", YES);
  }

/*
** gettl -- copy title from buf to ttl
*/
gettl(buf, ttl) char *buf, *ttl; {
  int i;
  i=0;
  while((buf[i]!=' ')&(buf[i]!='\t')&(buf[i]!=NULL)) ++i;
  skipbl(buf, &i);
  if((buf[i]=='\'')|(buf[i]=='"')) ++i; /** strip leading quote **/
  buf=buf+i;
  while(*ttl++ = *buf++);
  }

/*
** space -- space n lines or to bottom
*/
space(n) int n; {
  brk();
  if(lineno > bottom) return;
  if(lineno==0) {
    phead();
    if(n==HUGE) return;
    }
  if(n < (bottom+1-lineno)) skip(n);
  else                      skip(bottom+1-lineno);
  if(plval > 0) lineno=lineno+n;
  if(lineno > bottom) pfoot();
  }

/*
** leadbl -- delete leading blanks, set tival
*/
leadbl(buf) char buf[]; {
  int i, j;
  brk();
  i=0;
  while(buf[i]==' ') ++i;
  if(buf[i]!=NULL) tival = tival + i;
  j=0;
  while(buf[j++]=buf[i++]);
  }

/*
** width -- compute width of character string
*/
width(buf) char *buf; {
  int wide;
  wide=0;
  while(*buf) {
    if(*buf==27) {
      while(*buf++ != 1) ;
      continue;
      }
    if(*buf == '\b') --wide;
    else if(*buf != '\n') {
      ++wide;
      if(dwact && !isspace(*buf)) ++wide;
      }
    ++buf;
    }
  return (wide);
  }

/*
** brk -- end current filled line
*/
brk() {
  if(outp > -1) {
    outbuf[outp]=NULL;
    outp=-1;
    put(outbuf);
    }
  outw=outwds=0;
  }

/*
** putwrd -- put a word in outbuf (does margin justification)
*/
putwrd(wrdbuf) char wrdbuf[]; {
  int last, llval, nextra, w, i;
  w=width(wrdbuf);
  last=strlen(wrdbuf)+outp+1;
  llval=rmval-lmval-tival-inval-(sqval<<1);
  if((outp > -1)&((outw+w > llval)|(last >= MAXOUT))) {
    last=last-outp-1;
    if(just!=YES) nextra=0;
    else nextra=llval-outw+1;
    spread(outbuf, outp, MAXOUT, nextra, outwds, ' ');
    if((nextra > 0)&(outwds > 1)) outp=outp+nextra;
    brk();
    }
  if(outp > 0 && cuact > 1 && !itsub && ttymode)
    outbuf[outp]='_';
  else {
    ++outwds;
    if(cuact==1) cuact=2;
    }
  if((outp < 0)&(wrdbuf[0]==' ')) i=1; else i=0;
  scopy(wrdbuf+i, 0, outbuf, outp+1);
  outp=last-i;
  outbuf[outp]=' ';
  outw=outw+w+1-i;
  }

/*
** spread -- spread words to justify right margin
*/
spread(buf, outp, max, nextra, outwds, gapid)
  char buf[], gapid; int outp, max, nextra, outwds; {
  int i, j, nb, ne, nholes;
  if((nextra <= 0)|(outwds <= 1)) return;
  dir=1-dir; /* reverse prev direction */
  ne=nextra;
  nholes=outwds-1;
  i=outp-1;
  if((max-2) < (i+ne)) j=max-2;
  else j=i+ne;
  while(i < j) {
    if(buf[i]==gapid) {
      buf[j]=' ';
      if(dir==0) nb=(ne-1)/nholes + 1;
      else nb=ne/nholes;
      ne=ne-nb;
      nholes=nholes-1;
      while((nb--) > 0)
        buf[--j]=' ';
      }
    else buf[j]=buf[i];
    --i;
    --j;
    }
  }

/*
** center -- center a line by setting tival
*/
center(buf) char buf[]; {
  int i, j;
  j = -lmval - inval - sqval;
  if((i=((rmval-lmval-width(buf))/2)-inval-sqval) < j)
       tival = j;
  else tival = i;
  }

/*
** underl -- underline a line
*/
underl(buf, tbuf, size) char buf[], tbuf[]; int size; {
  char c, *ptr;
  int prefixed, i, j;
  if(*buf == 0) return;
  if(!ttymode) prefixed=NO;
  else prefixed=9;  /* neither YES nor NO */
  i=j=0;
  while((buf[i]!=NULL)&(j<(size-2))) {
    c=buf[i++];
    if(index(ulskips, c) < 0) {
      if(prefixed==NO) {
        prefixed=YES;
        ptr=ulpref;
        while((tbuf[j++]=*ptr++) != 1) ;
        }
      else if(ttymode) {
        tbuf[j++]='_';
        tbuf[j++]='\b';
        }
      }
    else {
      if(prefixed==YES) {
        prefixed=NO;
        ptr=ulsuff;
        while((tbuf[j++]=*ptr++) != 1) ;
        }
      if(cuact>0 && fill==NO && c==' ' && ttymode) c='_';
      }
    tbuf[j++]=c;
    }
  if(prefixed==YES) {
    ptr=ulsuff;
    while((tbuf[j++]=*ptr++) != 1) ;
    }
  tbuf[j]=NULL;
  scopy(tbuf, 0, buf, 0);
  }

/*
** bold -- boldface a line
*/
bold(buf, tbuf, size) char buf[], tbuf[]; int size; {
  char c, *ptr;
  int prefixed, i, j;
  if(*buf == 0) return;
  if(!ttymode) prefixed=NO;
  else prefixed=9;  /* neither YES nor NO */
  i=j=0;
  while((buf[i]!=NULL)&(j<(size-2))) {
    c=buf[i++];
    if(index(bfskips, c) < 0) {
      if(prefixed==NO) {
        prefixed=YES;
        ptr=bfpref;
        while((tbuf[j++]=*ptr++) != 1) ;
        if(bfstrikes > 2) {             /* extra heavy */
          ptr=dspref;
          while((tbuf[j++]=*ptr++) != 1) ;
          }
        }
      else if(ttymode && (c > ' ') && (c < 127)) {
        int i;
        i = bfstrikes;
        while (--i) {
          tbuf[j++]=c;
          tbuf[j++]='\b';
          }
        }
      }
    else {
      if(prefixed==YES) {
        prefixed=NO;
        ptr=bfsuff;
        while((tbuf[j++]=*ptr++) != 1) ;
        if(bfstrikes > 2) {             /* extra heavy */
          ptr=dssuff;
          while((tbuf[j++]=*ptr++) != 1) ;
          }
        }
      }
    tbuf[j++]=c;
    }
  if(prefixed==YES) {
    ptr=bfsuff;
    while((tbuf[j++]=*ptr++) != 1) ;
    if(bfstrikes > 2) {                 /* extra heavy */
      ptr=dssuff;
      while((tbuf[j++]=*ptr++) != 1) ;
      }
    }
  tbuf[j]=NULL;
  scopy(tbuf, 0, buf, 0);
  }

/*
** italic -- italicize a line
*/
italic(buf, tbuf, size) char buf[], tbuf[]; int size; {
  char *ptr;
  int i, j;
  if(*buf == 0) return;
  if(ttymode) return;
  i=j=0;
  ptr=itpref;
  while((tbuf[j++]=*ptr++) != 1) ;
  ptr=bfpref;
  while((tbuf[j++]=*ptr++) != 1) ;
  while((buf[i]!=NULL)&(j<(size-2)))  tbuf[j++]=buf[i++];
  ptr=itsuff;
  while((tbuf[j++]=*ptr++) != 1) ;
  ptr=bfsuff;
  while((tbuf[j++]=*ptr++) != 1) ;
  tbuf[j]=NULL;
  scopy(tbuf, 0, buf, 0);
  }

/*
** double -- double-wide a line (nontty mode only)
*/
double(buf, tbuf, size) char buf[], tbuf[]; int size; {
  char *ptr;
  int space, i, j;
  if(!*buf || ttymode) return;
  space = YES;
  i = j = 0;
  while(buf[i] && j < (size-3)) {
    if(isspace(buf[i])) {
      if(!space) {
        ptr=dwsuff;
        while((tbuf[j++]=*ptr++) != 1) ;
        }
      space = YES;
      }
    else {
      if(space) {
        ptr=dwpref;
        while((tbuf[j++]=*ptr++) != 1) ;
        }
      space = NO;
      }
    tbuf[j++]=buf[i++];
    }
  if(!space) {
    ptr=dwsuff;
    while((tbuf[j++]=*ptr++) != 1) ;
    }
  tbuf[j]=NULL;
  scopy(tbuf, 0, buf, 0);
  }

/*
** supersub -- process super- & sub-scripts
*/
supersub(buf, tbuf, size) char buf[], tbuf[]; int size; {
  char *pref, *suff, *ptr;
  int i, j;
  if(!*buf) return;
  i = j = suff = 0;
  while(buf[i] && j < (size-3)) {
    if(isspace(buf[i]) && suff) {
      while((tbuf[j++] = *suff++) != 1) ;
      suff = 0;
      }
    if(buf[i] == SUBCHAR && buf[i+1] == SUBCHAR) {
      if(!ttymode) {
        if(suff) {ptr = suff;   suff = 0;}
        else     {ptr = sbpref; suff = sbsuff;}
        while((tbuf[j++] = *ptr++) != 1) ;
        }
      i += 2;
      }
    else if(buf[i] == SUPCHAR && buf[i+1] == SUPCHAR) {
      if(!ttymode) {
        if(suff) {ptr = suff;   suff = 0;}
        else     {ptr = sppref; suff = spsuff;}
        while((tbuf[j++] = *ptr++) != 1) ;
        }
      i += 2;
      }
    else tbuf[j++] = buf[i++];
    }
  if(suff) while((tbuf[j++] = *suff++) != 1) ;
  tbuf[j] = NULL;
  scopy(tbuf, 0, buf, 0);
  }

/*
** doargs -- process command line arguments
*/
doargs(argc, argv) int argc, *argv; {
  char arg[MAXFN], cp;
  int i, err;
  show=err=ttymode=itsub=ulsub=NO;
  pause=ready=pbreak=pheader=pfooter=YES;
  bfstrikes=2;
  pospec=mrgfd=0;
  mrgbuf[0]=NULL;
  begin=bcopy=1;
  end=ecopy=HUGE;
  i=0;
  while(getarg(++i, arg, MAXFN, argc, argv)!=EOF) {
    if(arg[0]=='-') {
      for(cp=arg+1; *cp; ++cp) *cp = toupper(*cp);
      if(strcmp(arg+1, "NP") == 0) {      /* no pause */
        pause=NO;
        continue;
        }
      if(strcmp(arg+1, "NR") == 0) {      /* no ready */
        ready=NO;
        continue;
        }
      if(strcmp(arg+1, "NF") == 0) {      /* no fill */
        fillset = YES;
        continue;
        }
      if(strcmp(arg+1, "NJ") == 0) {      /* no justify */
        justset = YES;
        continue;
        }
      if(strcmp(arg+1, "NPB") == 0) {     /* no page breaks */
        pbreak=NO;
        continue;
        } 
      if(strcmp(arg+1, "NPH") == 0) {     /* no page headers */
        pheader=NO;
        continue;
        }
      if(strcmp(arg+1, "NPF") == 0) {     /* no page footers */
        pfooter=NO;
        continue;
        }
      if(strcmp(arg+1, "A") == 0) {       /* ascii mode */
        ascii = YES;
        continue;
        }
      if(strcmp(arg+1, "S") == 0) {       /* show */
        show = YES;
        continue;
        }
      if(strcmp(arg+1, "T") == 0) {       /* tty mode */
        ttymode=YES;
        continue;
        }
      if(strcmp(arg+1, "I") == 0) {       /* .it for .ul */
        itsub=YES;
        continue;
        }
      if(strcmp(arg+1, "U") == 0) {       /* .ul for .it */
        ulsub=YES;
        continue;
        }
      if(strncmp(arg+1, "LM", 2) == 0) {  /* left margin */
        if(utoi(arg+3, &lmval) > 0) {
          --lmval;
          lmset = YES; 
          continue;
          }
        }
      if(strncmp(arg+1, "RM", 2) == 0) {  /* right margin */
        if(utoi(arg+3, &rmval) > 0) {
          rmset = YES; 
          continue;
          }
        }
      if(strncmp(arg+1, "BP", 2) == 0) {  /* begin page # */
        if(utoi(arg+3, &begin) > 0)
          continue;
        }
      if(strncmp(arg+1, "BC", 2) == 0) {  /* begin copy # */
        if(utoi(arg+3, &bcopy) > 0)
          continue;
        }
      if(strncmp(arg+1, "BS", 2) == 0) {  /* boldface strikes */
        if(utoi(arg+3, &bfstrikes) > 0)
          continue;
        }
      if(strncmp(arg+1, "EP", 2) == 0) {  /* end page # */
        if(utoi(arg+3, &end) > 0)
          continue;
        }
      if(strncmp(arg+1, "EC", 2) == 0) {  /* end copy # */
        if(utoi(arg+3, &ecopy) > 0)
          continue;
        }
      if(strncmp(arg+1, "PO", 2) == 0) {  /* page offset */
        if(utoi(arg+3, &pospec) > 0)
          continue;
        }
      err=YES;
      }
    else  {
      if((mrgfd=fopen(arg, "r"))==NULL) cant(arg);
      continue;
      }
    err=YES;
    }
  if(err==YES) {
    fputs("usage: FMT [mergefile]\n",stderr);
    fputs("    [-BC#] [-EC#] [-BP#] [-EP#] [-LM#] [-RM#] [-PO#]\n",stderr);
    fputs("    [-NF] [-NJ] [-NPB] [-NPF] [-NPH] [-NP] [-NR]\n",stderr);
    fputs("    [-T] [-I] [-U] [-S] [-BS#]\n",stderr);
    abort(7);
    }
  }

/*
** merge -- merge fields from mrgbuf into inbuf
*/
merge() {
  int i, j, k, l, m, merged;
  if(mrgbuf[0]==NULL) return YES;
  scopy(inbuf, 0, ttlbuf, 0);
  merged=NO;
  i=j=0;
  while(inbuf[i]=ttlbuf[j++]) {
    if(inbuf[i++]==mrg) {
      if((l=utoi(ttlbuf+j, &k)) < 1) continue;
      if(ttlbuf[j+l]!=mrg) continue;
      --i;
      j=j+l+1;
      m=0;
      while(--k) {
        while(mrgbuf[m]) {
          if(mrgbuf[m++]==mrg) break;
          }
        }
      while((mrgbuf[m]!=NULL)&(mrgbuf[m]!=mrg)) {
        inbuf[i++]=mrgbuf[m++];
        if(i >= (INSIZE-1)) break;
        }
      merged=YES;
      }
    }
  if(merged==NO) return YES;
  i = -1;
  while(inbuf[++i]) {
    if((inbuf[i]!=' ')&(inbuf[i]!='\t')) return YES;
    }
  return NO;
  }

