
/*
** tabpos.c -- return YES if col is a tab stop, else NO
*/
tabpos(col, tabs) int col; char tabs[]; {
  if(col>MAXLINE) return YES;
  else return tabs[col];
  }


];
  }
MLINE];
  int col, i;
  settab(tabs);
  col=1;
  while((c=getchaª())!=EOF) {
    if(c=='\t')
      while(1) {
        putchar(' ');
        ++col;
        if(tabpos(col, tabs)==YES) break;
        }
    else if(c=='\n') {
      putchar('\n');
      col=1;
      }
    else {
      putchar(c);
      ++col;
