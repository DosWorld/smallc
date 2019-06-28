
/*
** cant.c -- abort with "name: can't open" message
*/
cant(str) char *str; {
  fputs(str, stderr);
  fputs(": can't open\n", stderr);
  abort(7);
  }
