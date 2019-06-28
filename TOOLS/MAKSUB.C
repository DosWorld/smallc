
/*
** maksub.c -- make substitution string in sub
*/
maksub(arg, from, delim, sub) char arg[], sub[], delim; int from; {
  int i, j;
  j=0;
  i=from;
  while((arg[i]!=delim)&(arg[i]!=NULL)) {
    if(arg[i]==DITTO) addset(DITCODE, sub, &j, MAXPAT);
    else addset(esc(arg, &i), sub, &j, MAXPAT);
    ++i;
    }
  if(arg[i]!=delim) return ERR; /**** missing delimiter ****/
  if(addset(NULL, sub, &j, MAXPAT)==NO) return ERR; /** no room **/
  return i;
  }
T], sub[MAXPAT];
char arg[MAXARG];