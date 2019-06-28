/*
** trim -- trim \n from str & return its length
*/
trim(str) char *str; {
  char *s;
  s = str - 1;
  while(*++s) ;  /* prefix ++ is faster */
  if((*--s == '\n') && (s >= str)) *s = NULL; else ++s;
  return (s - str);
  }

