
/*
** out.c -- output to fd checking for errors
*/
cout(c, fd) char c; int fd; {
  if(fputc(c, fd)==EOF) xout();
  }

sout(string, fd) char *string; int fd; {
  if(fputs(string, fd)==EOF) xout();
  }

lout(line, fd) char *line; int fd; {
  sout(line, fd);
  cout('\n', fd);
  }

xout() {
  fputs("output error\n", stderr);
  abort(7);
  }
                                                                                                                                                     