>>> FILES.C 10027
/*
** files.c - File Directory Functions
*/
#include "window.h"
#define CW         39     /* column width */
#define PERFILE (CW+2)    /* buffer space per file */
#define STACK    1024     /* allow for stack */

int
 *_farr,            /* file array pointer */
  _fdir,            /* file directory menu number */
  _fact,            /* file action menu number */
  _fnam,            /* filename window number */
  _fdo[4];          /* file action menu items */

char
 *_fbuf,            /* filename buffer */
  _dta[128];        /* Disk Transfer Area */

/*
** Setup for Calls to files()
** ha: highlighted attribute
** na: normal attribute
*/
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
files(fs) char *fs; {
  char *fp, *cp1, *cp2;
  int i, *ip, sz, pick, next, end;
  if(!_fnam)                              /* fileset() not called */
    return (0);
  if((sz = avail(NO)) < 0) sz  = 32767;   /* use up to 32K */
  if(sz < (PERFILE + STACK))              /* not enough memory */
    return (0);
  _farr = malloc(sz -= STACK);            /* get all you can */
  _fbuf = _farr + (end = sz/PERFILE) + 1;
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
      itou2(_dta+26, cp1, -8);            /* fetch file size */
      cp1 += 8;
      ip = _dta+24;                       /* date: yyyyyyym mmmddddd */
      cp1 = _dt(cp1, ' ', (*ip >> 5) &  15, 2);
      cp1 = _dt(cp1, '-',  *ip       &  31, 2);
      cp1 = _dt(cp1, '-', ((*ip >> 9) & 127) + 1980, 4);
      ip = _dta+22;                       /* time: hhhhhmmm mmmsssss */
      cp1 = _dt(cp1, ' ', (*ip >> 11) & 31, 2);
      cp1 = _dt(cp1, ':', (*ip >> 5)  & 63, 2);
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
    else break;                           /* exit if deleted file */
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
_align(dest, sour) int *dest, *sour; {
  int i;
  char *cp1, *cp2;
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
_dt(cp, ch, nbr, sz) char *cp, ch; int nbr, sz; {
  *cp++ = ch;
  itou(nbr, cp, -sz);
  if(*cp == ' ') *cp = '0';
  return (cp + sz);
  }

/*
** Get First Filename
** Return Error Status if Not Found
** fs: file spec pointer
** fa: file attirbutes
*/
_firstf(fs, fa) char *fs; int fa; {
  #asm
  push bx              ; preserve secondary register
  mov  dx,offset __dta ; file spec
  mov  ah,26           ; set DTA to DS:DX
  int  21h             ; call DOS
  mov  dx,[bp+6]       ; file spec
  mov  cx,[bp+4]       ; file attributes
  mov  ah,78           ; find first file
  int  21h             ; call DOS
  mov  ax,1            ; assume success
  jnc  __firstf_exit
  xor  ax,ax           ; failure
__firstf_exit:
  pop  bx              ; restore secondary register
  #endasm
  }

/*
** Get Next Filename
** Return Error Status if Not Found
*/
_nextf() {
  #asm
  push bx              ; preserve secondary register
  mov  ah,79           ; find next file
  int  21h             ; call DOS
  mov  ax,1            ; assume success
  jnc  __nextf_exit
  xor  ax,ax           ; failure
__nextf_exit:
  pop  bx              ; restore secondary register
  #endasm
  }

/*
** Rename a File
*/
_rename(new, old) char *new, *old; {
  #asm
  push bx              ; preserve secondary register
  mov  dx,[bp+4]       ; dx -> old path\filename
  mov  di,[bp+6]       ; dx -> new path\filename
  push ds
  pop  es
  mov  ah,86           ; rename a file
  int  21h             ; call DOS
  mov  ax,1            ; assume success
  jnc  __rename_exit
  xor  ax,ax           ; failure
__rename_exit:
  pop  bx              ; restore secondary register
  #endasm
  }

/*
** Delete a File
*/
_delete(fn) char *fn; {
  #asm
  push bx              ; preserve secondary register
  mov  dx,[bp+4]       ; dx -> path\filename
  mov  ah,65           ; delete a file
  int  21h             ; call DOS
  mov  ax,1            ; assume success
  jnc  __delete_exit
  xor  ax,ax           ; failure
__delete_exit:
  pop  bx              ; restore secondary register
  #endasm
  }

/*
** itou2 -- convert double precision nbr to unsigned decimal string
**         of width sz-1 right adjusted, blank filled; returns str
**
**    nbr = address of 4 byte integer -- low bits first
**    str = destination string address
**    sz  = size of string
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itou2(nbr, str, sz)  int *nbr;  char *str;  int sz;  {
  int lowbit, dvd[2], dvr[2], rem[2];
  if(sz > 0) str[--sz] = NULL;
  else if(sz < 0) sz = -sz;
  else while(str[sz] != NULL) ++sz;
  dvd[1] = nbr[1];  dvd[0] = nbr[0];
  dvr[1] = 0;
  while(sz) {
    lowbit = dvd[0] & 1;                       /* fetch low order bit */
    dvr[0] = 2; _div2(dvd, dvr, dvd, rem);     /* divide by 2 */
    dvd[1] &= 32767;                           /* zero high order bit */
    dvr[0] = 5; _div2(dvd, dvr, dvd, rem);     /* divide by 5 */
    str[--sz] = (rem[0] << 1) + lowbit + '0';  /* calc decimal digit */
    if(dvd[1] == 0 && dvd[0] == 0) break;      /* finished? */
    }
  while(sz) str[--sz] = ' ';
  return (str);
  }

int _div2ax, _div2bx;

_div2(dvd, dvr, quo, rem) int *dvd, *dvr, *quo, *rem; {
  #asm
  push bx
  mov  di, [bp+10]       ; di -> dividend
  mov  si, [bp+6]        ; si -> quotient
  mov  ax,[di]           ; initialize quotient with the dividend
  mov  [si],ax
  mov  ax,[di+2]
  mov  [si+2],ax
  mov  di, [bp+8]        ; di -> divisor
  xor  ax,ax             ; clear temporary high part of divisor
  xor  bx,bx
  mov  cx,33             ; initialize loop counter
__div20:
  mov  __div2ax,ax       ; save current high part
  mov  __div2bx,bx
  sub  ax,[di]           ; subtract divisor
  sbb  bx,[di+2]
  cmc                    ; complement CF
  jc   __div21           ; underflow?
  mov  ax,__div2ax       ; yes, restore
  mov  bx,__div2bx
__div21:
  rcl  word ptr [si],1   ; rotate (high <- quotient) one bit
  rcl  word ptr [si+2],1
  rcl  ax,1
  rcl  bx,1
  loop __div20           ; loop if not finished

  clc                    ; adjust and store remainder
  rcr  bx,1
  rcr  ax,1
  mov  di,[bp+4]         ; di -> remainder
  mov  [di],ax
  mov  [di+2],bx  
  pop bx
  #endasm
  }

>>> MENU.C 7914
/*
** menu.c -- Menu Management Functions
*/
#include "window.h"

#define RESET0  -32768

char
 *_sh,          /* array of sash values */
 *_ha,          /* array of highlight attributes */
 *_na,          /* array of normal attributes */
 *_nr,          /* array of number of rows */
 *_nc,          /* array of number of columns */
 *_cw;          /* array of column widths */

int
 *_po;          /* array of previous offsets */

extern int  _max;    /* maximum number of windows */
extern char _dseen;  /* default wopen() visibility status */

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
mopen(tr, lc, nr, nc, cw, ha, na, sh)
  int tr, lc, nr, nc, cw, ha, na, sh; {
  int menu, adj;
  char dseen;
  if(!_sh) {                            /* first open? */
    if(!(_po  = calloc(_max, 2)) ||
       !(_ha  = calloc(_max, 1)) ||
       !(_na  = calloc(_max, 1)) ||
       !(_cw  = calloc(_max, 1)) ||
       !(_nc  = calloc(_max, 1)) ||
       !(_nr  = calloc(_max, 1)) ||
       !(_sh  = calloc(_max, 1))
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
mfill(menu, pa, al, ao) int menu, *pa, al, ao; {
  int row, col, or, oc, hint;
  char *cp1, *cp2;
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
**   pa = string pointer array (0 = menu is already filled)
**        zeroth item is the menu label
**   al = array length
**
**        menus fill top down, left to right
*/
mpick(menu, pa, al) int menu, *pa, al; {
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
mshow(menu) int menu; {
  wpush(menu, YES);
  wpop(NO);
  }

/*
** Hide a Menu
*/
mhide(menu) int menu; {
  wpush(menu, NO);
  wpop(YES);
  }

>>> MSORT.C 921
#include "window.h"
#define NOCCARGC          /* no argument count passing */

/*
** Sort Any Part of an Array of String Pointers
**    pa: Pointer Array Address
**    lo: subscript of lowest element to sort
**    hi: subscript of highest element to sort
** order: 1 = ascending, -1 = descending
*/
msort(pa, lo, hi, order) int *pa, lo, hi, order; {
  int gap, i, j, jg, tmp;
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

_mcmp(p1, p2, order) char *p1, *p2; int order; {
  while(lexorder(*p1, *p2) == 0) {
    if(*p1 == NULL) return (0);
    ++p1; ++p2;
    }
  if(lexorder(*p1, *p2) > 0)
    return (order);
  return (-order);
  }

>>> MTEST.C 6186
/*
** mtest.c -- Menu Test Program
*/
#define  NO   0
#define YES   1

int
  pa[100],      /* pointer array for menu items */
  al,           /* array length */
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
  *fn="",
  fs[31]="*.*",
  response[31];

extern int _fdir;

main() {
  setlst();

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

setlst() {
  pa[ 0] = "MENU";
  pa[ 1] = " 1 coke";
  pa[ 2] = " 2 tea";
  pa[ 3] = " 3 milk";
  pa[ 4] = " 4 coffee";
  pa[ 5] = " 5 juice";
  pa[ 6] = " 6 sprite";
  pa[ 7] = " 7 water";
  pa[ 8] = " 8 cocoa";
  pa[ 9] = " 9 punch";
  pa[10] = "10 soda";
  pa[11] = "11 shake";
  pa[12] = "12 burger";
  pa[13] = "13 hotdog";
  pa[14] = "14 salad";
  pa[15] = "15 potato";
  pa[16] = "16 steak";
  pa[17] = "17 chicken";
  pa[18] = "18 fish";
  pa[19] = "19 chips";
  pa[20] = "20 ham";
  al = 21;
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
  int i;
  prompt(pr, response);
  dtoi(response, &i);
  return (i);
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

>>> VATT.C 1156
/*
** Display Attributes Only
*/
extern char vwait;
vatt(attr, len, row, col, mode, page)
  int attr, len, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  bl,[bp+14]   ; attr
  mov  cx,[bp+12]   ; length
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
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }

>>> VCHR.C 1155
/*
** Display Characters Only
*/
extern char vwait;
vchr(char, len, row, col, mode, page)
  int char, len, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  bl,[bp+14]   ; char
  mov  cx,[bp+12]   ; length
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
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> VCHRA.C 1181
/*
** Display Characters and Attributes
*/
extern char vwait;
vchra(chr, len, attr, row, col, mode, page)
  int chr, len, attr, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  bl,[bp+16]   ; char
  mov  cx,[bp+14]   ; length
  mov  bh,[bp+12]   ; attr
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
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> VCURSOR.C 335
/*
** Set Cursor Shape
*/
vcursor(top, bot) int top, bot; {
  #asm
  push bx         ; preserve secondary register
  mov  cl,[bp+4]  ; fetch bottom line of cursor
  mov  ch,[bp+6]  ; fetch top line of cursor
  mov  ah,1       ; video service 1
  int  10h        ; call bios
  pop  bx         ; restore secondary register
  #endasm
  }
>>> VDROP.C 480
/*
** Scroll Down
*/
vdrop(lines, attr, trow, brow, lcol, rcol)
  int  lines, attr, trow, brow, lcol, rcol; {
  #asm
  push bx         ; preserve secondary register
  mov  dl,[bp+4]  ; fetch rcol
  mov  cl,[bp+6]  ; fetch lcol
  mov  dh,[bp+8]  ; fetch brow
  mov  ch,[bp+10] ; fetch trow
  mov  bh,[bp+12] ; fetch attr 
  mov  al,[bp+14] ; fetch lines
  mov  ah,7       ; video service 7
  int  10h        ; call bios
  pop  bx         ; restore secondary register
  #endasm
  }
>>> VGOTO.C 376
/*
** Go To Row & Col on Page 
*/
vgoto(row, col, page) int row, col, page; {
  #asm
  push bx         ; preserve secondary register
  mov  bh,[bp+4]  ; fetch page number 
  mov  dl,[bp+6]  ; fetch column number
  mov  dh,[bp+8]  ; fetch row number
  mov  ah,2       ; video service 2
  int  10h        ; call bios
  pop  bx         ; restore secondary register
  #endasm
  }
>>> VISAT.C 328
/*
** Return Cursor (Row & Col) on Page
*/
visat(page) int page; {
  #asm
  push bx         ; preserve secondary register
  mov  bh,[bp+4]  ; fetch page number
  mov  ah,3       ; video service 3
  int  10h        ; call bios
  mov  ax,dx      ; return (row, column)
  pop  bx         ; restore secondary register
  #endasm
  }
>>> VLIFT.C 479
/*
** Scroll Up
*/
vlift(lines, attr, trow, brow, lcol, rcol)
  int  lines, attr, trow, brow, lcol, rcol; {
  #asm
  push bx         ; preserve secondary register
  mov  dl,[bp+4]  ; fetch rcol
  mov  cl,[bp+6]  ; fetch lcol
  mov  dh,[bp+8]  ; fetch brow
  mov  ch,[bp+10] ; fetch trow
  mov  bh,[bp+12] ; fetch attr 
  mov  al,[bp+14] ; fetch lines
  mov  ah,6       ; video service 6
  int  10h        ; call bios
  pop  bx         ; restore secondary register
  #endasm
  } 
>>> VMODE.C 1075
/*
** vmode(GET)  -- Return Mode
** vmode(mode) -- Set and Return Mode (and Clear Screen)
*/
#include "window.h"
char vwait = 2;      /* default fast monochrome and slow color */

vmode(mode) int mode; {
  int md;
  if(mode != GET) {
    #asm
    push bx         ; preserve secondary register
    mov  al,[bp+4]  ; fetch mode
    xor  ah,ah      ; video service 0
    int  10h        ; call bios
    pop  bx         ; restore secondary register
    #endasm
    }
  if((md = _vmode()) == 7)  vwait |= 2;      /* set fast mono bit */
  else                      vwait &= 1;      /* clear fast mono bit */
  return (md);
  }

/*
** Return Mode
*/
_vmode() {
  #asm
  push bx         ; preserve secondary register
  mov  ah,15      ; video service 15
  int  10h        ; call bios
  xor  ah,ah      ; return (zero, mode)
  pop  bx         ; restore secondary register
  #endasm
  }

/*
** Wait for Retrace To Eliminate Snow
*/
vslow() {
  vwait &= 2;      /* clear fast color bit */
  }

/*
** Don't Wait for Retrace
*/
vfast() {
  vwait |= 1;      /* set fast color bit */
  }

>>> VPAGE.C 644
/*
** vpage(GET)  -- Return Active Page
** vpage(page) -- Set and Return Active Page
*/
#include "window.h"
vpage(page) int page; {
  if(page != GET) {
    #asm
    push bx         ; preserve secondary register
    mov  al,[bp+4]  ; fetch page number
    mov  ah,5       ; video service 5
    int  10h        ; call bios
    pop  bx         ; restore secondary register
    #endasm
    }
  #asm
  push bx         ; preserve secondary register
  mov  ah,15      ; video service 15
  int  10h        ; call bios
  xor  ah,ah      ; zero high order bits
  mov  al,bh      ; return page
  pop  bx         ; restore secondary register
  #endasm
  }
>>> VPOINT.C 1227
/*
** Point ES to Video Page and Calculate Offset in AX
** To avoid pushing 4 arguments onto the stack again, vpoint()
** is called without arguments.  It overlooks the return address
** to the calling function and its byte pointer by declaring the
** arguments "return" and "bp."
** This is Small-C dependent code!
*/
vpoint(row, col, mode, page, return, bp)
   int row, col, mode, page, return, bp; {
  switch(mode) {
    case 0:                                /* 40x25 16 shades */
    case 1: 0xB800 + (page * 0x80);        /* 40x25 16 colors */
     forty: 
            #asm 
            mov es,ax                      ;  es -> display-page
            #endasm
            (row * 80) + (col << 1);       /* ax -> offset in page */
            break;
    case 2:                                /* 80x25 16 shades */
    case 3:                                /* 80x25 16 colors */
   default: 0xB800 + (page * 0x100);
            goto eighty;                   
    case 7: 0xB000;                        /* 80x25 monochrome */
    eighty:
            #asm 
            mov es,ax                      ;  es -> display-page
            #endasm
            (row * 160) + (col << 1);      /* ax -> offset in page */
    }
  }
>>> VSHOW.C 1320
/*
** Show Saved Again
*/
extern char vwait;
vshow(buf, len, row, col, mode, page)
  int *buf, len, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  cx,[bp+12]   ; length
  mov  si,[bp+14]   ; buffer address
  or   cx,cx
  jz   __show_exit
__show_next:
  mov  bx,[si]      ; fetch next source byte
  inc  si           ; increment source index
  inc  si           ; increment source index
  mov  al,_vwait
  or   al,al
  jnz  __show_go 
  mov  dx,03DAH     ; set port address in dx
__show_loop:
  in   al,dx        ; read CGA port 3DA
  rcr  al,1         ; horizontal retrace bit high?
  jc   __show_loop  ; yes, try again
  cli               ; disable interrupts
__show_loop2:
  in   al,dx        ; read CGA port 3DA
  rcr  al,1         ; horizontal retrace bit low?
  jnc  __show_loop2 ; no, try again
__show_go:
  mov  es:[di],bx   ; show char/attr on screen
  sti               ; reenable interrupts
  inc  di           ; increment destination index
  inc  di           ; increment destination index
  loop __show_next
__show_exit:
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> VSTOW.C 1208
/*
** Stow Display Memory
*/
extern char vwait;
vstow(buf, len, row, col, mode, page)
  int *buf, len, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  mov  si,ax        ; si -> offset in display-page (es)
  mov  cx,[bp+12]   ; length
  mov  di,[bp+14]   ; buffer address
  or   cx,cx
  jz   __stow_exit
__stow_next:
  mov  al,_vwait
  or   al,al
  jnz  __stow_go
  mov  dx,03DAH     ; set port address in dx
__stow_loop:
  in   al,dx        ; read CGA port 3DA
  rcr  al,1         ; horizontal retrace bit high?
  jc   __stow_loop  ; yes, try again
  cli               ; disable interrupts
__stow_loop2:
  in   al,dx        ; read CGA port 3DA
  rcr  al,1         ; horizontal retrace bit low?
  jnc  __stow_loop2 ; no, try again
__stow_go:
  mov  ax,es:[si]   ; fetch char/attr from screen
  sti               ; reenable interrupts
  inc  si           ; increment source index
  inc  si           ; increment source index
  mov  [di],ax      ; stow char/attr
  inc  di           ; increment source index
  inc  di           ; increment source index
  loop __stow_next
__stow_exit:
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> VSTR.C 1244
/*
** Display String Only
*/
extern char vwait;
vstr(str, len, row, col, mode, page)
  int str, len, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  si,[bp+14]   ; string address
  mov  cx,[bp+12]   ; length
  or   cx,cx
  jz   __str_exit
__str_next:
  mov  bl,[si]      ; fetch next char
  mov  al,_vwait
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
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> VSTRA.C 1276
/*
** Display String and Attribute
*/
extern char vwait;
vstra(str, len, attr, row, col, mode, page)
   char *str; int len, attr, row, col, mode, page; {
  vpoint();         /* point ES to page and calculate offset */
  #asm 
  push bx           ; preserve secondary register
  mov  di,ax        ; di -> offset in display-page (es)
  mov  si,[bp+16]   ; string address
  mov  cx,[bp+14]   ; length
  mov  bh,[bp+12]   ; attr
  or   cx,cx
  jz   __stra_exit
__stra_next:
  mov  bl,[si]      ; fetch next char
  mov  al,_vwait
  or   al,al
  jnz  __stra_go
  mov  dx,03DAH     ; set port address in dx
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
  stosw             ; put char/attr on screen & incr di by 2
  sti               ; reenable interrupts
  inc  si           ; increment to next char
  loop __stra_next
__stra_exit:
  pop  bx           ; restore secondary register
  mov  ax,ds        ; restore ES for Small C use
  mov  es,ax
  #endasm
  }
>>> WATT.C 322
/*
** Display Attributes Only
*/
#include "window.h"
#define NOCCARGC           /* no argument count passing */
extern int _window;
watt(attr, len) int attr, len; {
  char *cp;
  len = wchop(len);
  if(wseen(_window)) wsend(vatt, 0, attr, len);
  else {
    cp = wpoint();
    while(len--) {*++cp = attr; ++cp;}
    }
  }
>>> WAUTO.C 667
/*
** Control Automatic Newline and Scroll
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern char *_stat, *_row, *_sill, _redge;
extern int  _window;
wauto(newline, scroll) int newline, scroll; {
  if(newline) _stat[_window] |=  AUTONL;
  else        _stat[_window] &= ~AUTONL;
  if(scroll)  _stat[_window] |=  AUTOSCR;
  else        _stat[_window] &= ~AUTOSCR;
  }

/*
** Automatic Newline and Scroll
*/
wnl(force) int force; {
  if(force || _stat[_window] & AUTONL) {
    if(_row[_window] < _sill[_window])  wgo(1, - _redge);
    else {
      if(_stat[_window] & AUTOSCR)  wlift(1);
      wgo(0, - _redge);
      }
    }
  }

>>> WCHR.C 314
/*
** Display Characters Only
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern int _window;
wchr(ch, len) int ch, len; {
  char *cp;
  len = wchop(len);
  if(wseen(_window)) wsend(vchr, 0, ch, len);
  else {
    cp = wpoint();
    while(len--) {*cp++ = ch; ++cp;}
    }
  }
>>> WCHRA.C 354
/*
** Display Characters and Attributes
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern int _window;
wchra(ch, attr, len) char ch; int attr, len; {
  char *cp;
  len = wchop(len);
  if(wseen(_window)) wsend(vchra, ch, len, attr);
  else {
    cp = wpoint();
    while(len--) {*cp++ = ch; *cp++ = attr;}
    }
  }
>>> WCLEAN.C 413
/*
** Clean the Active Window
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern char
 *_attr,          /* array of default attributes */
 *_sill;          /* array of bottom rows */
extern int
  _window;        /* the active window */
wclean(attr) int attr; {
  int row;
  _attr[_window] = attr;
  row = -1;
  while(++row <= _sill[_window]) wblank(row);
  wgoto(0, 0);
  }

>>> WFRAME.C 1467
/*
** Draw a Sash Around the Active Window
*/
#define NOCCARGC           /* no argument count passing */
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
>>> WGETC.C 493
/*
** Get the next character from the active window.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern char *_stat;
extern int   _window;

wgetc() {
  int ch;
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
>>> WGETF.C 3749
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
#include "window.h"
#define NOCCARGC             /* no argument count passing */
#define OVR    2             /* overwrite cursor size */
#define INS    4             /* insert cursor size */

wgetf(dest, def, len) char *dest, *def; int len; {
  int bof, io, cs, i, ch;
  char *next, *end, *fence, *stop, *cp;
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

>>> WGETS.C 1312
/*
** Gets a string from the active window.
** At most size-1 bytes are accepted.
** If size is negative, the newline terminator is
** truncated.  Input is terminated by a null character.
** Entry: str  = Pointer to destination buffer.
**        size = Size of the destination buffer.
** Normally Returns str.  Returns zero if only ^z is entered.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

wgets(str, size) char *str; int size; {
  int backup, nl; char *next;
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


>>> WKERNEL.C 7960
/*
** wkernel.c -- Window Kernel Functions
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

char
  _dmode,       /* display mode */
  _dpage,       /* display page */
  _clines,      /* cursor scan lines */
  _redge,       /* right edge of screen */
  _bedge = 24,  /* bottom edge of screen */
  _dcsize = 4,    /* default window cursor size */
  _dseen  = SEEN, /* default window visibility */
  _dattr  = 7,    /* default window (payne) attribute */
 *_wstack,      /* window stack for wpush() and wpop() */
 *_stat,        /* array of window status bits */
 *_attr,        /* array of default attributes */
 *_csize,       /* array of cursor sizes in fourths */
 *_sash,        /* array of sash offsets */
 *_side,        /* array of right most columns */
 *_sill,        /* array of bottom rows */
 *_row,         /* array of cursor rows */
 *_col,         /* array of cursor columns */
 *_trow,        /* array of top rows */
 *_brow,        /* array of bottom rows */
 *_lcol,        /* array of left columns */
 *_rcol;        /* array of right columns */

int
  _window,      /* the active window */
  _max = 25,    /* default maximum number of windows */
  _wsptr,       /* window stack pointer for wpush() and wpop() */
 *_bbuf,        /* array of background buffer pointers */
 *_wbuf;        /* array of window buffer pointers */ 

/*
** Return the Active Window
*/
wactive() {
  return (_window);  
  }

/*
** Set the Maximum Number of Windows
*/
wallow(max) int max; {
  _max = max;
  }

/*
** Blank a Row
*/
wblank(row) int row; {
  wgoto(row, 0);
  wchra(' ', _attr[_window], _redge + 1);
  }

/*
** Chop Length at Right Side of Window
*/
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
wcsize() {
  return (_csize[_window]);  
  }

/*
** Set Cursor Size in Fourths
*/
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
wdcursor(size) int size; {
  _dcsize = size;
  }

/*
** Set Default Window Visibility to Hidden
*/
wdhide() {
  _dseen = 0;
  }

/*
** Set Default Window Visibility to Seen
*/
wdshow() {
  _dseen = SEEN;
  }

/*
** Set Default Window Pane Attribute
*/
wdatt(at) int at; {
  _dattr = at;
  }

/*
** Adjust Cursor Position
*/
wgo(ra, ca) int ra, ca; {
  return (wjump(_row[_window] + ra, _col[_window] + ca, 0));
  }

/*
** Send Cursor To Specified Position
*/
wgoto(row, col) int row, col; {
  return (wjump(row, col, _sash[_window]));
  }

/*
** Hide the Active Window
*/
whide() {
  wsxow(vstow, &_wbuf[_window]);
  if(_bbuf[_window]) wsxow(vshow, &_bbuf[_window]);
  _stat[_window] &= ~SEEN;
  }

/*
** Jump Cursor to Row and Column
*/
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
wok(w) int w; {
  return (_stat  &&  w >= 0  &&  w < _max  &&  _stat[w] & OPENED);  
  }

/*
** Open a Window
** Returns the number of the new window
*/
wopen(trow, brow, lcol, rcol, bbuf)
  int  trow, brow, lcol, rcol, bbuf; {
  int w;
  if(!_stat) {                            /* first open? */
    wmode(GET);
    wpage(GET);
    if(!(_attr   = calloc(_max, 1)) ||
       !(_csize  = calloc(_max, 1)) ||
       !(_sash   = calloc(_max, 1)) ||
       !(_side   = calloc(_max, 1)) ||
       !(_sill   = calloc(_max, 1)) ||
       !(_row    = calloc(_max, 1)) ||
       !(_col    = calloc(_max, 1)) ||
       !(_trow   = calloc(_max, 1)) ||
       !(_brow   = calloc(_max, 1)) ||
       !(_lcol   = calloc(_max, 1)) ||
       !(_rcol   = calloc(_max, 1)) ||
       !(_bbuf   = calloc(_max, 2)) ||
       !(_wbuf   = calloc(_max, 2)) ||
       !(_wstack = calloc(_max, 1)) ||
       !(_stat   = calloc(_max, 1))
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
wpage(pg) int pg; {
  return (_dpage = vpage(pg));
  }

/*
** Point to Cursor Position in the View Buffer
*/
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
wrow() {
  return (_row[_window]);  
  }

/*
** Is the Specified Window Seen?
*/
wseen(w) int w; {
  return (_stat[w] & SEEN);
  }

/*
** Make a Window Active
*/
wselect(w) int w; { 
  if(wok(w)) {
    if(wseen(_window = w)) {
      wcursor(_csize[_window]);
      wgoto(_row[_window], _col[_window]);
      }
    } 
  } 

/*
** Display Everything
*/
wsend(func, a1, a2, a3) int func, a1, a2, a3; {
  func(a1, a2, a3,
       _trow[_window] + _sash[_window] + _row[_window],
       _lcol[_window] + _sash[_window] + _col[_window],
       _dmode, _dpage);
  }

/*
** Show the Active Window
*/
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
wsxow(func, buf) int func, *buf; {
  int width, height, col, row, end;
  char *offset;
  width  = _rcol[_window] - _lcol[_window] + 1;
  height = _brow[_window] - _trow[_window] + 1;
  if(!(*buf) && !(*buf = calloc(width * height, 2))) abort(1);
  offset = *buf;
  col = _lcol[_window];
  row = _trow[_window] - 1;
  end = row + height;
  while(++row <= end) {
    func(offset, width, row, col, _dmode, _dpage);
    offset += width << 1;
    }
  }

>>> WMOVE.C 1203
/*
** Move the Active Window Around
*/
#define NOCCARGC           /* no argument count passing */
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
>>> WPRINTF.C 2018
/*
** wprintf(ctlstring, arg, arg, ...) - Formatted print
** to the active window.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
**
** Yes, that is correct.  Although these functions use an
** argument count, they do not call functions which need one.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

extern char *_stat;
extern int   _window;

wprintf(argc) int argc; {
  int cc;
  _stat[_window] &= ~CROPPED;
  cc = _wprintf(CCARGC() + &argc - 1);
  _stat[_window] &= ~CROPPED;
  return (cc);
  }

_wprintf(nxtarg) int *nxtarg; {
  int  arg, left, pad, cc, len, maxchr, width;
  char *ctl, *sptr, str[17];
  cc = 0;                                         
  ctl = *nxtarg--;                          
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
    arg = *nxtarg--;
    sptr = str;
    switch(*ctl++) {
      case 'c': str[0] = arg; str[1] = NULL; break;
      case 's': sptr = arg;        break;
      case 'd': itoa(arg,str);     break;
      case 'b': itoab(arg,str,2);  break;
      case 'o': itoab(arg,str,8);  break;
      case 'u': itoab(arg,str,10); break;
      case 'x': itoab(arg,str,16); break;
      default:  return (cc);
      }
    len = strlen(sptr);
    if(maxchr && maxchr<len) len = maxchr;
    if(width>len) width = width - len; else width = 0; 
    if(!left) while(width--) {wputc(pad); ++cc;}
    while(len--) {wputc(*sptr++); ++cc; }
    if(left) while(width--) {wputc(pad); ++cc;}  
    }
  return(cc);
  }
>>> WPUSH.C 900
#define NOCCARGC           /* no argument count passing */
#include "window.h"

extern char
 *_stat,       /* window status array pointer */
 *_wstack;     /* stack for wpop() and wpush() */
extern int
  _window,                /* the active window */
  _wsptr,                 /* stack pointer for wpop() and wpush() */
  _max;                   /* maximum number of windows */

/*
** Push Active Window into the Background
** and Select another
*/
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
wpop(hide) int hide; {
  if(_wsptr) {
    if(--_wsptr < _max) {
      if(hide && wseen(_window)) whide(); 
      wselect(_wstack[_wsptr]);
      }
    }
  }
>>> WPUTC.C 599
/*
** Output a character to the active window.
** Entry: ch = Character to write.
** Returns character written.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

extern char *_stat, *_col, *_side;
extern int   _window;

wputc(ch) int ch; {
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
>>> WPUTCA.C 665
/*
** Output a character and attribute to the active window.
** Entry: ch = Character to write.
**      attr = Attribute
** Returns character written.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

extern char *_stat, *_col, *_side;
extern int   _window;

wputca(ch, attr) int ch, attr; {
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
>>> WPUTS.C 250
/*
** Write a string to the active window. 
** Entry: string = Pointer to null-terminated string.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

wputs(string) char *string; {
  while(*string) wputc(*string++);
  }
>>> WPUTSA.C 341
/*
** Write a string and attribute to the active window. 
** Entry: string = Pointer to null-terminated string.
**          attr = attribute
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"

wputsa(string, attr) char *string; int attr; {
  while(*string) {
    watt(attr, 1);
    wputc(*string++);
    }
  }
>>> WSCANF.C 2417
/*
** wscanf(ctlstring, arg, arg, ...) - Formatted read
** from the active window.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
**
** Yes, that is correct.  Although these functions use an
** argument count, they do not call functions which need one.
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
char _ugot;                /* ungotten character */

wscanf(argc) int argc; {
  return (_wscanf(CCARGC() + &argc - 1));
  }

_wscanf(nxtarg) int *nxtarg; {
  char *carg, *ctl;
  int  *narg, wast, ac, width, ch, cnv, base, ovfl, sign;
  unsigned u;
  _ugot = ac = 0;
  ctl = *nxtarg--;
  while(*ctl) {
    if(isspace(*ctl)) {++ctl; continue;}
    if(*ctl++ != '%') continue;
    if(*ctl == '*') {narg = carg = &wast; ++ctl;}
    else             narg = carg = *nxtarg--;
    ctl += utoi(ctl, &width);
    if(!width) width = 32767;
    if(!(cnv = *ctl++)) break;
    while(isspace(ch = _getu())) ;
    if(ch == EOF) {if(ac) break; else return(EOF);}
    _ugot = ch;
    switch(cnv) {
      case 'c':
        *carg = _getu();
        break;
      case 's':
        while(width--) {
          if((*carg = _getu()) == EOF) break;
          if(isspace(*carg)) break;
          if(carg != &wast) ++carg;
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
        *narg = u = 0;
        while(width-- && !isspace(ch=_getu()) && ch!=EOF) {
          if(!sign)
            if(ch == '-') {sign = -1; continue;}
            else sign = 1;
          if(ch < '0') return (ac);
          if(ch >= 'a')      ch -= 87;
          else if(ch >= 'A') ch -= 55;
          else               ch -= '0';
          if(ch >= base || u > ovfl) return (ac);
          u = u * base + ch;
          }
        *narg = sign * u;
      }
    ++ac;                          
    }
  return (ac);
  }

_getu() {
  if(_ugot) {
    char tmp;
    tmp = _ugot;
    _ugot = 0;
    return (tmp);
    }
  return (wgetc());
  }

>>> WSCROLL.C 1797
#define NOCCARGC           /* no argument count passing */
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

/*
** Scroll the Active Window Up
*/
wlift(lines) int lines; {
  if(wseen(_window)) wsdis(lines, vlift);
  else               wsbuf(lines, 1);
  }

/*
** Scroll the Active Window Down
*/
wdrop(lines) int lines; {
  if(wseen(_window)) wsdis(lines, vdrop);
  else               wsbuf(lines, -1);
  }

/*
** Scroll the Screen
*/
wsdis(lines, vfunc) int lines, vfunc; {
  int mx, tr, br, lc, rc;
  if(lines) {
    tr = _trow[_window] + _sash[_window];
    br = _brow[_window] - _sash[_window];
    lc = _lcol[_window] + _sash[_window];
    rc = _rcol[_window] - _sash[_window];
    if((mx = br - tr) < 1 || lines > mx) lines = 0;
    vfunc(lines, _attr[_window], tr, br, lc, rc);
    }
  }

/*
** Scroll the View Buffer
*/
wsbuf(lines, dir) int lines, dir; {
  int or, oc, len, l, row, end, *ip1, *ip2;
  or = _row[_window];
  oc = _col[_window];
  wgoto(0, 0);
  len = wchop(_redge + 1);
  while(lines--) {
    if(dir > 0) {row = 0; end = _sill[_window];}
    else        {end = 0; row = _sill[_window];}
    while(row != end) {
      wgoto(row,        0);  ip1 = wpoint();
      wgoto(row += dir, 0);  ip2 = wpoint();
      for(l = len; l--; *ip1++ = *ip2++) ;
      }
    wblank(end);
    }
  wgoto(or, oc);
  }
>>> WSTR.C 338
/*
** Display String without Attributes
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern int _window;

wstr(str, len) char *str; int len; {
  char *cp;
  len = wchop(len);
  if(wseen(_window)) wsend(vstr, 0, str, len);
  else {
    cp = wpoint();
    while(len--) {*cp++ = *str++; cp++;}
    }
  }
>>> WSTRA.C 359
/*
** Display String and Attributes
*/
#define NOCCARGC           /* no argument count passing */
#include "window.h"
extern int _window;

wstra(str, attr, len) char *str; int attr, len; {
  char *cp;
  len = wchop(len);
  if(wseen(_window)) wsend(vstra, str, len, attr);
  else {
    cp = wpoint();
    while(len--) {*cp++ = *str++; *cp++ = attr;}
    }
  }
>>> WTEST.C 10200
/*
** wtest.c -- Window Test Program
*/
#define  NO   0
#define YES   1
#define GET   255

int
  md,          /* mode */
  pg,          /* page */
  wasseen,     /* was test window visible? */
  i, j, k,     /* miscellaneous variables */
  w_main,      /* main window number */
  w_prompt,    /* prompt window number */
  w_attrib,    /* attribute window number */
  wn,          /* test window number */
  bb =  1,     /* background buffer? */
  sz =  4,     /* cursor size */
  sh =  1,     /* sash */
  nl =  1,     /* auto-newline? */
  sc =  1,     /* auto-scroll? */
  tr = 17,     /* top row */
  br = 21,     /* bottom row */
  lc = 28,     /* left column */
  rc = 37,     /* right column */
  yn =  1,     /* yes/no */
  r  =  0,     /* row */
  c  =  0,     /* column */
  no =  1,     /* number */
  at =  7,     /* attribute */
  ln = 20;     /* length */

char
  ch='x',
  st[40]="abcdefghijklmnopqrstuvwxyz",
  response[31];

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
  int i;
  prompt(pr, response);
  dtoi(response, &i);
  return (i);
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

