
/*
** edt2.c -- edit part 2
*/

/*
** getnum -- convert one term to a line number
*/
getnum(pnum) int *pnum; {
  int stat;
  char *digits;
  digits="0123456789";
  stat=OK;
  if(index(digits, lin[i]) >= 0) {
    *pnum=ctoi(lin, &i);
    --i;  /** backup then bump at end **/
    }
  else if(lin[i]==CURLINE) *pnum=curln;
  else if(lin[i]==LASTLINE) *pnum=lastln;
  else if((lin[i]==SCAN)|(lin[i]==BACKSCAN)) {
    if(optpat()==ERR) stat=ERR;
    else if(lin[i]==SCAN) stat=ptscan(FORWARD, pnum);
    else stat=ptscan(BACKWARD, pnum);
    }
  else if((lin[i]==PLUS)|(lin[i]==MINUS)) --i;
  else stat=EOF;
  if(stat==OK) ++i;
  return (status=stat);
  }

/*
** optpat -- make pattern if specified at lin[i]
*/
optpat() {
  if(lin[i]==NULL) i = ERR;
  else if(lin[i+1]==NULL) i = ERR;
  else if(lin[i+1]==lin[i]) ++i;
  else i = makpat(lin, i+1, lin[i], pat);
  if(pat[0]==NULL) i = ERR;
  if(i==ERR) {
    pat[0]=NULL;
    return ERR;
    }
  return OK;
  }

/*
** ptscan -- scan for next occurrence of pattern
*/
ptscan(way, num) int way, *num; {
  *num=curln;
  while(YES) {
    if(poll(YES)==ESC) return (ERR);
    if(way==FORWARD) *num=nextln(*num);
    else *num=prevln(*num);
    if(match(buf+getind(*num)+TEXT, pat)==YES) return OK;
    if(*num==curln) break;
    }
  return ERR;
  }

/*
** ckglob -- if global prefix, mark lines to be affected
*/
ckglob() {
  int gflag, k, line;
  if((same(lin[i], GLOBAL)==NO)&(same(lin[i], EXCLUDE)==NO))
    return (status=EOF);
  if(same(lin[i], GLOBAL)) gflag=YES;
  else gflag=NO;
  ++i;
  if((optpat()==ERR)|(defalt(1, lastln)==ERR))
    return (status=ERR);
  ++i;
  line=line1;
  while(line<=line2) {
    if(poll(YES)==ESC) return (status=ERR);
    k=gettxt(line++);
    if(match(txt, pat)==gflag) buf[k+MARK]=YES;
    else buf[k+MARK]=NO;
    }
  line=nextln(line2);
  while(line!=line1) {
    if(poll(YES)==ESC) return (status=ERR);
    k=getind(line);
    buf[k+MARK]=NO;
    line=nextln(line);
    }
  return (status=OK);
  }

/*
** defalt -- set defaulted line numbers
*/
defalt(def1, def2) int def1, def2; {
  if(nlines==0) {
    line1=def1;
    line2=def2;
    }
  if((line1>line2)|(line1<=0)|(line2>lastln))
    return (status=ERR);
  return (status=OK);
  }

/*
** doglob -- do command at lin[i] on all marked lines
*/
doglob() {
  int count, istart, k, line;
  status=OK;
  count=0;
  line=line1;
  istart=i;
  while(YES) {
    if(poll(YES)==ESC) return (status=ERR);
    k=getind(line);
    if(buf[k+MARK]==YES) {
      buf[k+MARK]=NO;
      cursav=curln=line;
      i=istart;
      if(getlst()==OK) {
        if(docmd(YES)==OK) count=0;
        }
      }
    else {
      line=nextln(line);
      ++count;
      }
    if((count>lastln)|(status!=OK)) break;
    }
  return status;
  }

/*
** append -- append lines after ln
*/
append(ln, glob) int ln, glob; {
  int stat;
  if(glob==YES) return ERR;
  curln=ln;
  stat=NOSTATUS;
  while(stat==NOSTATUS) {
    fputc(' ', stderr);
    if(fgets(lin, MAXLINE, stdin)==NULL) stat=EOF;
    else {
      trim(lin);
      if((lin[0]==PERIOD)&(lin[1]==NULL)) stat=OK;
      else if(inject(lin)==ERR) stat=ERR;
      }
    }
  return stat;
  }

/*
** kill -- delete lines from through to
*/
kill(from, to) int from, to; {
  int k1, k2;
  if((from==1)&(to==lastln)) {
    setbuf();
    updtflag=NO;
    return (status=OK);
    }
  if(from<=0) return (status=ERR);
  k2=getind(nextln(to));
  k1=getind(prevln(from));
  /** leaves gotline & gotind below affected area **/
  lastln=lastln-(to-from+1);
  curln=prevln(from);
  relink(k1, k2, k1, k2);
  return (status=OK);
  }

/*
** ckp -- check for "p" after command
*/
ckp(lin, i, pflag) char lin[]; int i, *pflag; {
  if(same(lin[i], PRINT)) {
    *pflag=YES;
    ++i;
    }
  else *pflag=NO;
  if(view) *pflag=YES;
  if(lin[i]==NULL) status=OK;
  else status=ERR;
  return status;
  }

/*
** move -- move line1 through line2 after line3
*/
move(line3) int line3; {
  int k0, k1, k2, k3, k4, k5;
  if((line1<=0)|((line1<=line3)&(line3<=line2))) return ERR;
  k1=getind(line1);
  k2=getind(line2);
  k3=getind(nextln(line2));
  k0=getind(prevln(line1));
  /** leaves gotline & gotind below affected area **/
  relink(k0, k3, k0, k3);
  if(line3>line1) {
    curln=line3;
    line3=line3-(line2-line1+1);
    }
  else curln=line3+(line2-line1+1);
  k5=getind(nextln(line3));
  k4=getind(line3);
  /** leaves gotline & gotind below affected area **/
  relink(k4, k1, k2, k5);
  relink(k2, k5, k4, k1);
  return OK;
  }

/*
** getrhs -- get substitution string for "s" command
*/
getrhs(lin, i, sub, gflag) char lin[], sub[]; int *i, *gflag; {
  if(lin[*i]==NULL) return ERR;
  if(lin[*i+1]==NULL) return ERR;
  *i=maksub(lin, *i+1, lin[*i], sub);
  if(*i==ERR) return ERR;
  if(same(lin[*i+1], GLOBAL)) {
    *i = *i + 1;
    *gflag=YES;
    }
  else *gflag=NO;
  return OK;
  }

/*
** subst -- substitute "sub" for occurrences of pattern
*/
subst(sub, gflag) char sub[]; int gflag; {
  char new[MAXLINE];
  int j, k, lastn, line, n, subbed;
  if(line1<=0) return ERR;
  line=line1;
  while(line<=line2) {
    if(poll(YES)==ESC) return (ERR);
    j=0;
    subbed=NO;
    gettxt(line);
    lastn=-1;
    k=0;
    while(YES) {
      if((gflag==YES)|(subbed==NO)) n=amatch(txt, k, pat);
      else n=-1;
      if((n>=0)&(lastn!=n)) {    /** replace matched text **/
        subbed=YES;
        catsub(txt, k, n, sub, new, &j, MAXLINE);
        lastn=n;
        }
      if(txt[k]==NULL) break;
      if((n==-1)|(n==k)) {       /** no match or null match **/
        addset(txt[k], new, &j, MAXLINE);
        ++k;
        }
      else k=n;                  /** skip matched text **/
      }
    if(subbed==YES) {
      if(addset(NULL, new, &j, MAXLINE)==NO) return ERR;
      curln=prevln(line);
      if(inject(new)==ERR) {
        curln=line;
        return ERR;
        }
      kill(curln+1, curln+1);
      }
    ++line;
    }
  return OK;
  }

/*
** getfn -- get file name from lin[i]
*/
getfn(lin, i, file, max) char lin[], file[]; int i, max; {
  int j, k, stat;
  stat=ERR;
  if(lin[i+1]==BLANK) {
    j=i+2;  /** get new file name **/
    skipbl(lin, &j);
    k=0;
    while(file[k++]=lin[j++]) if(--max < 1) break;
    if((k > 1)&(max > 0)) stat=OK;
    }
  else if((lin[i+1]==NULL)&(savfil[0]!=NULL)) {
    scopy(savfil, 0, file, 0);
    stat=OK;
    }
  if((stat==OK)&(savfil[0]==NULL))
    scopy(file, 0, savfil, 0);  /** save if no old one **/
  return stat;
  }

/*
** dowrit -- write "from" through "to" into file
*/
dowrit(from, to, file) int from, to; char file[]; {
  char tmp[MAXFN], *ptr;
  int fd, line;

  strcpy(tmp, file);
  if((ptr = strchr(tmp, '.')) == 0) ptr = tmp + strlen(tmp);
  strcpy(ptr, ".$$$");
  rename(file, tmp);

  if((fd=fopen(file, "w"))==NULL) return ERR;
  line=from;
  while(line<=to) {
    fputs(buf+getind(line++)+TEXT, fd);
    if(fputc('\n', fd)==EOF) {
      fputs("write ", stderr);
      fclose(fd);
      return ERR;
      }
    }
  if(fclose(fd)) return ERR;
  unlink(tmp);
  updtflag=NO;
  return OK;
  }

/*
** doprnt -- print lines from through to
*/
doprnt(from, to, glob) int from, to, glob; {
  int j, k;
  char pref;
  if(from<=0) return OK;
  if(same(lin[i], PRINT)) {
    ++i;
    if((lin[i]>='0')&(lin[i]<='9')) {
      if(lin[i+1]==NULL) context=lin[i]-'0';
      else return ERR;
      }
    else if(lin[i]!=NULL) return ERR;
    }
  if((from==to)&(glob==NO)) {
    j=from-context;
    k=to+context;
    }
  else {
    j=from;
    k=to;
    }
  if(j < 1) j=1;
  if(k > lastln) k=lastln;
  if(glob==NO) fputs(CLEAR, stdout);
  while(j <= k) {
    if(poll(YES)) {
      /* underflow handled in docmd() */
      curln = j - context;
      return OK;
      }
    if((j==to)&(glob==NO)) pref=CLFLAG;
    else pref=' ';
    gettxt(j++);
    fputc(pref, stdout);
    fputs(txt, stdout);
    fputc('\n', stdout);
    }
  curln=to;
  return OK;
  }

/*
** enter -- enter a new file
*/
enter(name) char name[]; {
  int err;
  err=doread(0, name);
  if(view) doprnt(1, 1, NO);
  curln=1;
  updtflag=NO;
  return err;
  }

