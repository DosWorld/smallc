
/*
** buf.c -- buffer manipulation functions (memory version)
*/

int gotline, gotind;
char *bufptr;

/*
** clrbuf -- initialize for new file (stub)
*/
clrbuf() {
  return;
  }

/*
** getind -- locate line index in buf
*/
getind(ln) int ln; {
  int j;
  if((j=ln-gotline) > 0)
    while(j--) getint(gotind+NEXT, &gotind, INTEGER);
  else if(j < 0)
    while(j++) getint(gotind+PREV, &gotind, INTEGER);
  gotline=ln;
  return gotind;
  }

/*
** getint -- obtain integer or long from buf at offset
*/
getint(offset, dest, length) int offset, length; char *dest; {
  bufptr=buf+offset;
  while(length--)  *dest++ = *bufptr++;
  }

/*
** putint -- place integer or long into buf at offset
*/
putint(offset, source, length) int offset, length; char *source; {
  bufptr=buf+offset;
  while(length--) *bufptr++ = *source++;
  }

/*
** gettxt -- locate text for line and make available
*/
gettxt(line) int line; {
  int i, j, k;
  i=(k=getind(line)) + TEXT;
  j=0;
  while(txt[j++]=buf[i++]);
  return k;
  }

/*
** input -- input text from fd into buf
*/
input(fd) int fd; {
  int k3;
  while(YES) {
    if(poll(YES)==ESC) return ERR;
    k3=lastbf;
    if(fgets(buf + (lastbf += TEXT), MAXLINE, fd)) {
      lastbf += trim(buf+lastbf) + 1;
      inlink(k3);
      if((lastbf+MAXLINE) > maxbuf) {
        fputs("memory overflow ", stderr);
        return ERR;
        }
      }
    else break;
    }
  lastbf -= TEXT;
  return OK;
  }

/*
** inject -- put text from lin after curln
*/
inject(lin) char lin[]; {
  int i, k3;
  if(*lin==NULL) {
    *lin='\n';
    *(lin+1)=NULL;
    }
  i=0;
  while(lin[i]!=NULL) {
    k3=lastbf;
    lastbf=lastbf+TEXT;
    while(lin[i]!=NULL) {
      if(lin[i]=='\n') {
        ++i;
        break;
        }
      addset(lin[i++], buf, &lastbf, maxbuf);
      }
    if(addset(NULL, buf, &lastbf, maxbuf)==NO) {
      fputs("memory overflow ", stderr);
      return ERR;
      }
    inlink(k3);
    }
  return OK;
  }

/*
** inlink -- link injected line
*/
inlink(k3) int k3; {
  int k1, k2;
  k1=getind(curln);
  /** leaves gotline & gotind before affected area **/
  getint(k1+NEXT, &k2, INTEGER);
  relink(k1, k3, k3, k2);
  relink(k3, k2, k1, k3);
  ++curln;
  ++lastln;
  }

/*
** relink -- rewrite two half links
*/
relink(a, x, y, b) int a, x, y, b; {
  putint(x+PREV, &a, INTEGER);
  putint(y+NEXT, &b, INTEGER);
  updtflag=YES;
  }

/*
** setbuf -- initialize line storage buffer
*/
setbuf() {
  relink(LINE0, LINE0, LINE0, LINE0);
  lastbf=LINE0+TEXT;
  addset(NULL, buf, &lastbf, maxbuf);
  gotline=curln=lastln=0;
  gotind=LINE0;
  }
