
/*
** printf(controlstring, arg, arg, ...) -- formatted print
**        operates as described by Kernighan & Ritchie
**        only d, x, c, s, and u specs are supported.
*/
printf(argc) int argc; {
  int i, width, prec, preclen, len, *nxtarg;
  char *ctl, *cx, c, right, str[7], *sptr, pad;
  i = CCARGC();   /* fetch arg count from A reg first */
  nxtarg = &argc + i - 1;
  ctl = *nxtarg;
  while(c=*ctl++) {
    if(c!='%') {cout(c, stdout); continue;}
    if(*ctl=='%') {cout(*ctl++, stdout); continue;}
    cx=ctl;
    if(*cx=='-') {right=0; ++cx;} else right=1;
    if(*cx=='0') {pad='0'; ++cx;} else pad=' ';
    if((i=utoi(cx, &width)) >= 0) cx=cx+i; else continue;
    if(*cx=='.') {
      if((preclen=utoi(++cx, &prec)) >= 0) cx=cx+preclen;
      else continue;
      }
    else preclen=0;
    sptr=str; c=*cx++; i=*(--nxtarg);
    if(c=='d') itod(i, str, 7);
    else if(c=='x') itox(i, str, 7);
    else if(c=='c') {str[0]=i; str[1]=NULL;}
    else if(c=='s') sptr=i;
    else if(c=='u') itou(i, str, 7);
    else continue;
    ctl=cx; /* accept conversion spec */
    if(c!='s') while(*sptr==' ') ++sptr;
    len=-1; while(sptr[++len]); /* get length */
    if((c=='s')&(len>prec)&(preclen>0)) len=prec;
    if(right) while(((width--)-len)>0) cout(pad, stdout);
    while(len) {cout(*sptr++, stdout); --len; --width;}
    while(((width--)-len)>0) cout(pad, stdout);
    }
  }
                                                                                                         