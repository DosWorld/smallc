/*
** fmt2.c -- text formatter part 2
*/

/*
** getval -- evaluate optional numeric argument
*/
getval(buf, argtyp) char buf[]; int *argtyp; {
  int i;
  i=0;
  while((buf[i]!=' ')&(buf[i]!='\t')&(buf[i]!=NULL)) ++i;
  skipbl(buf, &i);
  *argtyp=buf[i];
  if((*argtyp=='+')|(*argtyp=='-')) ++i;
  if(utoi(buf+i, &i) < 0) {
    fputs("\7error: ", stderr);
    fputs(inbuf, stderr);
    fputc('\n', stderr);
    return 0;
    }
  return i;
  }

/*
** skipbl -- skip blanks and tabs
*/
skipbl(lin, i) char lin[]; int *i; {
  while((lin[*i]==' ')|(lin[*i]=='\t')) *i = *i + 1;
  }

/*
** setvalue -- set parameter and check range
*/
setvalue(param, val, argtyp, defval, minval, maxval)
  int *param, val, argtyp, defval, minval, maxval; {
  if(argtyp==NULL) *param=defval;
  else if(argtyp=='+') *param=*param+val;
  else if(argtyp=='-') *param=*param-val;
  else *param=val;
  if(*param > maxval) *param=maxval;
  if(*param < minval) *param=minval;
  }

/*
** text -- process text lines
*/
text(inbuf) char inbuf[]; {
  int i;
  char c1, c2;
  if((inbuf[0]==' ')|(inbuf[0]==NULL)) leadbl(inbuf);
  if(bfval > 0) {
    bold(inbuf, wrdbuf, INSIZE);
    --bfval;
    }
  dwact = NO;
  if(dwval > 0) {
    if(!ttymode) {
      dwact = YES;
      double(inbuf, wrdbuf, INSIZE);
      }
    --dwval;
    }
  if(ulval > 0) {
    if(itsub) italic(inbuf, wrdbuf, INSIZE);
    else      underl(inbuf, wrdbuf, INSIZE);
    --ulval;
    }
  if(itval > 0) {
    if(ulsub) underl(inbuf, wrdbuf, INSIZE);
    else      italic(inbuf, wrdbuf, INSIZE);
    --itval;
    }
  supersub(inbuf, wrdbuf, INSIZE);
  if(ceval > 0) {
    center(inbuf);
    put(inbuf);
    --ceval;
    }
  else if(inbuf[0]==NULL) put(inbuf);
  else if(fill==NO) put(inbuf);
  else {
    i=0;
    wrdbuf[0]=' ';
    while(getwrd(inbuf, &i, wrdbuf+1) > 0) {
      if((sentend==YES)&(isupper(wrdbuf[1])==YES)&(wrdbuf[2]!='.'))
        putwrd(wrdbuf);  /** leading space **/
      else putwrd(wrdbuf+1);
      if(inbuf[i-1]=='"') {
        c1=inbuf[i-3];
        c2=inbuf[i-2];
        }
      else {
        c1=inbuf[i-2];
        c2=inbuf[i-1];
        }
      if((inbuf[i]!='\t')&(isupper(c1)==NO)&
         ((c2=='.')|(c2=='?')|(c2=='!'))) sentend=YES;
      else sentend=NO;
      }
    }
  if(ulval==0) cuact=0;
  }


