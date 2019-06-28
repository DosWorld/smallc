
/*
** page.c -- tell stderr of new page and possibly pause
*/
page(pageno, pause, fd) int pageno, pause, fd; {
  int i;
  char str[6];
  str[5]=NULL;
  itod(pageno, str, 5);
  i=0;
  while(str[i]==' ') ++i;
  if((pause==YES)&&(pageno>1)&&isatty(fd)) {
    fputs("\7set page ", stderr);
    fputs(str+i, stderr);
    fputs("... ", stderr);
    return fgetc(stderr);
    }
  else {
    fputs("page ", stderr);
    fputs(str+i, stderr);
    fputc('\n', stderr);
    return NULL;
    }
  }
