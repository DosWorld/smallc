/*
** fnt.c -- choose FX printer font
*/
#include <stdio.h>
#include "tools.h"
#define NOCCARGC
char
   condensed[]={27, 15, 0},
  xcondensed[]={    18, 0},
   double[]={27, 'G', 0},
  xdouble[]={27, 'H', 0},
   elite[]={27, 'M', 0},
  xelite[]={27, 'P', 0},
   emphasized[]={27, 'E', 0},
  xemphasized[]={27, 'F', 0},
   enlarged[]={27, 'W', 49, 0},
  xenlarged[]={27, 'W', 48, 0},
   italics[]={27, '4', 0},
  xitalics[]={27, '5', 0},
   pica[]={27, 'P', 0},
  xpica[]={27, 'M', 0},
   subscript[]={27, 'S', 49, 0},
  xsubscript[]={27, 'T', 0},
   superscript[]={27, 'S', 48, 0},
  xsuperscript[]={27, 'T', 0},
   proportional[]={27, 'p', 49, 0},
  xproportional[]={27, 'p', 48, 0};
char str[15], *ptr="prn";
int i, fd;
main(argc, argv) int argc, *argv; {
  if(getarg(1, str, 15, argc, argv) != EOF) ptr = str;
  if(*str == '-') {
    fputs("usage: FNT [device]", stderr);
    abort(7);
    }
  if((fd=fopen(ptr, "w")) == 0) cant(ptr);
  do {
    fputs(CLEAR, stdout);
    fputs("    Select Epson FX Option (RETURN to exit)\n\n", stdout);
    fputs("set  clear   mode\n\n", stdout);
    fputs(" 1     2     condensed    \n", stdout);
    fputs(" 3     4     double strike\n", stdout);
    fputs(" 5     6     elite\n", stdout);
    fputs(" 7     8     emphasized   \n", stdout);
    fputs(" 9    10     enlarged\n", stdout);
    fputs("11    12     italics\n", stdout);
    fputs("13    14     pica\n", stdout);
    fputs("15    16     subscript\n", stdout);
    fputs("17    18     superscript\n", stdout);
    fputs("19    20     proportional\n", stdout);
    fputs("\nselect... ", stdout);
    fgets(str, 10, stdin);
    ptr = str;
    while(*ptr) {
      if(*ptr == '\n') *ptr=NULL;
      ++ptr;
      }
    if(utoi(str, &i)) {
      switch(i) {
        case  1: {fputs( condensed, fd); break;}
        case  2: {fputs(xcondensed, fd); break;}
        case  3: {fputs( double, fd); break;}
        case  4: {fputs(xdouble, fd); break;}
        case  5: {fputs( elite, fd); break;}
        case  6: {fputs(xelite, fd); break;}
        case  7: {fputs( emphasized, fd); break;}
        case  8: {fputs(xemphasized, fd); break;}
        case  9: {fputs( enlarged, fd); break;}
        case 10: {fputs(xenlarged, fd); break;}
        case 11: {fputs( italics, fd); break;}
        case 12: {fputs(xitalics, fd); break;}
        case 13: {fputs( pica, fd); break;}
        case 14: {fputs(xpica, fd); break;}
        case 15: {fputs( subscript, fd); break;}
        case 16: {fputs(xsubscript, fd); break;}
        case 17: {fputs( superscript, fd); break;}
        case 18: {fputs(xsuperscript, fd); break;}
        case 19: {fputs( proportional, fd); break;}
        case 20: {fputs(xproportional, fd); break;}
        }
      }
    } while (*str);
  }
#include "cant.c"

