/*
** strip -- Strip <, >, and " from str.
*/
strip(str) char *str; {
  char *cp;
  cp = str - 1;
  while(*++cp = *str++) {
    switch(*cp) {
      case '<':  case '>':  case '"':  --cp;
      }
    }
  }
