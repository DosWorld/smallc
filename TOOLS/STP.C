/*
** stp.c -- setup a device configuration
*/
#include <stdio.h>
#define LINESZ   133		/* maximum input line size */
#define FENCE    '|'		/* comment prefix */
#define QUOTE    '"'		/* quote for strings */
char ln[LINESZ], *lnptr;
int view, debug, error;
main(argc, argv) int argc, *argv; {
  int i;
  view = debug = NO;
  i = 0;
  while(getarg(++i, ln, 15, argc, argv) != EOF) {
    if(*ln == '-') {
      if(tolower(ln[1]) == 'v') {	/* view activity */
        view = YES;
        continue;
        }
      if(tolower(ln[1]) == 'd') {	/* debug mode - only show output */
        debug = YES;
        continue;
        }
      break;
      }
    fputs("usage: STP [-V] [-D]", stderr);
    abort(7);
    }
  while(fgets(ln, LINESZ, stdin)) {
    lnptr = ln;
    error = NO;
    while(*lnptr && *lnptr != FENCE && *lnptr != '\n') {
      poll(YES);
      if(*lnptr == QUOTE) dostring(); else dohex();
      while(isspace(*++lnptr)) ;
      }
    if(debug) fputs("\n", stderr);
    if(view || error) fputs(ln, stderr);
    }
  }

dostring() {
  ++lnptr;
  while(*lnptr >= ' ') {
    if(*lnptr == QUOTE) {
      ++lnptr;
      if(*lnptr != QUOTE) return;
      }
    send(lnptr++);
    }
  fputs("\7\n- unterminated string in following line\n", stderr);
  error = YES;
  }

dohex() {
  int i, hex;
  char ch;
  i = xtoi(lnptr, &hex);
  if(i < 1 || i > 2 || !isspace(lnptr[i])) i = 0;
  while(!isspace(*lnptr)) ++lnptr;
  if(i == 0) {
    fputs("\7\n- bad hex byte in following line\n", stderr);
    error = YES;
    return;
    }
  ch = hex;
  send(&ch);
  }

send(ptr) char *ptr; {
  if(debug) {
    int i;
    char str[3];
    i = *ptr & 255;
    itox(i, str, 3);
    fputs(str, stderr);
    fputs(" ", stderr);
    }
  else write(stdout, ptr, 1);
  }

