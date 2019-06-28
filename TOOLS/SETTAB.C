
/*
** settab.c -- search command line for tab list possibly
**             terminated with an interval (+n) and set up
**             tabs[MAXLINE+1]
*/
#define DEFTAB 8
settab(tabs, argc, argv) char tabs[]; int argc, *argv; {
  int i, j, k;
  char parm[9];
  i=0;
  while(++i<MAXLINE) tabs[i]=NO;
  if((getarg(1, parm, 9, argc, argv))==EOF) {
    i=0;
    while(++i<MAXLINE)
    if((i%DEFTAB)==1) tabs[i]=YES;
    return YES;
    }
  i=0;
  j=1;
  while(getarg(++i, parm, 9, argc, argv)!=EOF) {
    if(utoi(parm, &k)) {
      if(k<=MAXLINE) {
        tabs[k]=YES;
        j=k;
        }
      else {
        fputs("tab stop beyond max line length\n", stderr);
        return ERR;
        }
      }
    else if(parm[0]=='+') {
      if(utoi((parm+1), &k))
        while((j=j+k)<=MAXLINE) tabs[j]=YES;  
      }
    else return ERR;
    }
  return YES;
  }
