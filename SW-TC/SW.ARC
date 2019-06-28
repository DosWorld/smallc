>>> FILES.C 6659
#include "window.h"
#define CW         39     /* column width */
#define PERFILE (CW+2)    /* buffer space per file */

int
  _fdir,  /* file directory menu number */
  _fact,  /* file action menu number */
  _fnam;  /* filename window number */

char
 **_farr,      /* file array pointer */
  *_fbuf,      /* filename buffer */
  *_fdo[4],    /* file action menu items */
   _dta[128];  /* Disk Transfer Area */

char *_path();
char *_dt();
extern char *malloc();
extern char *ltou();

void  _align();
long  _getlong();

#if (sizeof(int *) > 2)
  extern unsigned      coreleft();
#else
  extern unsigned long coreleft();
#endif
extern void     free();

extern int    mfill();
extern void   mhide();
extern int    mopen();
extern int    mpick();
extern void   mreset();
extern void   mshow();
extern void   msort();
extern int    wgetf();
extern int    wgoto();
extern int    wopen();
extern void   wpop();
extern void   wpush();
extern void   wclean();
extern void   whide();
extern void   wputs();
extern int   _nextf();
extern int   _firstf();
extern int   _rename();
extern int   _delete();

/*
** Setup for Calls to files()
** ha: highlighted attribute
** na: normal attribute
*/
void
fileset(ha, na) int ha, na; {
  wpush(wactive(), NO);                 /* remember prior window */ 
  _fdir = mopen(0, 0, 10, 2, CW, ha, na, 1);
  _fact = mopen(0, 40, 1, 3,  8, ha, na, 0);
  _fnam = wopen(0, 0, 40, 61, YES);
  whide();
  wclean(ha);
  wputs("NEW NAME: ");
  _fdo[1] = " select ";
  _fdo[2] = " rename ";
  _fdo[3] = " delete ";
  wpop(NO);                             /* return to prior window */
  }

/*
** File Directory Menu
** Returns address of filename selected, else zero
** fs: file spec pointer
*/

char *
files(fs) char     *fs; {
  char *fp, *cp1, *cp2;
  int i, *ip, sz, pick, next, end;
  if(!_fnam)                              /* fileset() not called */
    return (0);
  if(coreleft() > 32767)                  /* use up to 32K */
       sz = 32767;
  else sz = (int) coreleft();
  if(sz < PERFILE * 2)                    /* not enough memory */
    return (0);
  _farr = (char **) malloc(sz);           /* get all you can */
  _fbuf = (char *) _farr + (end = sz/PERFILE) + 1;
  _farr[0] = fs;                          /* label menu with filespec */
  cp1 = _fbuf;
  next = 1;
  if(_firstf(fs, 0)) {                    /* get first file info into _dta */
    do {
      cp2 = _dta + 30;                    /* offset to filename */
      if(next == end) {
        _farr[next++] = "||| MORE |||";   /* too many files */
        break;
        }
      _farr[next++] = cp1;                /* save menu item address */
      *cp1++ = ' ';                       /* leading space */
      _align(&cp1, &cp2);                 /* fetch and align filename */
      ltou(_getlong(_dta+26), cp1, -8);   /* fetch file size */
      cp1 += 8;
      ip = (int *) (_dta+24);             /* date: yyyyyyym mmmddddd */
      cp1 = _dt(cp1, ' ',  (*ip >>  5) &  15, 2);
      cp1 = _dt(cp1, '-',   *ip        &  31, 2);
      cp1 = _dt(cp1, '-', ((*ip >>  9) & 127) + 1980, 4);
      ip = (int *) (_dta+22);             /* time: hhhhhmmm mmmsssss */
      cp1 = _dt(cp1, ' ',  (*ip >> 11) &  31, 2);
      cp1 = _dt(cp1, ':',  (*ip >>  5) &  63, 2);
      *cp1++ = 0;
      } while(_nextf());                  /* get next file info */
    msort(_farr, 1, next - 1, 1);         /* sort by filename */
    }
  mreset(_fdir);
  wpush(_fdir, YES);                      /* make file menu stick */
  while(fp = 0, pick = mpick(_fdir, _farr, next)) {
    cp1 = _path(_dta, fs);                /* copy path into _dta */
    cp2 = _farr[pick] + 1;                /* allow for leading space */
    if(*cp2) {                            /* not deleted? */
      fp = cp1;                           /* save for rename default */
      i = 0;
      while(i < 12) {                     /* fetch chosen filename */
        if(cp2[i] == ' ')  ++i;
        else *cp1++ = cp2[i++];
        }
      *cp1++ = 0;                         /* cap chosen path\file */
      } 
    else break;                           /* ignore path of deleted file */
    mreset(_fact);
    switch(mpick(_fact, _fdo, 4)) {
      case 3: if(_delete(_dta)) {
                cp2 = _farr[pick] + 1;    /* allow for leading space */
                *cp2 = 0;
                }
              continue;
      case 2: cp2 = _path(cp1, fs);       /* copy path for new name */
              wpush(_fnam, YES);          /* activate name window */
              wgoto(0, 10);               /* set cursor */
              wgetf(cp2, fp, 20);         /* get reply */
              wpop(YES);                  /* pop old window back */
              if(_fnok(cp2) && _rename(cp1, _dta)) {
                cp1 = _farr[pick] + 1;    /* allow for leading space */
                _align(&cp1, &cp2);
                }
              continue;
      case 1: fp = _dta;
              goto exit;
      }
    }

  exit:
  wpop(YES);                              /* hide this window, pop old one */
  free(_farr);                            /* return memory */
  return (fp);
  }

/*
** Copy Path Name
*/
char *
_path(dest, path) char *dest, *path; {
  char *cp;
  cp = dest;
  while(*path)
    switch(*dest++ = *path++) {
      case  ':':
      case '\\': cp = dest;
      }
  return (cp);
  }

/*
** Move Filename from SOUR to DEST Aligning the Extension
**        Does NOT terminate with a zero byte
** dest = pointer to destination character pointer
** sour = pointer to source character pointer
**        Pointers at dest and sour are updated
*/
void
_align(dest, sour) char **dest, **sour; {
  char *cp1, *cp2;
  int i;
  cp1 = *dest;
  cp2 = *sour;
  i = 1;
  while(*cp2) {                          /* fetch filename */
    if(i++ < 9 && *cp2 == '.')
         *cp1++ = ' ';
    else *cp1++ = *cp2++;
    }
  while(i++ < 13) *cp1++ = ' ';          /* align file size */
  *dest = cp1;
  *sour = cp2;
  }

/*
** Validate Length of Filename and Extension
** MS-DOS Validates the Characters
*/
int
_fnok(fn) char *fn; {
  int i, j;
  i = 0;  j = 9;
  while(++i <= j) {
    *fn = toupper(*fn);
    switch(*fn++) {
      case NULL:
        if(i > 1)
          return (YES);
      case  ' ':
        return (NO);
      case  '.':
        if(i == 1 || j == 4)
          return (NO);
        i = 0;  j = 4;
      }
    }
  return (NO);
  }

/*
** Format Parts of Date and Time
*/
char *
_dt(cp, ch, nbr, sz) char *cp, ch; int nbr, sz; {
  *cp++ = ch;
  itou(nbr, cp, -sz);
  if(*cp == ' ') *cp = '0';
  return (cp + sz);
  }

/*
** return long value at addr
*/
long
_getlong(addr) long *addr; {
  return (*addr);  
  }

>>> FILES2.ASM 6992
; 
;   _firstf() -- Get First Filename
;   _nextf()  -- Get Next  Filename
;   _rename() -- Rename a File
;   _delete() -- Delete a File
;
            EXTRN  __DTA:BYTE

_DATA       SEGMENT  WORD  PUBLIC  'DATA'
_DATA       ENDS

_BSS        SEGMENT  PARA  PUBLIC  'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF

            IFDEF    FARCODE
FILES2_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:FILES2_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF
;
;  Get First Filename
;  Return Error Status if Not Found
;  fs: file spec pointer
;  fa: file attributes
;
; _firstf(fs, fa)
;     BP  +4  +6  <-- NEARCODE NEARDATA  (small)
;     BP  +4  +8  <-- NEARCODE FARDATA   (compact)
;     BP  +6  +8  <-- FARCODE  NEARDATA  (medium)
;     BP  +6 +10  <-- FARCODE  FARDATA   (large)
;       
            PUBLIC  __FIRSTF

            IFDEF    NEARCODE
__FIRSTF    PROC     NEAR
            IFDEF    NEARDATA
FS          EQU      4
FA          EQU      6
            ENDIF
            IFDEF    FARDATA
FS          EQU      4
FA          EQU      8
            ENDIF
            ENDIF

            IFDEF    FARCODE
__FIRSTF    PROC     FAR
            IFDEF    NEARDATA
FS          EQU      6
FA          EQU      8
            ENDIF
            IFDEF    FARDATA
FS          EQU      6
FA          EQU     10
            ENDIF
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI
            push ds              ; save DS

            IFDEF    NEARDATA
            mov  ax,DGROUP       ; set up DS
            mov  ds,ax
            mov  dx,offset __dta ; file spec
            mov  ah,26           ; set DTA to DS:DX
            int  21h
            pop  ds              ; restore DS
            mov  dx,[bp+FS]      ; DS:DX -> file spec
            mov  cx,[bp+FA]      ; file attributes
            mov  ah,78           ; find first file
            int  21h
            ENDIF

            IFDEF    FARDATA
            mov  ax,DGROUP       ; set up DS
            mov  ds,ax
            mov  dx,offset __dta ; file spec
            mov  ah,26           ; set DTA to DS:DX
            int  21h
            mov  ax,[bp+FS+2]    ; set up DS
            mov  ds,ax
            mov  dx,[bp+FS]      ; DS:DX -> file spec
            mov  cx,[bp+FA]      ; file attributes
            mov  ah,78           ; find first file
            int  21h
            pop  ds              ; restore DS
            ENDIF

            mov  ax,1            ; assume success
            jnc  __firstf_exit
            xor  ax,ax           ; failure
__firstf_exit:
            POP  SI
            POP  DI
            POP  BP
            RET
__FIRSTF    ENDP

;
;  Get Next Filename
;  Return Error Status if Not Found
;
; _nextf()
;       
            PUBLIC  __NEXTF

            IFDEF    NEARCODE
__NEXTF     PROC     NEAR
            ENDIF

            IFDEF    FARCODE
__NEXTF     PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI
            mov  ah,79        ; find next file
            int  21h          ; call DOS
            mov  ax,1         ; assume success
            jnc  __nextf_exit
            xor  ax,ax        ; failure
__nextf_exit:
            POP  SI
            POP  DI
            POP  BP
            RET
__NEXTF     ENDP

;
;  Rename a File
;
; _rename(new, old)
;      BP  +4   +6  <-- NEARCODE NEARDATA  (small)
;      BP  +4   +8  <-- NEARCODE FARDATA   (compact)
;      BP  +6   +8  <-- FARCODE  NEARDATA  (medium)
;      BP  +6  +10  <-- FARCODE  FARDATA   (large)
;       
            PUBLIC  __RENAME

            IFDEF    NEARCODE
__RENAME    PROC     NEAR
            IFDEF    NEARDATA
NEW         EQU      4
OLD         EQU      6
            ENDIF
            IFDEF    FARDATA
NEW         EQU      4
OLD         EQU      8
            ENDIF
            ENDIF

            IFDEF    FARCODE
__RENAME    PROC     FAR
            IFDEF    NEARDATA
NEW         EQU      6
OLD         EQU      8
            ENDIF
            IFDEF    FARDATA
NEW         EQU      6
OLD         EQU     10
            ENDIF
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            IFDEF    NEARDATA
            mov  dx,[bp+OLD]     ; DS:DX -> old path\filename
            mov  di,[bp+NEW]     ; ES:DX -> new path\filename
            push ds
            pop  es
            mov  ah,86           ; rename a file
            int  21h             ; call DOS
            ENDIF

            IFDEF    FARDATA
            push ds
            mov  ax,[bp+OLD+2]   ; set DS
            mov  ds,ax
            mov  dx,[bp+OLD]     ; DS:DX -> old path\filename
            mov  ax,[bp+NEW+2]   ; set ES
            mov  es,ax
            mov  di,[bp+NEW]     ; ES:DX -> new path\filename
            mov  ah,86           ; rename a file
            int  21h             ; call DOS
            pop ds
            ENDIF

            mov  ax,1            ; assume success
            jnc  __rename_exit
            xor  ax,ax           ; failure
__rename_exit:
            POP  SI
            POP  DI
            POP  BP
            RET
__RENAME    ENDP

;
;  Delete a File
;
;  _delete(fn)
;      BP  +4  <-- NEARCODE NEARDATA  (small)
;      BP  +4  <-- NEARCODE FARDATA   (compact)
;      BP  +6  <-- FARCODE  NEARDATA  (medium)
;      BP  +6  <-- FARCODE  FARDATA   (large)
;       
            PUBLIC  __DELETE

            IFDEF    NEARCODE
__DELETE    PROC     NEAR
FN          EQU      4
            ENDIF

            IFDEF    FARCODE
__DELETE    PROC     FAR
FN          EQU      6
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            IFDEF    NEARDATA
            mov  dx,[bp+FN]      ; DS:DX -> path\filename
            mov  ah,65           ; delete a file
            int  21h             ; call DOS
            ENDIF

            IFDEF    FARDATA
            push ds
            mov  ax,[bp+FN+2]    ; set DS
            mov  ds, ax
            mov  dx,[bp+FN]      ; DS:DX -> path\filename
            mov  ah,65           ; delete a file
            int  21h             ; call DOS
            pop  ds
            ENDIF

            mov  ax,1            ; assume success
            jnc  __delete_exit
            xor  ax,ax           ; failure
__delete_exit:
            POP  SI
            POP  DI
            POP  BP
            RET
__DELETE    ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
FILES2_TEXT ENDS
            ENDIF

            END

>>> GETKEY.ASM 1519
;
; _getkey()
;
; Return next keyboard character
;
             PUBLIC  __GETKEY

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
__GETKEY     PROC     NEAR
             ENDIF

             IFDEF    FARCODE
_GETKEY_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_GETKEY_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
__GETKEY     PROC     FAR
             ENDIF

             mov  ah,0       ; sub-service = read keyboard
             int  16h        ; call bdos keyboard services
             or   al,al      ; special character?
             jnz  __get2     ; no
             mov  al,ah      ; yes, move it to al
             cmp  al,3       ; ctl-2 (simulated null)?
             jne  __get1     ; no
             xor  al,al      ; yes, report zero
             jmp  __get2
__get1:      
             add  al,113     ; offset to range 128-245
__get2:
             xor  ah,ah      ; zero ah
             RET
__GETKEY     ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_GETKEY_TEXT ENDS
             ENDIF

             END

>>> HITKEY.ASM 1274
;
; _hitkey()
;
;  Test for keyboard input
;
             PUBLIC  __HITKEY

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
__HITKEY     PROC     NEAR
             ENDIF

             IFDEF    FARCODE
_HITKEY_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_HITKEY_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
__HITKEY     PROC     FAR
             ENDIF

             mov  ah,1   ; sub-service = test keyboard
             int  16h    ; call bdos keyboard services
             jnz  __hit1
             xor  ax,ax  ; nothing there, return false
             jmp  __hit2 
__hit1:
             mov  ax,1   ; character ready, return true 
__hit2: 

             RET
__HITKEY     ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_HITKEY_TEXT ENDS
             ENDIF

             END

>>> ITOAB.C 582
/*
** itoab(n,s,b) - Convert "unsigned" n to characters in s using base b.
**                NOTE: This is a non-standard function.
*/

void _rev();
extern int strlen();

#pragma warn -pia

void
itoab(n, s, b) unsigned int n; char *s; int b; {
  char *ptr;
  ptr = s;
  do {
    if((*ptr = n % b) < 10) *ptr++ += '0';
    else                    *ptr++ += 55;
    } while(n /= b);
  *ptr = 0;
  _rev (s);
  }

/*
** reverse string in place 
*/
void
_rev(s) char *s; {
  char *j;
  int c;
  j = s + strlen(s) - 1;
  while(s < j) {
    c = *s;
    *s++ = *j;
    *j-- = c;
    }
  }

>>> ITOU.C 546
/*
** itou -- convert nbr to unsigned decimal string of width sz
**         right adjusted, blank filled; returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
char *
itou(nbr, str, sz)  unsigned int nbr;  char str[];  int sz;  {
  if(sz > 0) str[--sz] = 0;
  else if(sz < 0) sz = -sz;
  else while(str[sz] != 0) ++sz;
  while(sz) {
    str[--sz] = (nbr % 10) + '0';
    if((nbr /= 10) == 0) break;
    }
  while(sz) str[--sz] = ' ';
  return (str);
  }

>>> LTOU.C 685
/*
** ltou -- convert double precision nbr to unsigned decimal string
**         of width sz-1 right adjusted, blank filled; returns str
**
**    nbr = long integer
**    str = destination string address
**    sz  = size of string
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
char *
ltou(nbr, str, sz)  unsigned long nbr;  char *str;  int sz;  {
  if(sz > 0) str[--sz] = 0;
  else if(sz < 0) sz = -sz;
  else while(str[sz] != 0) ++sz;
  while(sz) {
    str[--sz] = ((char) (nbr % 10)) + '0';
    if((nbr /= 10) == 0) break;   /* finished? */
    }
  while(sz) str[--sz] = ' ';
  return (str);
  }
>>> MENU.C 8468
/*
** menu.c -- Menu Management Functions
*/
#include "window.h"

#define RESET0  -32768

char
 *_sh,  /* array of sash values */
 *_ha,  /* array of highlight attributes */
 *_na,  /* array of normal attributes */
 *_nr,  /* array of number of rows */
 *_nc,  /* array of number of columns */
 *_cw;  /* array of column widths */

int
 *_po;  /* array of previous offsets */

int    mopen();
void   mreset();
int    mfill();
void  _mhint();
int    mpick();
void  _mgoto();
void   mshow();
void   mhide();

extern int  _max;    /* maximum number of windows */
extern char _dseen;  /* default wopen() visibility status */

extern char *calloc();
extern int   wactive();
extern void  watt();
extern void  wchr();
extern void  wclean();
extern void  wcursor();
extern void  wframe();
extern int   wgoto();
extern void  whide();
extern int   wopen();
extern void  wpop();
extern void  wpush();
extern int   wseen();
extern void  wstr();

/* Define a Menu
**
**   tr    = top row on screen
**   lc    = left column on screen
**   nr    = number of usable rows
**   nc    = number of usable columns
**   cw    = column width
**   ha    = highlighting attribute (0 = use cursor)
**   na    = normal attribute of pane and sash
**   sh    = type of sash for window
** 
*/
int
mopen(tr, lc, nr, nc, cw, ha, na, sh)
  int tr, lc, nr, nc, cw, ha, na, sh; {
  int menu, adj;
  char dseen;
  if(!_sh) {                            /* first open? */
    if(!(_po  = (int *) calloc(_max, sizeof(int *))) ||
       !(_ha  = calloc(_max, sizeof(char))) ||
       !(_na  = calloc(_max, sizeof(char))) ||
       !(_cw  = calloc(_max, sizeof(char))) ||
       !(_nc  = calloc(_max, sizeof(char))) ||
       !(_nr  = calloc(_max, sizeof(char))) ||
       !(_sh  = calloc(_max, sizeof(char)))
      ) abort(1);                       /* insufficient memory */
    }
  if(nr < 1) nr = 1;
  if(nc < 1) nc = 1;
  if(sh) adj = 2;
  else   adj = 0;
  wpush(wactive(), NO);                 /* remember current window */
  dseen = _dseen;  _dseen = 0;          /* open as hidden window */
  menu = wopen(tr, tr+nr-1+adj, lc, lc+(nc*cw)-1+adj, YES);
  _dseen = dseen;                       /* restore _dseen */
  wclean(na);
  wframe(sh, na);
  if(ha) wcursor(0);
  else   wcursor(4);
  wpop(NO);                             /* return to prior window */
  _nr[menu] = nr;
  _nc[menu] = nc;
  _cw[menu] = cw;
  _na[menu] = na;
  _ha[menu] = ha;
  _sh[menu] = sh;
  return (menu);
  }

/*
** Reset Menu to its Beginning
*/
void
mreset(menu) int menu; {
  if(_po[menu] == 0)      _po[menu] = RESET0;  
  else if(_po[menu]  > 0) _po[menu] = -_po[menu];  
  }

/*
** Fill a Menu with Items
**   pa = string pointer array
**   al = array length
**   ao = array offset (first item to get)
**        zeroth item is the menu label
**        menus fill top down, left to right
**        returns the next ao to use
*/
int
mfill(menu, pa, al, ao) int menu, al, ao; char *pa[]; {
  char *cp1, *cp2;
  int row, col, or, oc, hint;
  wpush(menu, NO);                 /* activate menu */
  or = wrow();
  oc = wcol();
  _po[menu] = ao;                  /* remember for next _mpick() call */
  hint = _sh[menu];
  if(ao > 1) hint <<= 2;           /* tell _mhint() up */
  col = -1;  
  while(++col < _nc[menu]) {
    row = -1;  
    if(ao == 0 && col == 0) {
      if(pa[0] && _sh[menu]) {
        wframe(_sh[menu], _na[menu]);  /* reframe to clear old label */
        row = -2;
        }
      else ao = 1;
      }
    while(++row < _nr[menu]) {
      wgoto(row, col * _cw[menu]);
      if(ao < al) {
        cp1 = cp2 = pa[ao];
        while(*cp2 && *cp2 != '\n') ++cp2;
        if(row >= 0) wchr(' ', _cw[menu]);
        if((cp2 - cp1) > _cw[menu]) wstr(cp1, _cw[menu]);
        else                        wstr(cp1, cp2 - cp1);
        ++ao;
        }
      else wchr(' ', _cw[menu]);
      }
    }
  if(ao < al) hint <<= 4;           /* tell _mhint() down */
  _mhint(hint, _nr[menu], _nc[menu]);          /* give the hint */
  wgoto(or, oc);
  wpop(NO);                         /* return to prior window */
  return (ao);                      /* next offset to use */
  }

void
_mhint(hint, nr, nc) int hint, nr, nc; {
  char ch1, ch2;
  switch(hint) {
    case   0: return;              /* no sash */
    case   1: ch1 = ch2 = 'Ä';
              break;
    case   2: ch1 = ch2 = 'Í';
              break;
    case   4: ch2 = 'Ä'; goto up;
    case   8: ch2 = 'Í';
          up: if(nr == 1 && nc > 1) ch1 = 17;    /* left */
              else                  ch1 = 24;    /* up */
              break;
    case  16: ch1 = 'Ä'; goto down;
    case  32: ch1 = 'Í';
        down: if(nr == 1 && nc > 1) ch2 = 16;    /* right */
              else                  ch2 = 25;    /* down */
              break;
    case  64:
    case 128: if(nr == 1 && nc > 1) { ch1 = 17; ch2 = 16; }
              else                  { ch1 = 24; ch2 = 25; }
              break;
    }
  wgoto(-1, 1000);        wchr(ch2, 1);
  wgoto(-1, wcol() - 1);  wchr(ch1, 1);
  }

/*
** Pick a Menu Item
**   pa = string pointer array
**        zeroth item is the menu label
**        if pa == ((char **) 0) the menu is already filled
**   al = array length
**
**        menus fill top down, left to right
*/
int
mpick(menu, pa, al) int menu, al; char *pa[]; {
  int co,        /* current offset */
      no,        /* next offset */
      br,        /* bottom row */
      rc,        /* right column */
      lp,        /* offset to last page */
      base,      /* base offset */
      ndx,       /* index */
      seen;      /* originally visible? */
  br =  _nr[menu] - 1;
  rc = (_nc[menu] - 1) *_cw[menu];
  seen = wseen(menu);                               /* remember if seen */
  wpush(menu, YES);
  if(_po[menu] < 0) {
    if(pa || _po[menu] == RESET0)
         _po[menu] = 0;
    else _po[menu] = -_po[menu];
    _mgoto(menu, 0, 0);
    }
  if((co = no = _po[menu]) < 1) co = 1;
  lp = _nr[menu] * _nc[menu];                       /* temp use of lp */
  if(pa)        base = 1;
  else if(al > (base = co) + lp) al = base + lp;
  if((lp = al - lp) < base) lp = base;
  _mgoto(menu, wrow(), wcol());

  fill:
  if(pa) no = mfill(menu, pa, al, no);              /* fill the menu? */
  if(co < base) co = base;
  while(YES) {
    switch(poll(NO)) {
      case 185: if(wrow() >  0) {                    /* up */
                  _mgoto(menu, wrow() - 1, wcol());
                  break;
                  }
                no = co - 1;
                goto scroll;

      case 193: if(wrow() < br) {                   /* down */
                  _mgoto(menu, wrow() + 1, wcol());
                  break;
                  }
                no = co + 1;
                goto scroll;

      case 188: if(wcol() >  0) {                    /* left */
                  _mgoto(menu, wrow(), wcol() - _cw[menu]);
                  break;
                  }
                no = co - _nr[menu];
                goto scroll;

      case 190: if(wcol() < rc) {                    /* right */
                  _mgoto(menu, wrow(), wcol() + _cw[menu]);
                  break;
                  }
                no = co + _nr[menu];
                goto scroll;

      case 186: no = co - _nr[menu]*_nc[menu];       /* page up */
                goto scroll;

          home:
      case 184: no = base;                           /* home */
                _mgoto(menu, 0, 0);
                goto scroll;

           end:
      case 192: no = lp;                             /* end */
                _mgoto(menu, br, rc);

        scroll:
      case 194:                                      /* page down */
                if(no < base) goto home;
                if(no > lp)   goto end;
                co = no;
                goto fill;
      case  13:                                      /* return */
                ndx = wrow() + (((wcol() / _cw[menu])) * _nr[menu]) + co;
                if(ndx < al) goto exit;

      case  27: ndx = 0;                             /* escape */
                goto exit;
      }
    }

  exit:
  wpop(!seen);
  return (ndx);
  }

void
_mgoto(menu, row, col) int menu, row, col; {
  if(_ha[menu]) {
    watt(_na[menu], _cw[menu]);
    wgoto(row, col);
    watt(_ha[menu], _cw[menu]);
    }
  else wgoto(row, col);
  }

/*
** Show a Menu
*/
void
mshow(menu) int menu; {
  wpush(menu, YES);
  wpop(NO);
  }

/*
** Hide a Menu
*/
void
mhide(menu) int menu; {
  wpush(menu, NO);
  wpop(YES);
  }

>>> MSORT.C 1995
/*
** Sort Any Part of an Array of String Pointers
**    pa: Pointer Array Address
**    lo: subscript of lowest element to sort
**    hi: subscript of highest element to sort
** order: 1 = ascending, -1 = descending
*/

int _mcmp();
int  lexorder();

void
msort(pa, lo, hi, order) char *pa[]; int lo, hi, order; {
  char *tmp;
  int gap, i, j, jg;
  gap = (hi-lo+1) >> 1;
  while(gap > 0) {
    i = gap + lo;
    while(i <= hi) {
      j = i++ - gap;
      while(j >= lo) {
        jg = j + gap;
        if(_mcmp(pa[j], pa[jg], order) <= 0) break;
        tmp = pa[j]; pa[j] = pa[jg]; pa[jg] = tmp;   /* swap */
        j = j - gap;
        }
      }
    gap = gap >> 1;
    }
  }

int
_mcmp(p1, p2, order) char *p1, *p2; int order; {
  while(lexorder(*p1, *p2) == 0) {
    if(*p1 == 0) return (0);
    ++p1; ++p2;
    }
  if(lexorder(*p1, *p2) > 0)
    return (order);
  return (-order);
  }

/*
** lexorder(c1, c2)
**
** Return a negative, zero, or positive number if
** c1 is less than, equal to, or greater than c2,
** based on a lexicographical (dictionary order)
** colating sequence.
**
*/
char _lex[128] = {
     /**** NUL - / ****/
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47,
     /**** 0-9 ****/
      65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
     /**** : ; < = > ? @ ****/
      48, 49, 50, 51, 52, 53, 54,
     /**** A-Z ****/
      75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
      88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,
     /**** [ \ ] ^ _ ` ****/
      55, 56, 57, 58, 59, 60,
     /**** a-z ****/
      75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
      88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,
     /**** { | } ~ ****/
      61, 62, 63, 64,
     /**** DEL ****/
     127
     };
int
lexorder(c1, c2) char c1, c2; {
  return((int) (_lex[c1] - _lex[c2]));
  }


>>> MTEST.C 6087
/*
** mtest.c -- Menu Test Program
*/
#include <stdlib.h>
#include "sw.h"
#define   NO    0
#define  YES    1
#define  GET  255

int
  al = 21,      /* array length */
  ao,           /* array offset */
  pk,           /* menu pick */
  i, j, k,      /* miscellaneous variables */
  w_main,       /* main window number */
  w_prompt,     /* prompt window number */
  w_attrib,     /* attribute window number */
  mn,           /* menu number */
  tr =   3,     /* top row */
  lc =  40,     /* left column */
  nr =   3,     /* number of rows */
  nc =   3,     /* number of columns */
  cw =  12,     /* column width */
  ha = 112,     /* highlight attribute */
  na =   7,     /* normal attribute */
  sh =   1;     /* sash */

char
  *pa[100]={    /* pointer array for menu items */
     "MENU",
     " 1 coke",   " 2 tea",     " 3 milk",   " 4 coffee", " 5 juice",
     " 6 sprite", " 7 water",   " 8 cocoa",  " 9 punch",  "10 soda",
     "11 shake",  "12 burger",  "13 hotdog", "14 salad",  "15 potato",
     "16 steak",  "17 chicken", "18 fish",   "19 chips",  "20 ham"},

  *fn="",       /* file name pointer */
  fs[31]="*.*", /* file search specification */
  response[31]; /* keyboard response */

extern int _fdir;

main() {
  w_main = wopen(0, 23, 0, 79, NO);
  wclean(7);
  wframe(2, 7);
  wauto(NO, NO);
  fill();

  w_attrib = wopen(1, 18, 1, 33, YES);
  whide();
  wclean(7);
  wframe(1, 7);
  wauto(NO, NO);
  setatt();

  mn = mopen(tr, lc, nr, nc, cw, ha, na, sh);
  mshow(mn);

  w_prompt = wopen(24, 24, 0, 39, YES);
  wclean(7);
  wauto(NO, NO);

  while(YES) {
    refresh();
    switch(get_i("select: ")) { 
      case  1: mn = mopen(tr, lc, nr, nc, cw, ha, na, sh);
               continue;
      case  2: mfill(mn, pa, al, ao);      continue;
      case  3: pk = mpick(mn, pa, al);     continue;
      case  4: pk = mpick(mn,  0, al);     continue;
      case  5: mhide(mn);                  continue;
      case  6: mshow(mn);                  continue;
      case  7: msort(pa, 1, al-1,  1);     continue;
      case  8: msort(pa, 1, al-1, -1);     continue;
      case  9: mreset(mn);                 continue;
      case 10: if(!_fdir) fileset(ha, na);
               if(!(fn = files(fs))) fn = "";
               continue;
      }
    if(strncmp(response, "ao", 2) == 0) {
      ao = get_i("ao: ");
      continue;
      } 
    else if(strncmp(response, "tr", 2) == 0) {
      tr = get_i("tr: ");
      continue;
      } 
    else if(strncmp(response, "lc", 2) == 0) {
      lc = get_i("lc: ");
      continue;
      }
    else if(strncmp(response, "nr", 2) == 0) {
      nr = get_i("nr: ");
      continue;
      }
    else if(strncmp(response, "nc", 2) == 0) {
      nc = get_i("nc: ");
      continue;
      }
    else if(strncmp(response, "cw", 2) == 0) {
      cw = get_i("cw: ");
      continue;
      }
    else if(strncmp(response, "ha", 2) == 0) {
      attrib(&ha);
      continue;
      }
    else if(strncmp(response, "na", 2) == 0) {
      attrib(&na);
      continue;
      }
    else if(strncmp(response, "sh", 2) == 0) {
      sh =  get_i("sh: ");
      continue;
      }
    else if(strncmp(response, "al", 2) == 0) {
      al =  get_i("al: ");
      continue;
      }
    else if(strncmp(response, "fs", 2) == 0) {
           get_fs("fs: ");
      continue;
      }
    else if(strncmp(response, "q ", 1) == 0) {
      wpush(w_prompt, YES);
      wclean(7);
      wgoto(0, 0);
      exit(0);
      }
    }
  }

fill() {
  wgoto( 0, 0); wstr("al:      1 mopen(tr,lc,nr,nc,cw,ha,na,sh)", 41);
  wgoto( 1, 0); wstr("ao:      2 mfill(mn,pa,al,ao)            ", 41);  
  wgoto( 2, 0); wstr("cw:      3 mpick(mn,pa,al)               ", 41);
  wgoto( 3, 0); wstr("ha:      4 mpick(mn, 0,al)               ", 41);
  wgoto( 4, 0); wstr("lc:      5 mhide(mn)                     ", 41);
  wgoto( 5, 0); wstr("mn:      6 mshow(mn)                     ", 41);
  wgoto( 6, 0); wstr("na:      7 msort(pa, 1, al-1,  1)        ", 41);
  wgoto( 7, 0); wstr("nc:      8 msort(pa, 1, al-1, -1)        ", 41);
  wgoto( 8, 0); wstr("nr:      9 mreset(mn)                    ", 41);
  wgoto( 9, 0); wstr("pk:     10 files(fs, ha, na)             ", 41);
  wgoto(10, 0); wstr("sh:                                      ", 41);
  wgoto(11, 0); wstr("tr:                                      ", 41);
  wgoto(12, 0); wstr("(q)uit                                   ", 41);
  wgoto(20, 0); wstr("fs:                                      ", 41);
  wgoto(21, 0); wstr("fn:                                      ", 41);
  }

refresh() {
  wselect(w_main);
  wgoto( 0, 3); wprintf("%3d", al); 
  wgoto( 1, 3); wprintf("%3d", ao); 
  wgoto( 2, 3); wprintf("%3d", cw); 
  wgoto( 3, 3); wprintf("%3d", ha); 
  wgoto( 4, 3); wprintf("%3d", lc); 
  wgoto( 5, 3); wprintf("%3d", mn);
  wgoto( 6, 3); wprintf("%3d", na); 
  wgoto( 7, 3); wprintf("%3d", nc); 
  wgoto( 8, 3); wprintf("%3d", nr); 
  wgoto( 9, 3); wprintf("%3d", pk); 
  wgoto(10, 3); wprintf("%3d", sh); 
  wgoto(11, 3); wprintf("%3d", tr); 
  wgoto(20, 3); wchr(' ', 40);  wprintf("%s ", fs); 
  wgoto(21, 3); wchr(' ', 40);  wprintf("%s ", fn); 
  }

setatt() {
  for(i = 0; i < 16; ++i) {
    wgoto( i, 0);
    wstr("               ", 31);
    }
  for(i = 0; i < 16; ++i) {
    for(j = 0; j < 31; j += 2) {
      wgoto(i, j);
      watt(k++, 1);
      }
    }
  }

attrib(at) char *at; {
  wselect(w_attrib);
  wgoto((*at >> 4) & 15, (*at & 15) << 1);
  wshow();
  while(YES) {
    switch(poll(YES)) {
      case 185: wgo( -1,  0); break;
      case 193: wgo(  1,  0); break;
      case 188: wgo(  0, -2); break;
      case 190: wgo(  0,  2); break;
      case  13: *at = (wrow() << 4) + (wcol() >> 1);
                whide();
                return;
      }
    }
  }

get_fs(pr) char *pr; {
  prompt(pr, fs);
  }

get_i(pr) char *pr; {
  prompt(pr, response);
  return (atoi(response));
  }

get_c(pr) char *pr; {
  prompt(pr, response);
  return (*response);
  }

prompt(pr, buf) char *pr, *buf; {
  wselect(w_prompt);
  wclean(7);
  wgoto(0, 0);
  wputs(pr);    
  wgetf(buf, "", 30);
  }

>>> PAD.ASM 1555
; 
; _pad(buf, size, value)
;   BP  +4    +8    +10  <-- NEAR
;   BP  +6   +10    +12  <-- FAR
; 
; Pad a Far Buffer with a Value
;       
          PUBLIC  __PAD

_DATA     SEGMENT WORD PUBLIC 'DATA'
_DATA     ENDS

_BSS      SEGMENT WORD PUBLIC 'BSS'
_BSS      ENDS

          IFDEF    NEARCODE
BUFOFF    EQU      4
BUFSEG    EQU      6
SIZE      EQU      8
VALUE     EQU     10
_TEXT     SEGMENT  BYTE PUBLIC 'CODE'
DGROUP    GROUP    _DATA,_BSS
          ASSUME   DS:DGROUP
          ASSUME   CS:_TEXT
          IFDEF    NEARDATA
          ASSUME   SS:DGROUP
          ENDIF
__PAD     PROC     NEAR
          ENDIF

          IFDEF    FARCODE
BUFOFF    EQU      6
BUFSEG    EQU      8
SIZE      EQU     10
VALUE     EQU     12
_PAD_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP    GROUP    _DATA,_BSS
          ASSUME   DS:DGROUP
          ASSUME   CS:_PAD_TEXT
          IFDEF    NEARDATA
          ASSUME   SS:DGROUP
          ENDIF
__PAD     PROC     FAR
          ENDIF

          PUSH BP
          MOV  BP,SP
          PUSH DI
          mov  di,[bp+BUFOFF] ; di = buffer offset
          mov  ax,[bp+BUFSEG] ; es = buffer segment
          mov  es,ax
          mov  cx,[bp+SIZE]   ; size of buffer
          mov  al,[bp+VALUE]  ; value
          or   cx,cx
          jz   __pad_exit
          rep stosb           ; put value in buffer byte
__pad_exit:
          POP  DI
          POP  BP
          RET
__PAD     ENDP

          IFDEF    NEARCODE
_TEXT     ENDS
          ENDIF

          IFDEF    FARCODE
_PAD_TEXT ENDS
          ENDIF

          END

>>> POLL.C 449
#define ABORT    3    /* control-C */
#define PAUSE   19    /* control-S */

#pragma warn -pia

/*
** Poll for console input or interruption
*/
poll(pause) int pause; {
  int i;
  if(i = _hitkey())  i = _getkey();
  if(pause) {
    if(i == PAUSE) {
      i = _getkey();           /* wait for next character */
      if(i == ABORT) exit(7);  /* indicate abnormal exit */
      return (0);
      }
    if(i == ABORT) exit(7);
    }
  return (i);
  }

>>> SW.H 2927
/*
** sw.h -- Header File for Small-Windows Functions
**
**         Include in all Small-Windows programs.
*/
extern char     *files(char *);
extern void      fileset(int, int);

extern int       mfill(int, char *[], int, int);
extern void      mhide(int);
extern int       mopen(int, int, int, int, int, int, int, int);
extern int       mpick(int, char *[], int);
extern void      mreset(int);
extern void      mshow(int);
extern void      msort(char *[], int, int, int);

extern void      vatt(int, int, int, int, int, int);
extern void      vchr(char, int, int, int, int, int);
extern void      vchra(char, int, int, int, int, int, int);
extern void      vcursor(int, int);
extern void      vdrop(int, int, int, int, int, int);
extern void      vfast();
extern void      vgoto(int, int, int);
extern int       visat(int);
extern void      vlift(int, int, int, int, int, int);
extern int       vmode(int);
extern int       vpage(int);
extern int       vpoint(int, int, int, int, int, int, int);
extern void      vshow(char far *, int, int, int, int, int);
extern void      vslow();
extern void      vstow(char far *, int, int, int, int, int);
extern void      vstr(char *, int, int, int, int, int);
extern void      vstra(char *, int, int, int, int, int, int);

extern int       wactive();
extern void      wallow(int);
extern void      watt(int, int);
extern void      wauto(int, int);
extern void      wblank(int);
extern int       wchop(int);
extern void      wchr(char, int);
extern void      wchra(char, int, int);
extern void      wclean(int);
extern int       wcol();
extern int       wcsize();
extern void      wcursor(int);
extern void      wdrop(int);
extern void      wframe(int, int);
extern char      wgetc();
extern int       wgetf(char *, char *, int);
extern char     *wgets(char *, int);
extern int       wgo(int, int);
extern int       wgoto(int, int);
extern void      whide();
extern void      wlift(int);
extern int       wmode(int);
extern void      wmove(int, int);
extern void      wnl(int);
extern int       wok(int);
extern int       wopen(int, int, int, int, int);
extern int       wpage(int);
extern char far *wpoint();
extern void      wpop(int);
extern int       wprintf();
extern void      wpush(int, int);
extern char      wputc(char);
extern char      wputca(char, int);
extern void      wputs(char *);
extern void      wputsa(char *, int);
extern void      wsbuf(int, int);
extern int       wrow();
extern int       wscanf();
extern void      wsdis(int, void (*)());
extern int       wseen(int);
extern void      wselect(int);
extern void      wshow();
extern void      wstr(char *, int);
extern void      wstra(char *, int, int);
extern void      wsxow(void (*)(), char far *);

extern char     *itou(unsigned, char *, int);
extern void      itoab(unsigned, char *, int);
extern char     *ltou(unsigned long, char *, int);
extern int       poll(int);
extern int       utoi(char *, int *);

>>> UTOI.C 381
/*
** utoi -- convert unsigned decimal string to integer nbr
**          returns field size, else ERR on error
*/
int
utoi(decstr, nbr)  char *decstr;  int *nbr;  {
  int d, t;
  d = 0;
  *nbr = 0;
  while((*decstr >= '0') & (*decstr <= '9')) {
    t = *nbr; t = (10 * t) + (*decstr++ - '0');
    if ((t >= 0) & (*nbr < 0)) return (-1);
    d++; *nbr = t;
    }
  return (d);
  }

>>> VATT.ASM 2833
; 
; vatt(attr, len, row, col, mode, page)
;  BP    +4   +6   +8  +10   +12   +14   <-- NEARCODE
;  BP    +6   +8  +10  +12   +14   +16   <-- FARCODE
; 
; Display Attributes Only
;       
            PUBLIC  _VATT
            EXTRN   _VWAIT:BYTE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
ATTR        EQU      4
LEN         EQU      6
ROW         EQU      8
COL         EQU     10
MOD         EQU     12
PAG         EQU     14
            EXTRN   _VPOINT:NEAR
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VATT       PROC     NEAR
            ENDIF

            IFDEF    FARCODE
ATTR        EQU      6
LEN         EQU      8
ROW         EQU     10
COL         EQU     12
MOD         EQU     14
PAG         EQU     16
            EXTRN   _VPOINT:FAR
_VATT_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VATT_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VATT       PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            push word ptr [bp+PAG]
            push word ptr [bp+MOD]
            push word ptr [bp+COL]
            push word ptr [bp+ROW]
            call _vpoint      ; vpoint() -- point ES to page and calc offset
            add  sp,8

            mov  di,ax        ; di -> offset in display-page (es)
            mov  bl,[bp+ATTR] ; attr
            mov  cx,[bp+LEN]  ; length
            or   cx,cx
            jz   __att_exit

__att_next:
            inc  di           ; increment di past data
            mov  al,_vwait
            or   al,al
            jnz  __att_go
            mov  dx,03DAH     ; set port address in dx
__att_loop:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit high?
            jc   __att_loop   ; yes, try again
            cli               ; disable interrupts
__att_loop2:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit low?
            jnc  __att_loop2  ; no, try again
__att_go:
            mov  al,bl        ; fetch attr
            stosb             ; put attr on screen & incr di
            sti               ; reenable interrupts
            loop __att_next

__att_exit:
            POP  SI
            POP  DI
            POP  BP
            RET
_VATT       ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VATT_TEXT  ENDS
            ENDIF

            END

>>> VCHR.ASM 2708
;
; vchr(char, len, row, col, mode, page)
;  BP    +4   +6   +8  +10   +12   +14   <-- NEARCODE
;  BP    +6   +8  +10  +12   +14   +16   <-- FARCODE
;
; Display Characters Only
; 
           PUBLIC  _VCHR
           EXTRN   _VWAIT:BYTE

_DATA      SEGMENT WORD PUBLIC 'DATA'
_DATA      ENDS

_BSS       SEGMENT WORD PUBLIC 'BSS'
_BSS       ENDS

           IFDEF    NEARCODE
CHAR       EQU      4
LEN        EQU      6
ROW        EQU      8
COL        EQU     10
MOD        EQU     12
PAG        EQU     14
           EXTRN   _VPOINT:NEAR
_TEXT      SEGMENT  BYTE PUBLIC 'CODE'
DGROUP     GROUP    _DATA,_BSS
           ASSUME   DS:DGROUP
           ASSUME   CS:_TEXT
           IFDEF    NEARDATA
           ASSUME   SS:DGROUP
           ENDIF
_VCHR      PROC     NEAR
           ENDIF

           IFDEF    FARCODE
CHAR       EQU      6
LEN        EQU      8
ROW        EQU     10
COL        EQU     12
MOD        EQU     14
PAG        EQU     16
           EXTRN   _VPOINT:FAR
_VCHR_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP     GROUP    _DATA,_BSS
           ASSUME   DS:DGROUP
           ASSUME   CS:_VCHR_TEXT
           IFDEF    NEARDATA
           ASSUME   SS:DGROUP
           ENDIF
_VCHR      PROC     FAR
           ENDIF

           PUSH BP
           MOV  BP,SP
           PUSH DI

           push word ptr [bp+PAG]
           push word ptr [bp+MOD]
           push word ptr [bp+COL]
           push word ptr [bp+ROW]
           call _vpoint      ; vpoint() -- point ES to page and calc offset
           add  sp,8

           mov  di,ax        ; di -> offset in display-page (es)
           mov  bl,[bp+CHAR] ; char
           mov  cx,[bp+LEN]  ; length
           or   cx,cx
           jz   __chr_exit

__chr_next:
           mov  al,_vwait
           or   al,al
           jnz  __chr_go
           mov  dx,03DAH     ; set port address in dx
__chr_loop:
           in   al,dx        ; read CGA port 3DA
           rcr  al,1         ; horizontal retrace bit high?
           jc   __chr_loop   ; yes, try again
           cli               ; disable interrupts
__chr_loop2:
           in   al,dx        ; read CGA port 3DA
           rcr  al,1         ; horizontal retrace bit low?
           jnc  __chr_loop2  ; no, try again
__chr_go:
           mov  al,bl        ; fetch char
           stosb             ; put char on screen & incr di
           sti               ; reenable interrupts
           inc  di           ; increment di past attr
           loop __chr_next

__chr_exit:
           POP  DI
           POP  BP
           RET
_VCHR      ENDP

           IFDEF    NEARCODE 
_TEXT      ENDS
           ENDIF 

           IFDEF    FARCODE
_VCHR_TEXT ENDS 
           ENDIF 

           END

>>> VCHRA.ASM 2855
;
; vchra(chr, len, attr, row, col, mode, page)
;    BP  +4   +6   +8  +10   +12   +14   +16  <-- NEAR
;    BP  +6   +8  +10  +12   +14   +16   +18  <-- FAR
;
; Display Characters and Attributes
;
            PUBLIC  _VCHRA
            EXTRN   _VWAIT:BYTE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
CHAR        EQU      4
LEN         EQU      6
ATTR        EQU      8
ROW         EQU     10
COL         EQU     12
MOD         EQU     14
PAG         EQU     16
            EXTRN   _VPOINT:NEAR
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VCHRA      PROC     NEAR
            ENDIF

            IFDEF    FARCODE
CHAR        EQU      6
LEN         EQU      8
ATTR        EQU     10
ROW         EQU     12
COL         EQU     14
MOD         EQU     16
PAG         EQU     18
            EXTRN   _VPOINT:FAR
_VCHRA_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VCHRA_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VCHRA      PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI

            push word ptr [bp+PAG]
            push word ptr [bp+MOD]
            push word ptr [bp+COL]
            push word ptr [bp+ROW]
            call _vpoint      ; vpoint() -- point ES to page and calc offset
            add  sp,8

            mov  di,ax        ; di -> offset in display-page (es)
            mov  bl,[bp+CHAR] ; char
            mov  cx,[bp+LEN]  ; length
            mov  bh,[bp+ATTR] ; attr
            or   cx,cx
            jz   __chra_exit

__chra_next:
            mov  al,_vwait
            or   al,al
            jnz  __chra_go
            mov  dx,03DAH     ; set port address in dx
__chra_loop:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit high?
            jc   __chra_loop  ; yes, try again
            cli               ; disable interrupts
__chra_loop2:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit low?
            jnc  __chra_loop2 ; no, try again
__chra_go:
            mov  ax,bx        ; fetch char/attr
            stosw             ; put char/attr on screen & incr di by 2
            sti               ; reenable interrupts
            loop __chra_next

__chra_exit:
            POP  DI
            POP  BP
            RET
_VCHRA      ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VCHRA_TEXT ENDS
            ENDIF

            END

>>> VCURSOR.ASM 1543
;
; vcursor(top, bot)
;      BP  +4   +6  <-- NEAR
;      BP  +6   +8  <-- FAR
;
; Set Cursor Shape
;
              PUBLIC  _VCURSOR

_DATA         SEGMENT WORD PUBLIC 'DATA'
_DATA         ENDS

_BSS          SEGMENT WORD PUBLIC 'BSS'
_BSS          ENDS

              IFDEF    NEARCODE
TOP           EQU      4
BOT           EQU      6
_TEXT         SEGMENT  BYTE PUBLIC 'CODE'
DGROUP        GROUP    _DATA,_BSS
              ASSUME   DS:DGROUP
              ASSUME   CS:_TEXT
              IFDEF    NEARDATA
              ASSUME   SS:DGROUP
              ENDIF
_VCURSOR      PROC     NEAR
              ENDIF

              IFDEF    FARCODE
TOP           EQU      6
BOT           EQU      8
_VCURSOR_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP        GROUP    _DATA,_BSS
              ASSUME   DS:DGROUP
              ASSUME   CS:_VCURSOR_TEXT
              IFDEF    NEARDATA
              ASSUME   SS:DGROUP
              ENDIF
_VCURSOR      PROC     FAR
              ENDIF

              PUSH BP
              MOV  BP,SP
              PUSH DI
              PUSH SI
              mov  cl,[bp+BOT] ; fetch bottom line of cursor
              mov  ch,[bp+TOP] ; fetch top line of cursor
              mov  ah,1        ; video service 1
              int  10h         ; call bios
              POP  SI
              POP  DI
              POP  BP
              RET
_VCURSOR      ENDP

              IFDEF    NEARCODE
_TEXT         ENDS
              ENDIF

              IFDEF    FARCODE
_VCURSOR_TEXT ENDS
              ENDIF

              END

>>> VDROP.ASM 1911
;
; vdrop(lines, attr, trow, brow, lcol, rcol)
;      BP  +4    +6    +8   +10   +12   +14  <-- NEAR
;      BP  +6    +8   +10   +12   +14   +16  <-- FAR
;
; Scroll Down
;
             PUBLIC  _VDROP

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
LINES        EQU      4
ATTR         EQU      6
TROW         EQU      8
BROW         EQU     10
LCOL         EQU     12
RCOL         EQU     14
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VDROP       PROC     NEAR
             ENDIF

             IFDEF    FARCODE
LINES        EQU      6
ATTR         EQU      8
TROW         EQU     10
BROW         EQU     12
LCOL         EQU     14
RCOL         EQU     16
_VDROP_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_VDROP_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VDROP       PROC     FAR
             ENDIF

             PUSH BP
             MOV  BP,SP
             PUSH DI
             PUSH SI
             mov  dl,[bp+RCOL]  ; fetch rcol
             mov  cl,[bp+LCOL]  ; fetch lcol
             mov  dh,[bp+BROW]  ; fetch brow
             mov  ch,[bp+TROW]  ; fetch trow
             mov  bh,[bp+ATTR]  ; fetch attr 
             mov  al,[bp+LINES] ; fetch lines
             mov  ah,7          ; video service 7
             int  10h           ; call bios
             POP  SI
             POP  DI
             POP  BP
             RET
_VDROP       ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_VDROP_TEXT  ENDS
             ENDIF

             END

>>> VGOTO.ASM 1593
;
; vgoto(row, col, page)
;     BP +4   +6    +8  <-- NEAR
;     BP +6   +8   +10  <-- FAR
;
; Go To Row & Col on Page 
;
             PUBLIC  _VGOTO

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
ROW          EQU      4
COL          EQU      6
PAG          EQU      8
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VGOTO       PROC     NEAR
             ENDIF

             IFDEF    FARCODE
ROW          EQU      6
COL          EQU      8
PAG          EQU     10
_VGOTO_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_VGOTO_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VGOTO       PROC     FAR
             ENDIF

             PUSH BP
             MOV  BP,SP
             PUSH DI
             PUSH SI
             mov  bh,[bp+PAG] ; fetch page number 
             mov  dl,[bp+COL] ; fetch column number
             mov  dh,[bp+ROW] ; fetch row number
             mov  ah,2        ; video service 2
             int  10h         ; call bios
             POP  SI
             POP  DI
             POP  BP
             RET
_VGOTO       ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_VGOTO_TEXT  ENDS
             ENDIF

             END

>>> VISAT.ASM 1429
;
; visat(page)
;     BP  +4  <-- NEAR
;     BP  +6  <-- FAR
;
; Return Cursor (Row & Col) on Page
;
             PUBLIC  _VISAT

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
PAG          EQU      4
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VISAT       PROC     NEAR
             ENDIF

             IFDEF    FARCODE
PAG          EQU      6
_VISAT_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_VISAT_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VISAT       PROC     FAR
             ENDIF

             PUSH BP
             MOV  BP,SP
             PUSH DI
             PUSH SI
             mov  bh,[bp+PAG] ; fetch page number
             mov  ah,3        ; video service 3
             int  10h         ; call bios
             mov  ax,dx       ; return (row, column)
             POP  SI
             POP  DI
             POP  BP
             RET
_VISAT       ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_VISAT_TEXT  ENDS
             ENDIF

             END



>>> VLIFT.ASM 1909
;
; vlift(lines, attr, trow, brow, lcol, rcol)
;      BP  +4    +6    +8   +10   +12   +14  <-- NEAR
;      BP  +6    +8   +10   +12   +14   +16  <-- FAR
;
; Scroll Up
;
             PUBLIC  _VLIFT

_DATA        SEGMENT WORD PUBLIC 'DATA'
_DATA        ENDS

_BSS         SEGMENT WORD PUBLIC 'BSS'
_BSS         ENDS

             IFDEF    NEARCODE
LINES        EQU      4
ATTR         EQU      6
TROW         EQU      8
BROW         EQU     10
LCOL         EQU     12
RCOL         EQU     14
_TEXT        SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VLIFT       PROC     NEAR
             ENDIF

             IFDEF    FARCODE
LINES        EQU      6
ATTR         EQU      8
TROW         EQU     10
BROW         EQU     12
LCOL         EQU     14
RCOL         EQU     16
_VLIFT_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP       GROUP    _DATA,_BSS
             ASSUME   DS:DGROUP
             ASSUME   CS:_VLIFT_TEXT
             IFDEF    NEARDATA
             ASSUME   SS:DGROUP
             ENDIF
_VLIFT       PROC     FAR
             ENDIF

             PUSH BP
             MOV  BP,SP
             PUSH DI
             PUSH SI
             mov  dl,[bp+RCOL]  ; fetch rcol
             mov  cl,[bp+LCOL]  ; fetch lcol
             mov  dh,[bp+BROW]  ; fetch brow
             mov  ch,[bp+TROW]  ; fetch trow
             mov  bh,[bp+ATTR]  ; fetch attr 
             mov  al,[bp+LINES] ; fetch lines
             mov  ah,6          ; video service 6
             int  10h           ; call bios
             POP  SI
             POP  DI
             POP  BP
             RET
_VLIFT       ENDP

             IFDEF    NEARCODE
_TEXT        ENDS
             ENDIF

             IFDEF    FARCODE
_VLIFT_TEXT  ENDS
             ENDIF

             END

>>> VMODE.C 765
/*
** vmode(GET)  -- Return Mode
** vmode(mode) -- Set and Return Mode (and Clear Screen)
*/
#include "window.h"

                 /* NOTE: initializing vwait puts it in _DATA segment */
char vwait = 2;             /* default fast monochrome and slow color */

extern void near _setmode(int);
extern int  near _getmode();

int
vmode(mode) int mode; {
  int md;
  if(mode != GET) _setmode(mode);
  if((md = _getmode()) == 7)  vwait |= 2;      /* set fast mono bit */
  else                        vwait &= 1;      /* clear fast mono bit */
  return (md);
  }

/*
** Wait for Retrace To Eliminate Snow
*/
void
vslow() {
  vwait &= 2;      /* clear fast color bit */
  }

/*
** Don't Wait for Retrace
*/
void
vfast() {
  vwait |= 1;      /* set fast color bit */
  }

>>> VMODE2.ASM 1703
;
; _setmode(mode) -- Set Mode (and Clear Screen)
;     BP     +4  <-- NEAR
;     BP     +6  <-- FAR
;
; _getmode()     -- Return Mode
;
            PUBLIC  __SETMODE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
MODE        EQU      4
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF

            IFDEF    FARCODE
MODE        EQU      6
VMODE_TEXT  SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:VMODE_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF

__SETMODE   PROC     NEAR
            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI
            mov  al,[bp+MODE] ; fetch mode
            xor  ah,ah        ; video service 0
            int  10h          ; call bios
            POP  SI
            POP  DI
            POP  BP
            RET
__SETMODE   ENDP

  PUBLIC  __GETMODE
__GETMODE   PROC     NEAR
            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI
            mov  ah,15        ; video service 15
            int  10h          ; call bios
            xor  ah,ah        ; return (zero, mode)
            POP  SI
            POP  DI
            POP  BP
            RET
__GETMODE   ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
VMODE_TEXT  ENDS
            ENDIF

            END

>>> VPAGE.ASM 1575
;
; vpage(page)
;     BP  +4  <-- NEAR
;     BP  +6  <-- FAR
;
; Set (optionally) and Return Video Page Number
;
            PUBLIC  _VPAGE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
PAG         EQU      4
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VPAGE      PROC     NEAR
            ENDIF

            IFDEF    FARCODE
PAG         EQU      6
_VPAGE_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VPAGE_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VPAGE      PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI
            mov al,[bp+PAG] ; fetch page number
            cmp al,255
            je  _vpage_2
            mov  ah,5       ; video service 5
            int  10h        ; call bios
_vpage_2:
            mov  ah,15      ; video service 15
            int  10h        ; call bios
            xor  ah,ah      ; zero high order bits
            mov  al,bh      ; return page
            POP  SI
            POP  DI
            POP  BP
            RET
_VPAGE      ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VPAGE_TEXT ENDS
            ENDIF

            END

>>> VPOINT.C 720
/*
** Point ES to Video Page and Calculate Offset in AX
*/
extern near __es();

vpoint(row, col, mode, page)  int row, col, mode, page; {
  switch(mode) {
    case 0:                                   /* 40x25 16 shades */
    case 1: __es(0xB800 + (page * 0x0080));   /* 40x25 16 colors */
     forty: return (row * 80) + (col << 1);   /* offset within page */

    case 2:                                   /* 80x25 16 shades */
    case 3:                                   /* 80x25 16 colors */
   default: __es(0xB800 + (page * 0x0100));
            goto eighty;

    case 7: __es(0xB000);                     /* 80x25 monochrome */
    eighty: return (row * 160) + (col << 1);  /* offset within page */
    }
  }

>>> VPOINT2.ASM 1093
;
; __es(arg)
;  Move argument to ES
; 
            PUBLIC ___ES

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF

            IFDEF    FARCODE
VPOINT_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS: VPOINT_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
            ENDIF

___ES       PROC     NEAR
            PUSH     BP
            MOV      BP,SP

            mov      ax,[bp+4]    ; fetch argument
            mov      es,ax        ; move AX to ES

            POP      BP
            RET
___ES       ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
VPOINT_TEXT ENDS
            ENDIF

            END

>>> VSHOW.ASM 3368
;
; vshow(buf, len, row, col, mode, page)
;    BP  +4   +8  +10  +12   +14   +16  <-- NEARCODE
;    BP  +6  +10  +12  +14   +16   +18  <-- FARCODE
;
; Show Saved Field Again
;
            PUBLIC  _VSHOW
            EXTRN   _VWAIT:BYTE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
BUFOFF      EQU      4
BUFSEG      EQU      6
LEN         EQU      8
ROW         EQU     10
COL         EQU     12
MOD         EQU     14
PAG         EQU     16
            EXTRN   _VPOINT:NEAR
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSHOW      PROC     NEAR
            ENDIF

            IFDEF    FARCODE
BUFOFF      EQU      6
BUFSEG      EQU      8
LEN         EQU     10
ROW         EQU     12
COL         EQU     14
MOD         EQU     16
PAG         EQU     18
            EXTRN   _VPOINT:FAR
_VSHOW_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VSHOW_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSHOW      PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            push word ptr [bp+PAG]
            push word ptr [bp+MOD]
            push word ptr [bp+COL]
            push word ptr [bp+ROW]
            call _vpoint        ; vpoint() -- point ES to page and calc offset
            add  sp,8

            push ds
            mov  di,ax          ; di -> offset in display-page (es)
            mov  cx,[bp+LEN]    ; length
            mov  ax,[bp+BUFSEG] ; buffer segment
            mov  ds,ax
            mov  si,[bp+BUFOFF] ; buffer offset
            or   cx,cx
            jz   __show_exit
__show_next:
            mov  bx,[si]        ; fetch next source byte
            inc  si             ; increment source index
            inc  si             ; increment source index
            push ds           ; save DS
            mov  ax,DGROUP    ; set up DS
            mov  ds, ax
            mov  al,_vwait
            pop  ds           ; restore DS
            or   al,al
            jnz  __show_go 
            mov  dx,03DAH       ; set port address in dx
__show_loop:
            in   al,dx          ; read CGA port 3DA
            rcr  al,1           ; horizontal retrace bit high?
            jc   __show_loop    ; yes, try again
            cli                 ; disable interrupts
__show_loop2:
            in   al,dx          ; read CGA port 3DA
            rcr  al,1           ; horizontal retrace bit low?
            jnc  __show_loop2   ; no, try again
__show_go:
            mov  es:[di],bx     ; show char/attr on screen
            sti                 ; reenable interrupts
            inc  di             ; increment destination index
            inc  di             ; increment destination index
            loop __show_next
__show_exit:
            pop  ds

            POP  SI
            POP  DI
            POP  BP
            RET
_VSHOW      ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VSHOW_TEXT ENDS
            ENDIF

            END

>>> VSTOW.ASM 3349
;
; vstow(buf, len, row, col, mode, page)
;    BP  +4   +8  +10  +12   +14   +16  <-- NEARCODE
;    BP  +6  +10  +12  +14   +16   +18  <-- FARCODE
;
; Stow Display Memory
;
            PUBLIC  _VSTOW
            EXTRN   _VWAIT:BYTE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
BUFOFF      EQU      4
BUFSEG      EQU      6
LEN         EQU      8
ROW         EQU     10
COL         EQU     12
MOD         EQU     14
PAG         EQU     16
            EXTRN   _VPOINT:NEAR
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSTOW      PROC     NEAR
            ENDIF

            IFDEF    FARCODE
BUFOFF      EQU      6
BUFSEG      EQU      8
LEN         EQU     10
ROW         EQU     12
COL         EQU     14
MOD         EQU     16
PAG         EQU     18
            EXTRN   _VPOINT:FAR
_VSTOW_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VSTOW_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSTOW      PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            push word ptr [bp+PAG]
            push word ptr [bp+MOD]
            push word ptr [bp+COL]
            push word ptr [bp+ROW]
            call _vpoint        ; vpoint() -- point ES to page and calc offset
            add  sp,8

            push ds
            mov  si,ax          ; si -> offset in display-page (es)
            mov  cx,[bp+LEN]    ; length
            mov  ax,[bp+BUFSEG] ; buffer segment
            mov  ds,ax
            mov  di,[bp+BUFOFF] ; buffer offset
            or   cx,cx
            jz   __stow_exit
__stow_next:
            push ds           ; save DS
            mov  ax,DGROUP    ; set up DS
            mov  ds, ax
            mov  al,_vwait
            pop  ds           ; restore DS
            or   al,al
            jnz  __stow_go
            mov  dx,03DAH       ; set port address in dx
__stow_loop:
            in   al,dx          ; read CGA port 3DA
            rcr  al,1           ; horizontal retrace bit high?
            jc   __stow_loop    ; yes, try again
            cli                 ; disable interrupts
__stow_loop2:
            in   al,dx          ; read CGA port 3DA
            rcr  al,1           ; horizontal retrace bit low?
            jnc  __stow_loop2   ; no, try again
__stow_go:
            mov  ax,es:[si]     ; fetch char/attr from screen
            sti                 ; reenable interrupts
            inc  si             ; increment source index
            inc  si             ; increment source index
            mov  [di],ax        ; stow char/attr
            inc  di             ; increment source index
            inc  di             ; increment source index
            loop __stow_next
__stow_exit:
            pop  ds

            POP  SI
            POP  DI
            POP  BP
            RET
_VSTOW      ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VSTOW_TEXT ENDS
            ENDIF

            END

>>> VSTR.ASM 3943
;
; vstr(str, len, row, col, mode, page)
;  BP   +4   +6   +8  +10   +12   +14   <-- NEARCODE  NEARDATA  (small)
;  BP   +4   +8  +10  +12   +14   +16   <-- NEARCODE  FARDATA   (compact)
;  BP   +6   +8  +10  +12   +14   +16   <-- FARCODE   NEARDATA  (medium)
;  BP   +6  +10  +12  +14   +16   +18   <-- FARCODE   FARDATA   (large)
;
; Display String Only
;
           PUBLIC  _VSTR
           EXTRN   _VWAIT:BYTE

_DATA      SEGMENT WORD PUBLIC 'DATA'
_DATA      ENDS

_BSS       SEGMENT WORD PUBLIC 'BSS'
_BSS       ENDS

           IFDEF    NEARCODE
           EXTRN   _VPOINT:NEAR
           IFDEF    NEARDATA
STR        EQU      4
LEN        EQU      6
ROW        EQU      8
COL        EQU     10
MOD        EQU     12
PAG        EQU     14
           ENDIF
           IFDEF    FARDATA
STR        EQU      4
LEN        EQU      8
ROW        EQU     10
COL        EQU     12
MOD        EQU     14
PAG        EQU     16
           ENDIF
_TEXT      SEGMENT  BYTE PUBLIC 'CODE'
DGROUP     GROUP    _DATA,_BSS
           ASSUME   DS:DGROUP
           ASSUME   CS:_TEXT
           IFDEF    NEARDATA
           ASSUME   SS:DGROUP
           ENDIF
_VSTR      PROC     NEAR
           ENDIF

           IFDEF    FARCODE
           EXTRN   _VPOINT:FAR
           IFDEF    NEARDATA
STR        EQU      6
LEN        EQU      8
ROW        EQU     10
COL        EQU     12
MOD        EQU     14
PAG        EQU     16
           ENDIF
           IFDEF    FARDATA
STR        EQU      6
LEN        EQU     10
ROW        EQU     12
COL        EQU     14
MOD        EQU     16
PAG        EQU     18
           ENDIF
_VSTR_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP     GROUP    _DATA,_BSS
           ASSUME   DS:DGROUP
           ASSUME   CS:_VSTR_TEXT
           IFDEF    NEARDATA
           ASSUME   SS:DGROUP
           ENDIF
_VSTR      PROC     FAR
           ENDIF

           PUSH BP
           MOV  BP,SP
           PUSH DI
           PUSH SI

           push word ptr [bp+PAG]
           push word ptr [bp+MOD]
           push word ptr [bp+COL]
           push word ptr [bp+ROW]
           call _vpoint      ; vpoint() -- point ES to page and calc offset
           add  sp,8

           mov  di,ax        ; di -> offset in display-page (es)

           IFDEF    FARDATA
           push ds
           mov  ax,[bp+STR+2]; string segment
           mov  ds,ax
           ENDIF

           mov  si,[bp+STR]  ; string offset
           mov  cx,[bp+LEN]  ; length
           or   cx,cx
           jz   __str_exit

__str_next:
           mov  bl,[si]      ; fetch next char

           IFDEF    NEARDATA
           mov  al,_vwait
           ENDIF

           IFDEF    FARDATA
           push ds           ; save DS
           mov  ax,DGROUP    ; set up DS
           mov  ds, ax
           mov  al,_vwait
           pop  ds           ; restore DS
           ENDIF

           or   al,al
           jnz  __str_go
           mov  dx,03DAH     ; set port address in dx
__str_loop:
           in   al,dx        ; read CGA port 3DA
           rcr  al,1         ; horizontal retrace bit high?
           jc   __str_loop   ; yes, try again
           cli               ; disable interrupts
__str_loop2:
           in   al,dx        ; read CGA port 3DA
           rcr  al,1         ; horizontal retrace bit low?
           jnc  __str_loop2  ; no, try again
__str_go:
           mov  al,bl        ; fetch char
           stosb             ; put char on screen & incr di
           sti               ; reenable interrupts
           inc  di           ; increment past attribute
           inc  si           ; increment to next char
           loop __str_next

__str_exit:

           IFDEF    FARDATA
           pop  ds
           ENDIF

           POP  SI
           POP  DI
           POP  BP
           RET
_VSTR      ENDP

           IFDEF    NEARCODE
_TEXT      ENDS
           ENDIF

           IFDEF    FARCODE
_VSTR_TEXT ENDS
           ENDIF

           END

>>> VSTRA.ASM 4199
;
; vstra(str, len, attr, row, col, mode, page)
;    BP  +4   +6    +8  +10  +12   +14   +16  <-- NEARCODE NEARDATA  (small)
;    BP  +4   +8   +10  +12  +14   +16   +18  <-- NEARCODE FARDATA   (compact)
;    BP  +6   +8   +10  +12  +14   +16   +18  <-- FARCODE  NEARDATA  (medium)
;    BP  +6  +10   +12  +14  +16   +18   +20  <-- FARCODE  FARDATA   (large)
;
; Display String and Attribute
;
            PUBLIC  _VSTRA
            EXTRN   _VWAIT:BYTE

_DATA       SEGMENT WORD PUBLIC 'DATA'
_DATA       ENDS

_BSS        SEGMENT WORD PUBLIC 'BSS'
_BSS        ENDS

            IFDEF    NEARCODE
            EXTRN   _VPOINT:NEAR
            IFDEF    NEARDATA
STR         EQU      4
LEN         EQU      6
ATTR        EQU      8
ROW         EQU     10
COL         EQU     12
MOD         EQU     14
PAG         EQU     16
            ENDIF
            IFDEF    FARDATA
STR         EQU      4
LEN         EQU      8
ATTR        EQU     10
ROW         EQU     12
COL         EQU     14
MOD         EQU     16
PAG         EQU     18
            ENDIF
_TEXT       SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSTRA      PROC     NEAR
            ENDIF

            IFDEF    FARCODE
            EXTRN   _VPOINT:FAR
            IFDEF    NEARDATA
STR         EQU      6
LEN         EQU      8
ATTR        EQU     10
ROW         EQU     12
COL         EQU     14
MOD         EQU     16
PAG         EQU     18
            ENDIF
            IFDEF    FARDATA
STR         EQU      6
LEN         EQU     10
ATTR        EQU     12
ROW         EQU     14
COL         EQU     16
MOD         EQU     18
PAG         EQU     20
            ENDIF
_VSTRA_TEXT SEGMENT  BYTE PUBLIC 'CODE'
DGROUP      GROUP    _DATA,_BSS
            ASSUME   DS:DGROUP
            ASSUME   CS:_VSTRA_TEXT
            IFDEF    NEARDATA
            ASSUME   SS:DGROUP
            ENDIF
_VSTRA      PROC     FAR
            ENDIF

            PUSH BP
            MOV  BP,SP
            PUSH DI
            PUSH SI

            push word ptr [bp+PAG]
            push word ptr [bp+MOD]
            push word ptr [bp+COL]
            push word ptr [bp+ROW]
            call _vpoint      ; vpoint() -- point ES to page and calc offset
            add  sp,8

            mov  di,ax        ; di -> offset in display-page (es)

            IFDEF    FARDATA
            push ds
            mov  ax,[bp+STR+2]; string segment
            mov  ds,ax
            ENDIF

            mov  si,[bp+STR]  ; string address
            mov  cx,[bp+LEN]  ; length
            mov  bh,[bp+ATTR] ; attr
            or   cx,cx
            jz   __stra_exit

__stra_next:
            mov  bl,[si]      ; fetch next char

            IFDEF    NEARDATA
            mov  al,_vwait
            ENDIF

            IFDEF    FARDATA
            push ds           ; save DS
            mov  ax,DGROUP    ; set up DS
            mov  ds, ax
            mov  al,_vwait
            pop  ds           ; restore DS
            ENDIF

            or   al,al
            jnz  __stra_go
            mov  dx,03DAH     ; set port address in DX
__stra_loop:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit high?
            jc   __stra_loop  ; yes, try again
            cli               ; disable interrupts
__stra_loop2:
            in   al,dx        ; read CGA port 3DA
            rcr  al,1         ; horizontal retrace bit low?
            jnc  __stra_loop2 ; no, try again
__stra_go:
            mov  ax,bx        ; fetch char/attr
            stosw             ; put char/attr on screen & incr DI by 2
            sti               ; reenable interrupts
            inc  si           ; increment to next char
            loop __stra_next

__stra_exit:
            IFDEF    FARDATA
            pop  ds
            ENDIF

            POP  SI
            POP  DI
            POP  BP
            RET
_VSTRA      ENDP

            IFDEF    NEARCODE
_TEXT       ENDS
            ENDIF

            IFDEF    FARCODE
_VSTRA_TEXT ENDS
            ENDIF

            END 

>>> WATT.C 526
/*
** Display Attributes Only
*/
#include "window.h"


extern int  _window;
extern char _dmode;
extern char _dpage;

extern int      _arow();
extern int      _acol();
extern void      vatt();
extern int       wchop();
extern int       wseen();
extern void      wsend();
extern char far *wpoint();

void
watt(attr, len) int attr, len; {
  char far *cp;
  len = wchop(len);
  if(wseen(_window))
    vatt(attr, len, _arow(), _acol(), _dmode, _dpage);
  else {
    cp = wpoint();
    while(len--) {*++cp = attr; ++cp;}
    }
  }

>>> WAUTO.C 697
/*
** Control Automatic Newline and Scroll
*/
#include "window.h"

extern char
       *_stat,
       *_row,
       *_sill,
       _redge;

extern int
      _window;

extern int   wgo();
extern void  wlift();

void
wauto(newline, scroll) int newline, scroll; {
  if(newline) _stat[_window] |=  AUTONL;
  else        _stat[_window] &= ~AUTONL;
  if(scroll)  _stat[_window] |=  AUTOSCR;
  else        _stat[_window] &= ~AUTOSCR;
  }

/*
** Automatic Newline and Scroll
*/
void
wnl(force) int force; {
  if(force || _stat[_window] & AUTONL) {
    if(_row[_window] < _sill[_window])  wgo(1, - _redge);
    else {
      if(_stat[_window] & AUTOSCR)  wlift(1);
      wgo(0, - _redge);
      }
    }
  }

>>> WCHR.C 524
/*
** Display Characters Only
*/
#include "window.h"


extern int  _window;
extern char _dmode;
extern char _dpage;

extern int      _arow();
extern int      _acol();
extern void      vchr();
extern int       wchop();
extern int       wseen();
extern void      wsend();
extern char far *wpoint();

void
wchr(ch, len) char ch; int len; {
  char far *cp;
  len = wchop(len);
  if(wseen(_window))
    vchr(ch, len, _arow(), _acol(), _dmode, _dpage);
  else {
    cp = wpoint();
    while(len--) {*cp++ = ch; ++cp;}
    }
  }


>>> WCHRA.C 561
/*
** Display Characters and Attributes
*/
#include "window.h"


extern int  _window;
extern char _dmode;
extern char _dpage;

extern int      _arow();
extern int      _acol();
extern void      vchra();
extern int       wchop();
extern int       wseen();
extern void      wsend();
extern char far *wpoint();

void
wchra(ch, attr, len) char ch; int attr, len; {
  char far *cp;
  len = wchop(len);
  if(wseen(_window))
    vchra(ch, len, attr, _arow(), _acol(), _dmode, _dpage);
  else {
    cp = wpoint();
    while(len--) {*cp++ = ch; *cp++ = attr;}
    }
  }
>>> WCLEAN.C 425
/*
** Clean the Active Window
*/
#include "window.h"

extern char
       *_attr,          /* array of default attributes */
       *_sill;          /* array of bottom rows */
extern int
        _window;        /* the active window */

extern void  wblank();
extern int   wgoto();

void
wclean(attr) int attr; {
  int row;
  _attr[_window] = attr;
  row = -1;
  while(++row <= _sill[_window]) wblank(row);
  wgoto(0, 0);
  }

>>> WFRAME.C 1514
/*
** Draw a Sash Around the Active Window
*/
#include "window.h"

extern char
        _redge,       /* right edge of screen */
       *_sash,        /* array of sash offsets */
       *_side,        /* array of right most columns */
       *_sill,        /* array of bottom rows */
       *_trow,        /* array of top rows */
       *_brow,        /* array of bottom rows */
       *_lcol,        /* array of left columns */
       *_rcol;        /* array of right columns */
extern int
        _window;      /* the active window */

extern void  wchra();
extern int   wgoto();

void
wframe(sash, attr) int sash, attr; {
  int row; char *ch;
  switch(sash) {
     case 0: ch = "        "; break;
     case 1: ch = "ÚÄ¿³³ÀÄÙ"; break;
     case 2: ch = "ÉÍ»ººÈÍ¼"; break;
    default: return;
    }
  if(_trow[_window] >= _brow[_window] ||
     _lcol[_window] >= _rcol[_window]) return;
  if(_sash[_window]) {
    _sash[_window]  = 0;
    _sill[_window] += 2;
    _side[_window] += 2; 
    }
  wgoto(0, 0);
  wchra(ch[1], attr,  _redge);
  wchra(ch[0], attr, 1);
  wgoto(0, _side[_window]);
  wchra(ch[2], attr, 1);
  row = 0;
  while(++row < _sill[_window]) {
    wgoto(row, 0);
    wchra(ch[3], attr, 1);
    wgoto(row, _side[_window]);
    wchra(ch[4], attr, 1);
    }
  wgoto(row, 0);
  wchra(ch[6], attr, _redge);
  wchra(ch[5], attr, 1);
  wgoto(row, _side[_window]);
  wchra(ch[7], attr, 1);
  if(sash) {
    _sash[_window]  = 1;
    _sill[_window] -= 2;
    _side[_window] -= 2; 
    }
  wgoto(0, 0);
  }
>>> WGETC.C 508
/*
** Get the next character from the active window.
*/
#include "window.h"

extern char      *_stat;
extern int        _window;
extern int        _getkey();
extern char        wputc();

char
wgetc() {
  char ch;
  switch(ch = _getkey()) {
    case DOSEOF: return (EOF);
    case  ABORT: exit(7);
    case    DEL: ch = RUB;
    case    RUB:
    case   WIPE: break;
    case     LF:
    case     CR: ch = '\n';
        default: _stat[_window] &= ~CROPPED;
                 wputc(ch);
    }
  return (ch);
  }
>>> WGETF.C 3936
/*
** Get a field from the active window.
** Provides a default value and allows editing.
**
**     dest: Destination string
**      def: Default string
**      len: Length of field
**
**  returns: length of input string
**
** Dest must be at least one larger than len.
** Def must fit within dest.
*/
#include <ctype.h>
#include "window.h"

#define OVR   2             /* overwrite cursor size */
#define INS   4             /* insert cursor size */

extern int   _getkey();
extern int    wchop();
extern void   wchr();
extern int    wcol();
extern int    wcsize();
extern void   wcursor();
extern int    wgo();
extern int    wgoto();
extern int    wrow();

#pragma warn -pia

int
wgetf(dest, def, len) char *dest, *def; int len; {
  char *next, *end, *fence, *stop, *cp;
  int bof, io, cs, i, ch;
  cs = wcsize();
  io = INS;  wcursor(io);
  next = end = dest;
  len = wchop(len);
  fence = dest + len - 1;     /* set fence for dest */
  bof = wcol();               /* save beginning of field */
  wchr(' ', len);             /* clear old data from field */
  while( end <= fence
    && (*end = *def++)) {     /* establish default string */
    wchr(*end++, 1);
    wgo(0, 1);
    }
  *end = NULL;
  wgoto(wrow(), bof);         /* set cursor to beg of field */
  while((ch = _getkey()) != CR) {   /* fetch next character */
    if(end < fence)
         stop = end;
    else stop = fence;
    switch(ch) {
      case  175:              /*        F4 = insert/overwrite */
        if(io == OVR) io = INS;
        else          io = OVR;
        wcursor(io);
        break;

      case  184:              /*      HOME = go to bof */
        wgoto(wrow(), bof);
        next = dest;
        break;

      case  188:              /*      LEFT = go left one */
        if(next > dest) {
          wgo(0, -1);
          --next;
          }
        break;

      case  190:              /*     RIGHT = go right one */
        i = 1;
        goto bump;

      case  192:              /*       END = go to eof */
        i = 1000;

        bump:
        if((next + i) > stop)
          i = stop - next;
        next += i;
        wgo(0, i);
        break;

      case    9:              /* RIGHT TAB = next punct */
        while(next < stop) {
          wgo(0, 1);
          if(ispunct(*next++)) break;
          }
        break;

      case  195:              /*    INSERT = insert space */
        i = 0;                /* hold the cursor */
        ch = ' ';
        goto insert;

      default:                /*      data = insert/overwrite */
        if(!isprint(ch)) break;
        i = 1;                /* advance the cursor */
        if(io == INS) {                        /* insert */

          insert:
          if(end <= fence) {
            cp = end++;
            *end = NULL;
            wgo(0, cp - next);
            while(next < cp--) {
              wchr(*(cp + 1) = *cp, 1);
              wgo(0, -1);
              }
            goto post;
            }
          }
        else {                                 /* overwrite */
          if(next == end) *++end = NULL;

          post:
          wchr(*next = ch, 1);
          if(next < fence) {
            wgo(0, i);
            next += i;
            }
          }
        break;
  
      case   27:              /*       ESC = delete field */
        wgoto(wrow(), bof);
        wchr(' ', len);
        end = next = dest;
        *end = NULL;
        break;

      case    8:              /* BACKSPACE = delete prev char */
        if(next <= dest) break;
        wgo(0, -1);
        --next;

      case  196:              /*    DELETE = delete char */
        if(next < end) {
          cp = next;
          while(++cp < end) {
            wchr(*(cp - 1) = *cp, 1);
            wgo(0, 1);
            }
          wchr(' ', 1);
          *--end = NULL;
          wgo(0, next - end);
          }
        break;
      }
    }
  wcursor(cs);
  return (end - dest);
  }

>>> WGETS.C 1326
/*
** Gets a string from the active window.
** At most size-1 bytes are accepted.
** If size is negative, the newline terminator is
** truncated.  Input is terminated by a null character.
** Entry: str  = Pointer to destination buffer.
**        size = Size of the destination buffer.
** Normally Returns str.  Returns zero if only ^z is entered.
*/
#include "window.h"

extern int   wcol();
extern char  wgetc();
extern int   wgo();

char *
wgets(str, size) char *str; int size; {
  char *next;
  int backup, nl;
  if(size < 0) {
    size = 0 - size;
    nl = 0;
    }
  else nl = 1;
  next = str;
  while(--size > 0) {
    switch (*next = wgetc()) {
      case  EOF: *next = NULL;
                 if(next == str) return (NULL);
                 return (str);
      case '\n': *(next + nl) = NULL;
                 return (str);
      case  RUB: if(next > str) backup = 1;
                 else           backup = 0;
                 goto backout;
      case WIPE: backup = next - str;
        backout: ++size;
                 while(backup--) {
                   if(wcol()) wgo(0, -1);
                   else       wgo(-1, 1000);
                   wchr(' ', 1);
                   --next; ++size;
                   }
                 continue;
        default: ++next;
      }
    }
  *next = NULL;
  return (str);
  }
>>> WINDOW.H 1209
/*
**                    Small-Windows
**
**           Copyright 1987 James E. Hendrix
**
**                 All rights reserved
*/
#define GET     255        /* means 'get' to video functions */

                           /* status bits */
#define OPENED    1        /* open window? */
#define SEEN      2        /* shown window? */
#define AUTONL    4        /* auto newline allowed? */
#define AUTOSCR   8        /* auto scroll allowed? */
#define CROPPED  16        /* output cropped in wputc()? */

                           /* wgetc() and wgets() definitions */
#define ABORT     3        /* abort the program */
#define RUB       8        /* rub out the last byte */
#define WIPE     24        /* wipe out the line */
#define DOSEOF   26        /* DOS end-of-file byte */
#define DEL     127        /* same as rub out */

                           /* standard Small-C definitions */
#define EOF    (-1)        /* end of file */
#define YES      1         /* true */
#define NO       0         /* false */
#define NULL     0         /* null value */
#define BELL     7         /* ASCII bell */
#define LF      10         /* ASCII line feed */
#define CR      13         /* ASCII carriage return */


>>> WKERNEL.C 9066
/*
** wkernel.c -- Window Kernel Functions
*/
#include "window.h"

int       wactive();
void      wallow();
void      wblank();
int       wchop();
int       wcol();
int       wcsize();
void      wcursor();
int       wgo();
int       wgoto();
void      whide();
int       wjump();
int       wmode();
int       wok();
int       wopen();
int       wpage();
char far *wpoint();
int       wrow();
int       wseen();
void      wselect();
void      wshow();
void      wsxow();

extern char      * calloc();
extern char far  *farmalloc();
extern void       vshow();
extern void       vstow();
extern void       wchra();
extern void       vcursor();
extern void       vgoto();
extern int        vmode();
extern int        vpage();

char
        _dmode,     /* display mode */
        _dpage,     /* display page */
        _clines,    /* cursor scan lines */
        _redge,     /* right edge of screen */
        _bedge =24, /* bottom edge of screen */
        _dcsize = 4,    /* default window cursor size */
        _dseen  = SEEN, /* default window visibility */
        _dattr  = 7,    /* default window (payne) attribute */
       *_wstack,    /* window stack for wpush() and wpop() */
       *_stat,      /* array of window status bits */
       *_attr,      /* array of default attributes */
       *_csize,     /* array of cursor sizes in fourths */
       *_sash,      /* array of sash offsets */
       *_side,      /* array of right most columns */
       *_sill,      /* array of bottom rows */
       *_row,       /* array of cursor rows */
       *_col,       /* array of cursor columns */
       *_trow,      /* array of top rows */
       *_brow,      /* array of bottom rows */
       *_lcol,      /* array of left columns */
       *_rcol,      /* array of right columns */
  far **_bbuf,      /* array of background buffer pointers */
  far **_wbuf;      /* array of window buffer pointers */ 

int
  _window,          /* the active window */
  _max = 25,        /* default maximum number of windows */
  _wsptr;           /* window stack pointer for wpush() and wpop() */

/*
** Return the Active Window
*/
int
wactive() {
  return (_window);  
  }

/*
** Set the Maximum Number of Windows
*/
void
wallow(max) int max; {
  _max = max;
  }

/*
** Blank a Row
*/
void
wblank(row) int row; {
  wgoto(row, 0);
  wchra(' ', _attr[_window], _redge + 1);
  }

/*
** Chop Length at Right Side of Window
*/
int
wchop(len) int len; { 
  int max;
  if(len > (max = _side[_window] - _col[_window] + 1)) 
    return (max);
  return (len);
  } 

/*
** Return Current Column
*/
wcol() {
  return (_col[_window]);  
  }

/*
** Return Cursor Size
*/
int
wcsize() {
  return (_csize[_window]);  
  }

/*
** Set Cursor Size in Fourths
*/
void
wcursor(size) int size; {
  if(size >= 0 && size <= 4) {
    _csize[_window] = size;
    if(wseen(_window))
      vcursor(size ? _clines - ((_clines >> 2) * size) : 32, _clines-1);
    }
  }

/*
** Set Default Cursor Size in Fourths
*/
void
wdcursor(size) int size; {
  _dcsize = size;
  }

/*
** Set Default Window Visibility to Hidden
*/
void
wdhide() {
  _dseen = 0;
  }

/*
** Set Default Window Visibility to Seen
*/
void
wdshow() {
  _dseen = SEEN;
  }

/*
** Set Default Window Pane Attribute
*/
void
wdatt(at) int at; {
  _dattr = at;
  }

/*
** Adjust Cursor Position
*/
int
wgo(ra, ca) int ra, ca; {
  return (wjump(_row[_window] + ra, _col[_window] + ca, 0));
  }

/*
** Send Cursor To Specified Position
*/
int
wgoto(row, col) int row, col; {
  return (wjump(row, col, _sash[_window]));
  }

/*
** Hide the Active Window
*/
void
whide() {
  wsxow(vstow, &_wbuf[_window]);
  if(_bbuf[_window]) wsxow(vshow, &_bbuf[_window]);
  _stat[_window] &= ~SEEN;
  }

/*
** Jump Cursor to Row and Column
*/
int
wjump(row, col, sash) int row, col, sash; {
  int ok;
  ok = YES;
  switch(row > 0) {
    case  NO: if(row < -sash) {
                 row = -sash;
                 ok = NO;
                 }
              break; 
    case YES: if(row > _sill[_window] + sash) {
                 row = _sill[_window] + sash;
                 ok = NO;
                 }
    }
  switch(col > 0) {
    case  NO: if(col < 0) {
                 col = 0;
                 ok = NO;
                 }
              break; 
    case YES: if(col > _side[_window]) {
                 col = _side[_window];
                 ok = NO;
                 }
    }
  _row[_window] = row;
  _col[_window] = col;
  if(wseen(_window))
    vgoto(_trow[_window] + _sash[_window] + row,
          _lcol[_window] + _sash[_window] + col, _dpage);
  return (ok);
  }

/*
** wmode(GET)    Return Display Mode
** wmode(mode)   Set and Return Display Mode (and clear screen)
*/
int
wmode(md) int md; {
  switch(_dmode = vmode(md)) {
    case  0:                     /* 40x25 grey scale text */
    case  1: _redge  = 39;       /* 40x25 color text */
             _clines =  8;
              break;

    case  2:                     /* 80x25 grey scale text */
    case  3:                     /* 80x25 color text */
    default: _redge  = 79;
             _clines =  8;
              break;
    case  7: _redge  = 79;       /* 80x25 monochrome text */
             _clines = 12;       /* ignore last two lines */
              break;
    }
  return (_dmode);
  }

/*
** Is Window Number Valid?
*/
int
wok(w) int w; {
  return (_stat  &&  w >= 0  &&  w < _max  &&  _stat[w] & OPENED);  
  }

/*
** Open a Window
** Returns the number of the new window
*/
int
wopen(trow, brow, lcol, rcol, bbuf)
  int  trow, brow, lcol, rcol, bbuf; {
  int w;
  if(!_stat) {                            /* first open? */
    wmode(GET);
    wpage(GET);
    if(!(_attr   = calloc(_max, sizeof(char))) ||
       !(_csize  = calloc(_max, sizeof(char))) ||
       !(_sash   = calloc(_max, sizeof(char))) ||
       !(_side   = calloc(_max, sizeof(char))) ||
       !(_sill   = calloc(_max, sizeof(char))) ||
       !(_row    = calloc(_max, sizeof(char))) ||
       !(_col    = calloc(_max, sizeof(char))) ||
       !(_trow   = calloc(_max, sizeof(char))) ||
       !(_brow   = calloc(_max, sizeof(char))) ||
       !(_lcol   = calloc(_max, sizeof(char))) ||
       !(_rcol   = calloc(_max, sizeof(char))) ||
       !(_bbuf   = (char far **) calloc(_max, sizeof(char far *))) ||
       !(_wbuf   = (char far **) calloc(_max, sizeof(char far *))) ||
       !(_wstack = calloc(_max, sizeof(char))) ||
       !(_stat   = calloc(_max, sizeof(char)))
      ) abort(1);                         /* insufficient memory */
    }
  w = -1;
  while(++w < _max) {
    if(_stat[w] == 0) {
      if(trow < 0) trow = 0;
      if(lcol < 0) lcol = 0;
     if(brow > _bedge) brow = _bedge;
      if(rcol > _redge) rcol = _redge;
      if(trow > brow) trow = brow;
      if(lcol > rcol) lcol = rcol;
      _sill[w] = (_brow[w] = brow) - (_trow[w] = trow);
      _side[w] = (_rcol[w] = rcol) - (_lcol[w] = lcol);
      _row[w] =  _col[w] = 0;
      _stat[w] = OPENED | _dseen | AUTONL | AUTOSCR;
      _attr[w] = _dattr;
      _csize[w] = _dcsize;
      _sash[w] = 0;
      wselect(w);
      if(bbuf)    wsxow(vstow, &_bbuf[w]);
      if(!_dseen) wsxow(vstow, &_wbuf[w]);
      return (w);
      }
    }
  return (--w);
  }

/*
** wpage(GET)    Return Display Page
** wpage(page)   Set and Return Display Page
*/
int
wpage(pg) int pg; {
  return (_dpage = vpage(pg));
  }

/*
** Point to Cursor Position in the View Buffer
*/
char far *
wpoint() {
  return (_wbuf[_window] +
           (
             (
               (_row[_window] + _sash[_window]) *
               (_rcol[_window] - _lcol[_window] + 1) +
               _col[_window] + _sash[_window]
             ) << 1
           )
         );
  }

/*
** Return Current Row
*/
int
wrow() {
  return (_row[_window]);  
  }

/*
** Is the Specified Window Seen?
*/
int
wseen(w) int w; {
  return (_stat[w] & SEEN);
  }

/*
** Make a Window Active
*/
void
wselect(w) int w; { 
  if(wok(w)) {
    if(wseen(_window = w)) {
      wcursor(_csize[_window]);
      wgoto(_row[_window], _col[_window]);
      }
    } 
  } 

/*
** Show the Active Window
*/
void
wshow() {
  if(_bbuf[_window]) wsxow(vstow, &_bbuf[_window]);
  wsxow(vshow, &_wbuf[_window]);
  _stat[_window] |= SEEN;
  wcursor(_csize[_window]);
  wgoto(_row[_window], _col[_window]);
  }

/*
** Show or Stow the Active Window
*/
void
wsxow(func, buf) void (    *func)(); char far **buf; {
  int width, height, size, col, row, end;
  char far *offset;
  width  = _rcol[_window] - _lcol[_window] + 1;
  height = _brow[_window] - _trow[_window] + 1;
  size = (width * height) << 1;
  if(!(*buf)) {
    if(!(*buf = farmalloc((long) size))) abort(1);
    _pad(*buf, size, 0);
    }
  offset = *buf;
  col = _lcol[_window];
  row = _trow[_window] - 1;
  end = row + height;
  while(++row <= end) {
    (*func)(offset, width, row, col, _dmode, _dpage);
    offset += width << 1;
    }
  }

/*
** return absolute column
*/
int
_acol() {
  return(_lcol[_window] + _sash[_window] + _col[_window]);
  }

/*
** return absolute row
*/
int
_arow() {
  return(_trow[_window] + _sash[_window] + _row[_window]);
  }

>>> WMOVE.C 1244
/*
** Move the Active Window Around
*/
#include "window.h"

extern char
      _redge,       /* right edge of screen */
      _bedge,       /* bottom edge of screen */
     *_trow,        /* array of top rows */
     *_brow,        /* array of bottom rows */
     *_lcol,        /* array of left columns */
     *_rcol;        /* array of right columns */

extern int
      _window;      /* the active window */

extern void  whide();
extern int   wseen();
extern void  wshow();

void
wmove(ra, ca) int ra, ca; {
  switch(ra > 0) {
    case  NO: if((_trow[_window] + ra) < 0)
                ra = 0 - _trow[_window];
              break; 
    case YES: if((_brow[_window] + ra) > _bedge)
              ra = _bedge - _brow[_window];
    }
  switch(ca > 0) {
    case  NO: if((_lcol[_window] + ca) < 0)
                ca = 0 - _lcol[_window];
              break; 
    case YES: if((_rcol[_window] + ca) > _redge)
              ca = _redge - _rcol[_window];
    }
  if(wseen(_window)) {
    whide();
    _trow[_window] += ra;
    _brow[_window] += ra;
    _lcol[_window] += ca;
    _rcol[_window] += ca;
    wshow();
    }
  else {
    _trow[_window] += ra;
    _brow[_window] += ra;
    _lcol[_window] += ca;
    _rcol[_window] += ca;
    }
  }
>>> WPRINTF.C 2017
#include <ctype.h>
#include "window.h"

extern char      *_stat;
extern int        _window;

extern char  wputc(char);
/*
** wprintf(ctlstring, arg, arg, ...)
**
** Formatted print to the active window.
** Operates as described by Kernighan & Ritchie.
** Only b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
int
wprintf(ctl) char *ctl; {
  char *sptr, str[17];
  int  *nxtarg, arg, left, pad, cc, len, maxchr, width;
  _stat[_window] &= ~CROPPED;
  nxtarg = (int *) (&ctl + 1);
  cc = 0;                                         
  while(*ctl) {
    if(*ctl!='%') {wputc(*ctl++); ++cc; continue;}
    else ++ctl;
    if(*ctl=='%') {wputc(*ctl++); ++cc; continue;}
    if(*ctl=='-') {left = 1; ++ctl;} else left = 0;       
    if(*ctl=='0') pad = '0'; else pad = ' ';           
    if(isdigit(*ctl)) {
      width = atoi(ctl++);
      while(isdigit(*ctl)) ++ctl;
      }
    else width = 0;
    if(*ctl=='.') {            
      maxchr = atoi(++ctl);
      while(isdigit(*ctl)) ++ctl;
      }
    else maxchr = 0;
    arg = *nxtarg++;
    sptr = str;
    switch(*ctl++) {
      case 'c': str[0] = arg; str[1] = NULL; break;
      case 's':
                #if (sizeof(int *) > 2)
                  sptr = *((char far **) (nxtarg++ - 1));
                #else
                  sptr = (char *) arg;
                #endif
                break;
      case 'd': itoa (arg ,str, 10); break; /* signed */
      case 'b': itoab(arg, str,  2); break;
      case 'o': itoab(arg, str,  8); break;
      case 'u': itoab(arg, str, 10); break;
      case 'x': itoab(arg, str, 16); break;
      default:  goto exit;
      }
    len = strlen(sptr);
    if(maxchr && maxchr<len) len = maxchr;
    if(width>len) width = width - len; else width = 0; 
    if(!left) while(width--) {wputc(pad);     ++cc;}
    while(len--)             {wputc(*sptr++); ++cc;}
    if(left)  while(width--) {wputc(pad);     ++cc;}  
    }
  exit:
  _stat[_window] &= ~CROPPED;
  return(cc);
  }
>>> WPUSH.C 944
#include "window.h"

extern char
       *_stat,    /* window status array pointer */
       *_wstack;  /* stack for wpop() and wpush() */

extern int
        _window,  /* the active window */
        _wsptr,   /* stack pointer for wpop() and wpush() */
        _max;     /* maximum number of windows */

extern void wactive();
extern void whide();
extern void wselect();
extern int  wseen();
extern void wshow();

/*
** Push Active Window into the Background
** and Select another
*/
void
wpush(w, show) int w, show; {
  if(_stat) {         /* at least one window exists */
    if(_wsptr < _max) {
      _wstack[_wsptr] = wactive();
      wselect(w);
      if(show && !wseen(_window)) wshow(); 
      }
    ++_wsptr;
    }
  }

/*
** Pop Top Background Window into Active Status
*/
void
wpop(hide) int hide; {
  if(_wsptr) {
    if(--_wsptr < _max) {
      if(hide && wseen(_window)) whide(); 
      wselect(_wstack[_wsptr]);
      }
    }
  }
>>> WPUTC.C 635
/*
** Output a character to the active window.
** Entry: ch = Character to write.
** Returns character written.
*/
#include "window.h"

extern char
       *_stat,
       *_col,
       *_side;

extern int
       _window;

extern void  wnl();
extern int   wgo();
extern void  wchr();

char
wputc(ch) char ch; {
  if(ch == '\n') wnl(YES);
  else {
    if(_col[_window] < _side[_window]) {
      _stat[_window] &= ~CROPPED;
      wchr(ch, 1);
      wgo(0, 1);
      }
    else {
      if((_stat[_window] & CROPPED) == 0) {
        _stat[_window] |= CROPPED;
        wchr(ch, 1);
        wnl(NO);
        }
      }
    }
  return (ch);
  }
>>> WPUTCA.C 707
/*
** Output a character and attribute to the active window.
** Entry: ch = Character to write.
**      attr = Attribute
** Returns character written.
*/
#include "window.h"

extern char
       *_stat,
       *_col,
       *_side;

extern int
        _window;

extern void  wchra();
extern int   wgo();
extern void  wnl();

char
wputca(ch, attr) char ch; int attr; {
  if(ch == '\n') wnl(YES);
  else {
    if(_col[_window] < _side[_window]) {
      _stat[_window] &= ~CROPPED;
      wchra(ch, attr, 1);
      wgo(0, 1);
      }
    else {
      if((_stat[_window] & CROPPED) == 0) {
        _stat[_window] |= CROPPED;
        wchra(ch, attr, 1);
        wnl(NO);
        }
      }
    }
  return (ch);
  }
>>> WPUTS.C 225
/*
** Write a string to the active window. 
** Entry: string = Pointer to null-terminated string.
*/
#include "window.h"

extern char  wputc();

void
wputs(string) char *string; {
  while(*string) wputc(*string++);
  }






>>> WPUTSA.C 331
/*
** Write a string and attribute to the active window. 
** Entry: string = Pointer to null-terminated string.
**          attr = attribute
*/
#include "window.h"

extern void  watt();
extern char  wputc();

void
wputsa(string, attr) char *string; int attr; {
  while(*string) {
    watt(attr, 1);
    wputc(*string++);
    }
  }
>>> WSCANF.C 2396
#include <ctype.h>
#include "window.h"

char _ugot;            /* ungotten character */
char _getu();

extern char  wgetc();

/*
** wscanf(ctlstring, arg, arg, ...)
**
** Formatted read from the active window.
** Operates as described by Kernighan & Ritchie.
** Only b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
int
wscanf(ctl) char *ctl; {
  char *carg;
  int  *narg, **nxtarg, wast, ac, width, ch, cnv, base, ovfl, sign;
  unsigned temp;
  nxtarg = ((int **) &ctl) + 1;
  _ugot = ac = 0;
  while(*ctl) {
    if(isspace(*ctl)) {++ctl; continue;}
    if(*ctl++ != '%') continue;
    if(*ctl == '*') {
      narg = (int  *) &wast;
      carg = (char *) &wast;
      ++ctl;
      }
    else {
      narg = (int  *) *nxtarg;
      carg = (char *) *nxtarg;
      nxtarg += 1;
      }
    ctl += utoi(ctl, &width);
    if(!width) width = 32767;
    if(!(cnv = *ctl++)) break;
    while(isspace(ch = _getu())) ;
    if(ch == EOF) {
      if(ac) break;
      else   return(EOF);
      }
    _ugot = ch;
    switch(cnv) {
      case 'c':
        *carg = _getu();
        break;
      case 's':
        while(width--) {
          if((*carg = _getu()) == EOF) break;
          if(isspace(*carg)) break;
          if(carg != (char *) &wast) ++carg;
          }
        *carg = 0;
        break;
      default:
        switch(cnv) {
          case 'b': base =  2; sign = 1; ovfl = 32767; break;
          case 'd': base = 10; sign = 0; ovfl =  3276; break;
          case 'o': base =  8; sign = 1; ovfl =  8191; break;
          case 'u': base = 10; sign = 1; ovfl =  6553; break;
          case 'x': base = 16; sign = 1; ovfl =  4095; break;
          default:  return (ac);
          }
        *narg = temp = 0;
        while(width-- && !isspace(ch=_getu()) && ch!=EOF) {
          if(!sign)
            if(ch == '-') {sign = -1; continue;}
            else sign = 1;
          if(ch < '0') return (ac);
          if(ch >= 'a')      ch -= 87;
          else if(ch >= 'A') ch -= 55;
          else               ch -= '0';
          if(ch >= base || temp > ovfl) return (ac);
          temp = temp * base + ch;
          }
        *narg = sign * temp;
      }
    ++ac;                          
    }
  return (ac);
  }

char
_getu() {
  if(_ugot) {
    char tmp;
    tmp = _ugot;
    _ugot = 0;
    return (tmp);
    }
  return (wgetc());
  }

>>> WSCROLL.C 2114
#include "window.h"

extern char
        _redge,       /* right edge of screen */
       *_attr,        /* array of default attributes */
       *_sash,        /* array of sash offsets */
       *_sill,        /* array of bottom rows */
       *_row,         /* array of cursor rows */
       *_col,         /* array of cursor columns */
       *_trow,        /* array of top rows */
       *_brow,        /* array of bottom rows */
       *_lcol,        /* array of left columns */
       *_rcol;        /* array of right columns */

  extern int
        _window;      /* the active window */

extern void       vdrop();
extern void       vlift();
extern void       wblank();
extern int        wchop();
extern int        wgoto();
extern void       wsdis();
extern void       wsbuf();
extern int        wseen();
extern char far * wpoint();

/*
** Scroll the Active Window Up
*/
void
wlift(lines) int lines; {
  if(wseen(_window)) wsdis(lines, vlift);
  else               wsbuf(lines, 1);
  }

/*
** Scroll the Active Window Down
*/
void
wdrop(lines) int lines; {
  if(wseen(_window)) wsdis(lines, vdrop);
  else               wsbuf(lines, -1);
  }

/*
** Scroll the Screen
*/
void
wsdis(lines, func) int lines; void (*func)(); {
  int mx, tr, br, lc, rc;
  if(lines) {
    tr = _trow[_window] + _sash[_window];
    br = _brow[_window] - _sash[_window];
    lc = _lcol[_window] + _sash[_window];
    rc = _rcol[_window] - _sash[_window];
    if((mx = br - tr) < 1 || lines > mx) lines = 0;
    (*func)(lines, _attr[_window], tr, br, lc, rc);
    }
  }

/*
** Scroll the View Buffer
*/
void
wsbuf(lines, dir) int lines, dir; {
  int or, oc, len, l, row, end, far *ip1, far *ip2;
  or = _row[_window];
  oc = _col[_window];
  wgoto(0, 0);
  len = wchop(_redge + 1);
  while(lines--) {
    if(dir > 0) {row = 0; end = _sill[_window];}
    else        {end = 0; row = _sill[_window];}
    while(row != end) {
      wgoto(row,        0);  ip1 = (int far *) wpoint();
      wgoto(row += dir, 0);  ip2 = (int far *) wpoint();
      for(l = len; l--; *ip1++ = *ip2++) ;
      }
    wblank(end);
    }
  wgoto(or, oc);
  }
>>> WSTR.C 539
/*
** Display String without Attributes
*/
#include "window.h"

extern int  _window;
extern char _dmode;
extern char _dpage;

extern int      _arow();
extern int      _acol();
extern void      vstr();
extern int       wchop();
extern int       wseen();
extern void      wsend();
extern char far *wpoint();

void
wstr(str, len) char *str; int len; {
  char far *cp;
  len = wchop(len);
  if(wseen(_window))
    vstr(str, len, _arow(), _acol(), _dmode, _dpage);
  else {
    cp = wpoint();
    while(len--) {*cp++ = *str++; cp++;}
    }
  }
>>> WSTRA.C 564
/*
** Display String and Attributes
*/
#include "window.h"

extern int  _window;
extern char _dmode;
extern char _dpage;

extern int      _arow();
extern int      _acol();
extern void      vstra();
extern int       wchop();
extern int       wseen();
extern void      wsend();
extern char far *wpoint();

void
wstra(str, attr, len) char *str; int attr, len; {
  char far *cp;
  len = wchop(len);
  if(wseen(_window))
    vstra(str, len, attr, _arow(), _acol(), _dmode, _dpage);
  else {
    cp = wpoint();
    while(len--) {*cp++ = *str++; *cp++ = attr;}
    }
  }
>>> WTEST.C 10377
/*
** wtest.c -- Window Test Program
*/
#include <ctype.h>
#include <stdlib.h>
#include "sw.h"
#define   NO    0
#define  YES    1
#define  GET  255

int
  md       =  0, /* mode */
  pg       =  0, /* page */
  wasseen  =  0, /* was test window visible? */
  i        =  0, /* miscellaneous */
  j        =  0, /* miscellaneous */
  k        =  0, /* miscellaneous */
  w_main   =  0, /* main window number */
  w_prompt =  0, /* prompt window number */
  w_attrib =  0, /* attribute window number */
  wn       =  0, /* test window number */
  bb       =  1, /* background buffer? */
  sz       =  4, /* cursor size */
  sh       =  1, /* sash */
  nl       =  1, /* auto-newline? */
  sc       =  1, /* auto-scroll? */
  tr       = 17, /* top row */
  br       = 21, /* bottom row */
  lc       = 28, /* left column */
  rc       = 37, /* right column */
  yn       =  1, /* yes/no */
  r        =  0, /* row */
  c        =  0, /* column */
  no       =  1, /* number */
  at       =  7, /* attribute */
  ln       = 20; /* length */

char
  ch           = 'x',
  st[40]       = "abcdefghijklmnopqrstuvwxyz",
  response[31] = {0};

main() {
  md = wmode(GET);
  pg = wpage(GET);

  w_main = wopen(0, 23, 0, 39, NO);
  wclean(7);
  wframe(2, 7);
  wauto(NO, NO);

  w_prompt = wopen(24, 24, 0, 39, YES);
  whide();
  wclean(7);
  wauto(NO, NO);

  w_attrib = wopen(1, 18, 1, 33, YES);
  whide();
  wclean(7);
  wframe(1, 7);
  wauto(nl, sc);
  setatt();

  wn = wopen(tr, br, lc, rc, YES);
  wclean(at);
  wframe(1, at);

  while(YES) {
    refresh();
    switch(get_i("select: ")) { 
      case  1: wn = wopen(tr, br, lc, rc, bb); continue;
      case  2: wselect(wn);         continue;
      case  3: vslow();             continue;
      case  4: vfast();             continue;
      case  5: md = wmode(GET);     continue;
      case  6: whide();
               wselect(w_main);
               whide();
               md = wmode(md);
               wshow();
               wselect(wn);
               wshow();
               continue;
      case  7: pg = wpage(GET);     continue;
      case  8: whide();
               wselect(w_main);
               whide();
               pg = wpage(pg);
               wshow();
               wselect(wn);
               wshow();
               continue;
      case  9: whide();             continue;
      case 10: wshow();             continue;
      case 11: wgoto(r, c);         continue;
      case 12: go();                continue;
      case 13: wlift(no);           continue;
      case 14: wdrop(no);           continue;
      case 15: watt(at, ln);        continue;
      case 16: wchr(ch, ln);        continue;
      case 17: wchra(ch, at, ln);   continue;
      case 18: wstr(st, ln);        continue;
      case 19: wstra(st, at, ln);   continue;
      case 20: move();              continue;
      case 21: ln = wgetf(st, st, ln);
               continue;
      case 22: wcursor(sz);         continue;
      case 23: wclean(at);          continue;
      case 24: wframe(sh, at);      continue;
      case 25:      wauto(nl, sc);  continue;
      case 26: ch = wgetc();        continue;
      case 27:      wputc(ch);      continue;
      case 28:      wputca(ch, at); continue;
      case 29:      wgets(st,-ln);  continue;
      case 30:      wputs(st);      continue;
      case 31:      wputsa(st, at); continue;
      case 32:      wpush(wn, yn);
                    wn = wactive();
                    continue;
      case 33:      wpop(yn);
                    wn = wactive();
                    continue;
      case 34:      wdatt(at);      continue;
      case 35:      wdcursor(sz);   continue;
      case 36:      wdhide();       continue;
      }
    if(strncmp(response, "bb", 2) == 0) {
      bb = get_i("bb: ");
      continue;
      }
    else if(strncmp(response, "wn", 2) == 0) {
      wn = get_i("wn: ");
      continue;
      }
    else if(strncmp(response, "sh", 2) == 0) {
      sh =  get_i("sh: ");
      continue;
      }
    else if(strncmp(response, "st", 2) == 0) {
           get_s("st: ");
      continue;
      }
    else if(strncmp(response, "ln", 2) == 0) {
      ln = get_i("ln: ");
      continue;
      }
    else if(strncmp(response, "no", 2) == 0) {
      no = get_i("no: ");
      continue;
      }
    else if(strncmp(response, "at", 2) == 0) {
      attrib();
      continue;
      }
    else if(strncmp(response, "tr", 2) == 0) {
      tr = get_i("tr: ");
      continue;
      }
    else if(strncmp(response, "br", 2) == 0) {
      br = get_i("br: ");
      continue;
      }
    else if(strncmp(response, "lc", 2) == 0) {
      lc = get_i("lc: ");
      continue;
      }
    else if(strncmp(response, "rc", 2) == 0) {
      rc = get_i("rc: ");
      continue;
      }
    else if(strncmp(response, "sz", 2) == 0) {
      sz = get_i("sz: ");
      continue;
      }
    else if(strncmp(response, "ch", 2) == 0) {
      ch = get_c("ch: ");
      continue;
      }
    else if(strncmp(response, "md", 2) == 0) {
      md = get_i("md: ");
      continue;
      }
    else if(strncmp(response, "pg", 2) == 0) {
      pg = get_i("pg: ");
      continue;
      }
    else if(strncmp(response, "nl", 2) == 0) {
      nl = get_i("nl: ");
      continue;
      }
    else if(strncmp(response, "sc", 2) == 0) {
      sc = get_i("sc: ");
      continue;
      }
    else if(strncmp(response, "yn", 2) == 0) {
      yn = get_i("yn: ");
      continue;
      }
    else if(strncmp(response, "r ", 1) == 0) {
      r  = get_i(" r: ");
      continue;
      }
    else if(strncmp(response, "c ", 1) == 0) {
      c  = get_i(" c: ");
      continue;
      }
    else if(strncmp(response, "q ", 1) == 0) {
      wpush(w_prompt, YES);
      wclean(7);
      wgoto(0, 0);
      exit(0);
      }
    while(!poll(YES)) ;
    }
  }

refresh() {
  if(wseen(wactive())) {
    wasseen = YES;
    whide();
    }
  else wasseen = NO;
  wselect(w_main);
  wgoto( 0, 0); wstr("at:     1 wopen(tr,br,lc,rc,bb)       ", 38);
  wgoto( 1, 0); wstr("bb:     2 wselect(wn) 22 wcursor(sz)  ", 38);  
  wgoto( 2, 0); wstr("br:     3 vslow()     23 wclean(at)   ", 38);
  wgoto( 3, 0); wstr("c :     4 vfast()     24 wframe(sh,at)", 38);
  wgoto( 4, 0); wstr("ch:     5 wmode(255)  25 wauto(nl,sc) ", 38);
  wgoto( 5, 0); wstr("lc:     6 wmode(md)   26 wgetc()      ", 38);
  wgoto( 6, 0); wstr("ln:     7 wpage(255)  27 wputc(ch)    ", 38);
  wgoto( 7, 0); wstr("md:     8 wpage(pg)   28 wputca(ch,at)", 38);
  wgoto( 8, 0); wstr("nl:     9 whide()     29 wgets(st,-ln)", 38);
  wgoto( 9, 0); wstr("no:    10 wshow()     30 wputs(st)    ", 38);
  wgoto(10, 0); wstr("pg:    11 wgoto(r,c)  31 wputsa(st,at)", 38);
  wgoto(11, 0); wstr("r :    12 wgo(ra,ca)  32 wpush(wn,yn) ", 38);
  wgoto(12, 0); wstr("rc:    13 wlift(no)   33 wpop(yn)     ", 38);
  wgoto(13, 0); wstr("sc:    14 wdrop(no)   34 wdatt(at)    ", 38);
  wgoto(14, 0); wstr("sh:    15 watt(at,ln) 35 wdcursor(sz) ", 38);
  wgoto(15, 0); wstr("sz:    16 wchr(ch,ln) 36 wdhide()     ", 38);
  wgoto(16, 0); wstr("tr:    17 wchra(ch,at,ln)             ", 38);
  wgoto(17, 0); wstr("wn:    18 wstr(st,ln)                 ", 38);
  wgoto(18, 0); wstr("yn:    19 wstra(st,at,ln)             ", 38);
  wgoto(19, 0); wstr("(q)uit 20 wmove(ra,ca)                ", 38);
  wgoto(20, 0); wstr("       21 wgetf(st,st,ln)             ", 38);
  wgoto(21, 0); wstr("st:                                   ", 38);

  wgoto( 0, 3); wprintf("%3d", at); 
  wgoto( 1, 3); wprintf("%3d", bb);
  wgoto( 2, 3); wprintf("%3d", br); 
  wgoto( 3, 3); wprintf("%3d",  c); 
  wgoto( 4, 4); if(ch >= 32) wprintf(" %c", ch); 
                else         wprintf("^%c", ch + 64);
  wgoto( 5, 3); wprintf("%3d", lc); 
  wgoto( 6, 3); wprintf("%3d", ln); 
  wgoto( 7, 3); wprintf("%3d", md); 
  wgoto( 8, 3); wprintf("%3d", nl); 
  wgoto( 9, 3); wprintf("%3d", no); 
  wgoto(10, 3); wprintf("%3d", pg); 
  wgoto(11, 3); wprintf("%3d",  r); 
  wgoto(12, 3); wprintf("%3d", rc); 
  wgoto(13, 3); wprintf("%3d", sc); 
  wgoto(14, 3); wprintf("%3d", sh); 
  wgoto(15, 3); wprintf("%3d", sz); 
  wgoto(16, 3); wprintf("%3d", tr); 
  wgoto(17, 3); wprintf("%3d", wn);
  wgoto(18, 3); wprintf("%3d", yn); 
  wgoto(21, 3); wchr(' ', 40); 
                wprintf("%s ", st); 
  wselect(wn);
  if(wasseen) wshow();
  }

go() {
  while(YES) {
    switch(poll(YES)) {
      case 185: wgo(-1,  0); break;   /* up */
      case 193: wgo( 1,  0); break;   /* down */
      case 188: wgo( 0, -1); break;   /* left */
      case 190: wgo( 0,  1); break;   /* right */
      case 186: wgo(-5,  0); break;   /* pg up */
      case 194: wgo( 5,  0); break;   /* pg down */
      case 184: wgo( 0, -5); break;   /* home */
      case 192: wgo( 0,  5); break;   /* end */
      case  13: return;               /* return */
      }
    }
  }

move() {
  while(YES) {
    switch(poll(YES)) {
      case 185: wmove( -1,   0); break;   /* up */
      case 193: wmove(  1,   0); break;   /* down */
      case 188: wmove(  0,  -1); break;   /* left */
      case 190: wmove(  0,   1); break;   /* right */
      case 186: wmove(-10,   0); break;   /* pg up */
      case 194: wmove( 10,   0); break;   /* pg down */
      case 184: wmove(  0, -10); break;   /* home */
      case 192: wmove(  0,  10); break;   /* end */
      case  13: return;                   /* return */
      }
    }
  }

setatt() {
  for(i = 0; i < 16; ++i) {
    wgoto( i, 0);
    wstr("               ", 31);
    }
  for(i = 0; i < 16; ++i) {
    for(j = 0; j < 31; j += 2) {
      wgoto(i, j);
      watt(k++, 1);
      }
    }
  }

attrib() {
  whide();
  wselect(w_attrib);
  wgoto((at >> 4) & 15, (at & 15) << 1);
  wshow();
  while(YES) {
    switch(poll(YES)) {
      case 185: wgo( -1,  0); break;
      case 193: wgo(  1,  0); break;
      case 188: wgo(  0, -2); break;
      case 190: wgo(  0,  2); break;
      case  13: at = (wrow() * 16) + (wcol() / 2);
                whide();
                wselect(wn);
                wshow();
                return;
      }
    }
  }

get_i(pr) char *pr; {
  prompt(pr, response);
  return (atoi(response));
  }

get_c(pr) char *pr; {
  prompt(pr, response);
  return (*response);
  }

get_s(pr) char *pr; {
  prompt(pr, st);
  }

prompt(pr, buf) char *pr, *buf; {
  wpush(w_prompt, YES);
  wgoto(0, 0);
  wputs(pr);    
  wgetf(buf, "", 30);
  wpop(YES);
  }

