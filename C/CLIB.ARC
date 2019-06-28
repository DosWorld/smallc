>>> CLIB.H 1617
/*
** CLIB.H -- Definitions for Small-C library functions.
**
** Copyright 1983  L. E. Payne and J. E. Hendrix
*/

/*
** Misc parameters
*/
#define MAXFILES 20  /* maximum open files */
#define DOSEOF   26  /* DOS end-of-file byte */
#define ARCHIVE  32  /* file archive bit */

/*
** DOS function calls
*/
#define CREATE   60  /* make file */
#define OPEN     61  /* open file */
#define CLOSE    62  /* close file or device */
#define READ     63  /* read from a file */
#define WRITE    64  /* write to a file */
#define DELETE   65  /* delete file */
#define SEEK     66  /* seek within a file */
#define CONTROL  68  /* control device */
#define FORCE    70  /* force use of a handle */
#define RETDOS   76  /* close files and return to DOS */
#define FNDFIL   78  /* find first occurrence of a file */
#define FNDNXT   79  /* find next occurrence of a file */
#define RENAME   86  /* rename file */

/*
** File status bits
*/
#define OPNBIT   1  /* open condition */
#define EOFBIT   2  /* end-of-file condition */
#define ERRBIT   4  /* error condition */

/*
** File positioning origins
*/
#define FROM_BEG  0  /* from beginning of file */
#define FROM_CUR  1  /* from current position */
#define FROM_END  2  /* from end of file */

/*
** Buffer usage codes
** NULL means the buffer is not used.
*/
#define EMPTY     1  /* buffer is currently empty */
#define IN        2  /* buffer is currently holding input data */
#define OUT       3  /* buffer is currently holding output data */

/*
** ASCII characters
*/
#define ABORT    3
#define RUB      8
#define PAUSE   19
#define WIPE    24
#define DEL    127

>>> ABS.C 117
/*
** abs -- returns absolute value of nbr
*/
abs(nbr)  int nbr; {
  if(nbr < 0) return (-nbr);
  return (nbr);
  }

>>> ATOI.C 259
/*
** atoi(s) - convert s to integer.
*/
atoi(s) char *s; {
  int sign, n;
  while(isspace(*s)) ++s;
  sign = 1;
  switch(*s) {
    case '-': sign = -1;
    case '+': ++s;
    }
  n = 0;
  while(isdigit(*s)) n = 10 * n + *s++ - '0';
  return (sign * n);
  }

>>> ATOIB.C 435
/*
** atoib(s,b) - Convert s to "unsigned" integer in base b.
**              NOTE: This is a non-standard function.
*/
atoib(s, b) char *s; int b; {
  int n, digit;
  n = 0;
  while(isspace(*s)) ++s;
  while((digit = (127 & *s++)) >= '0') {
    if(digit >= 'a')      digit -= 87;
    else if(digit >= 'A') digit -= 55;
    else                  digit -= '0';
    if(digit >= b) break;
    n = b * n + digit;
    }
  return (n);
  }


>>> AUXBUF.C 786
#include "stdio.h"
#include "clib.h"
extern int
  _bufsiz[MAXFILES],  /* size of buffer */
  _bufptr[MAXFILES];  /* aux buffer address */

/*
** auxbuf -- allocate an auxiliary input buffer for fd
**   fd = file descriptor of an open file
** size = size of buffer to be allocated
** Returns NULL on success, else ERR.
** Note: Ungetc() still works.
**       A 2nd call allocates a new buffer replacing old one.
**       If fd is a device, buffer is allocated but ignored.
**       Buffer stays allocated when fd is closed or new one is allocated.
**       May be used on a closed fd.
*/
auxbuf(fd, size) int fd; char *size; {   /* fake unsigned */
  if(!size || avail(NO) < size) return (ERR);
  _bufptr[fd] = malloc(size);
  _bufsiz[fd] = size;
  _empty(fd, NO);
  return (NULL);
  }

>>> AVAIL.C 347
extern char *_memptr;
/*
** Return the number of bytes of available memory.
** In case of a stack overflow condition, if 'abort'
** is non-zero the program aborts with an 'S' clue,
** otherwise zero is returned.
*/
avail(abort) int abort; {
  char x;
  if(&x < _memptr) {
    if(abort) exit(1);
    return (0);
    }
  return (&x - _memptr);
  }

>>> BSEEK.C 727
#include "stdio.h"
#include "clib.h"
extern int _nextc[], _bufuse[];
/*
** Position fd to the character in file indicated by "offset."
** "Offset" is the address of a long integer or array of two
** integers containing the offset, low word first.
** 
**     BASE     OFFSET-RELATIVE-TO
**       0      beginning of file
**       1      current byte in file
**       2      end of file (minus offset)
**
** Returns NULL on success, else EOF.
*/
bseek(fd, offset, base) int fd, offset[], base; {
  int hi, lo;
  if(!_mode(fd) || !_bufuse[fd]) return (EOF);
  if(_adjust(fd)) return (EOF);
  lo = offset[0];
  hi = offset[1];
  if(!_seek(base, fd, &hi, &lo)) return (EOF);
  _nextc[fd] = EOF;
  _clreof(fd);
  return (NULL);
  }

>>> BTELL.C 447
#include "stdio.h"
#include "clib.h"
extern int _bufuse[];
/*
** Retrieve offset to next character in fd.
** "Offset" must be the address of a long int or
** a 2-element int array.  The offset is placed in
** offset in low, high order.
*/
btell(fd, offset) int fd, offset[]; {
  if(!_mode(fd) || !_bufuse[fd]) return (EOF);
  if(_adjust(fd)) return (EOF);
  offset[0] = offset[1] = 0;
  _seek(FROM_CUR, fd, offset+1, offset);
  return (NULL);
  }
>>> CALL.ASM 3939
;
; Small-C Run Time Library for MS/PC-DOS
;
        extrn   __main: near
        extrn    _exit: near
        extrn   __memptr: word

data   segment public
        dw      1
data   ends

stack   segment stack
        dw      32 dup(?)
stack   ends

code    segment public
        assume  cs:code
start:
        mov     ax,data         ; set data segment for program
        mov     ds,ax
        mov     ax,es:[2]       ; paragraphs of memory on system
        sub     ax,data         ; paragraphs beyond code segment
        cmp     ah,10h          ; more than 64K?
        jb      start_1         ; no
        mov     ax,1000h        ; only use 64K
start_1:
        mov     cl,4
        shl     ax,cl           ; byte offset to end of data/free/stack
        cli                     ; disable interrupts
        mov     bx,ds
        mov     ss,bx           ; make data and stack segments coincide
        mov     sp,ax           ; top of stack = end of data/free/stack
        push    ax              ; force sp non-zero (if 64K used)
        sti                     ; reenable interrupts
        mov     ax,stack        ; paragraph following data
        sub     ax,data         ; number of data paragraphs
        shl     ax,cl           ; number of data bytes (offset to free/stack)
        mov     bx,ax
        inc     bh              ; adjust for minimum stack space
        cmp     bx,sp           ; enough memory?
        jb      start_2         ; yes
        mov     ax,1            ; no, terminate with exit code 1
        push    ax
        call    _exit
start_2:
        mov     __memptr,ax     ; set memory allocation pointer
;
; ------------ release unused memory -----------
; ------ cannot run debug with this code -------
;        mov     bx,sp
;        mov     ah,4AH
;        int     21H
; ----------------------------------------------
;
; make sure that es -> psp, because __main requires it
;
        jmp     __main          ; __main never returns

        public  _ccargc
_ccargc:
        mov     al,cl
        xor     ah,ah
        ret

;
; Test if Secondary (BX) <oper> Primary (AX)
;-------------------------- MASM version
;compare macro   name, cond
;        public  __&name
;__&name:
;        cmp     ax,bx
;        j&cond  true
;        xor     ax,ax   ; returns zero
;        ret
;        endm
;-------------------------- ASM version
compare macro   name, cond
        public  __?1
__?1:
        cmp     ax,bx
        j?2     true
        xor     ax,ax   ; returns zero
        ret
        endm
;--------------------------
        compare ult,a
        compare ugt,b
        compare ule,ae
        compare uge,be
        compare eq,e
        compare ne,ne
        compare lt,g
        compare gt,l
        compare le,ge
        compare ge,le

;
; Logical Negate of Primary
;
        public  __lneg
__lneg:
        or      ax,ax
        jnz     false
true:   mov     ax,1    ; returns one
        ret
false:  xor     ax,ax   ; returns zero
        ret
;
;
; execute "switch" statement
;
;  ax  =  switch value
; (sp) -> switch table
;         dw addr1, value1
;         dw addr2, value2
;         ...
;         dw 0
;        [jmp default]
;         continuation
;
        public  __switch
__switch:
        pop     bx              ; bx -> switch table
        jmp     skip            ; skip the pre-increment
back:
        add     bx,4
skip:   mov     cx,cs:[bx]
        jcxz    default         ; end of table -- jump out
        cmp     ax,cs:[bx+2]
        jnz     back
        jmp     cx              ; match -- jump to case
default:
        inc     bx
        inc     bx
        jmp     bx              ; jump to default/continuation
;
; dummy entry point to resolve the external reference _LINK
; which is no longer generated by Small-C but which exists in
; library modules and .OBJ files compiled by earlier versions
; of Small-C
        public  __link
__link: ret

code ends
        end     start

>>> CALLOC.C 315
#include "stdio.h"
/*
** Cleared-memory allocation of n items of size bytes.
** n     = Number of items to allocate space for.
** size  = Size of the items in bytes.
** Returns the address of the allocated block,
** else NULL for failure.
*/
calloc(n, size) unsigned n, size; {
  return (_alloc(n*size, YES));
  }

>>> CLEARERR.C 152
#include "stdio.h"
#include "clib.h"
extern int _status[];
/*
** Clear error status for fd.
*/
clearerr(fd) int fd; {
  if(_mode(fd)) _clrerr(fd);
  }

>>> CSEEK.C 1539
#include "stdio.h"
#include "clib.h"
extern int _nextc[], _bufuse[];
/*
** Position fd to the 128-byte record indicated by
** "offset" relative to the point indicated by "base."
** 
**     BASE     OFFSET-RELATIVE-TO
**       0      first record
**       1      current record
**       2      end of file (last record + 1)
**              (offset should be minus)
**
** Returns NULL on success, else EOF.
*/
cseek(fd, offset, base) int fd, offset, base; {
  int newrec, oldrec, hi, lo;
  if(!_mode(fd) || !_bufuse[fd]) return (EOF);
  if(_adjust(fd)) return (EOF);
  switch (base) {
     case 0: newrec = offset;
             break;
     case 1: oldrec = ctell(fd);
             goto calc;
     case 2: hi = lo = 0;
             if(!_seek(FROM_END, fd, &hi, &lo)) return (EOF);
             oldrec = ((lo >> 7) & 511) | (hi << 9);
     calc:
             newrec = oldrec + offset;
             break;
    default: return (EOF);
    }
  lo = (newrec << 7);       /* convert newrec to long int */
  hi = (newrec >> 9) & 127;
  if(!_seek(FROM_BEG, fd, &hi, &lo)) return (EOF);
  _nextc[fd] = EOF;
  _clreof(fd);
  return (NULL);
  }

/*
** Position fd to the character indicated by
** "offset" within current 128-byte record.
** Must be on record boundary.
**
** Returns NULL on success, else EOF.
*/
cseekc(fd, offset) int fd, offset; {
  int hi, lo;
  if(!_mode(fd) || isatty(fd) ||
     ctellc(fd) || offset < 0 || offset > 127) return (EOF);
  hi = 0; lo = offset;
  if(!_seek(FROM_CUR, fd, &hi, &lo)) return (EOF);
  return (NULL);
  }

>>> CSYSLIB.C 9708
/*
** CSYSLIB -- System-Level Library Functions
*/

#include "stdio.h"
#include "clib.h"

/*
****************** System Variables ********************
*/

int
  _cnt=1,             /* arg count for main */
  _vec[20],           /* arg vectors for main */
  _status[MAXFILES] = {OPNBIT, OPNBIT, OPNBIT, OPNBIT, OPNBIT},
  _cons  [MAXFILES],  /* fast way to tell if file is the console */
  _nextc [MAXFILES] = {EOF, EOF, EOF, EOF, EOF},
  _bufuse[MAXFILES],  /* current buffer usage: NULL, EMPTY, IN, OUT */
  _bufsiz[MAXFILES],  /* size of buffer */
  _bufptr[MAXFILES],  /* aux buffer address */
  _bufnxt[MAXFILES],  /* address of next byte in buffer */
  _bufend[MAXFILES],  /* address of end-of-data in buffer */
  _bufeof[MAXFILES];  /* true if current buffer ends file */

char
 *_memptr,           /* pointer to free memory. */
  _arg1[]="*";       /* first arg for main */

/*
*************** System-Level Functions *****************
*/

/*
**  Process command line, allocate default buffer to each fd,
**  execute main(), and exit to DOS.  Must be executed with es=psp.
**  Small default buffers are allocated because a high price is paid for
**  byte-by-byte calls to DOS.  Tests gave these results for a simple
**  copy program:
**
**          chunk size       copy time in seconds
**              1                    36
**              5                    12
**             25                     6
**             50                     6
*/
_main() {
  int fd;
  _parse();
  for(fd = 0; fd < MAXFILES; ++fd) auxbuf(fd, 32);
  if(!isatty(stdin))  _bufuse[stdin]  = EMPTY;
  if(!isatty(stdout)) _bufuse[stdout] = EMPTY;
  main(_cnt, _vec);
  exit(0);
  }

/*
** Parse command line and setup argc and argv.
** Must be executed with es == psp
*/
_parse() {
  char *ptr;
#asm
  mov     cl,es:[80h]  ; get parameter string length
  mov     ch,0       
  push    cx           ; save it
  inc     cx
  push    cx           ; 1st __alloc() arg
  mov     ax,1
  push    ax           ; 2nd __alloc() arg
  call    __alloc      ; allocate zeroed memory for args
  add     sp,4
  mov     [bp-2],ax    ; ptr = addr of allocated memory
  pop     cx
  push    es           ; exchange
  push    ds           ; es         (source)
  pop     es           ;    and
  pop     ds           ;        ds  (destination)
  mov     si,81h       ; source offset
  mov     di,[bp-2]    ; destination offset
  rep     movsb        ; move string
  mov     al,0
  stosb                ; terminate with null byte
  push    es
  pop     ds           ; restore ds
#endasm
  _vec[0]=_arg1;       /* first arg = "*" */
  while (*ptr) {
    if(isspace(*ptr)) {++ptr; continue;}
    if(_cnt < 20) _vec[_cnt++] = ptr;
    while(*ptr) {
      if(isspace(*ptr)) {*ptr = NULL; ++ptr; break;}
      ++ptr;
      }
    }
  }

/*
** Open file on specified fd.
*/
_open(fn, mode, fd) char *fn, *mode; int *fd; {
  int rw, tfd;
  switch(mode[0]) {
    case 'r': {
      if(mode[1] == '+') rw = 2; else rw = 0;
      if ((tfd = _bdos2((OPEN<<8)|rw, NULL, NULL, fn)) < 0) return (NO);
      break;
      }
    case 'w': {
      if(mode[1] == '+') rw = 2; else rw = 1;
    create:
      if((tfd = _bdos2((CREATE<<8), NULL, ARCHIVE, fn)) < 0) return (NO);
      _bdos2(CLOSE<<8, tfd, NULL, NULL);
      if((tfd = _bdos2((OPEN<<8)|rw, NULL, NULL, fn)) < 0) return (NO);
      break;
      }
    case 'a': {
      if(mode[1] == '+') rw = 2; else rw = 1;
      if((tfd = _bdos2((OPEN<<8)|rw, NULL, NULL, fn)) < 0) goto create;
      if(_bdos2((SEEK<<8)|FROM_END, tfd, NULL, 0) < 0) return (NO);
      break;
      }
    default: return (NO);
    }
  _empty(tfd, YES);
  if(isatty(tfd)) _bufuse[tfd] = NULL;
  *fd = tfd;
  _cons  [tfd] = NULL;
  _nextc [tfd] = EOF;
  _status[tfd] = OPNBIT;
  return (YES);
  }

/*
** Binary-stream input of one byte from fd.
*/
_read(fd) int fd; {
  unsigned char ch;
  if(_nextc[fd] != EOF) {
    ch = _nextc[fd];
    _nextc[fd] = EOF;
    return (ch);
    }
  if(iscons(fd))  return (_getkey());
  if(_bufuse[fd]) return(_readbuf(fd));
  switch(_bdos2(READ<<8, fd, 1, &ch)) {
     case 1:  return (ch);
     case 0: _seteof(fd); return (EOF);
    default: _seterr(fd); return (EOF);
    }
  }

/*
** Fill buffer if necessary, and return next byte.
*/
_readbuf(fd) int fd; {
  int got, chunk;
  char *ptr, *max;
  if(_bufuse[fd] == OUT && _flush(fd)) return (EOF);
  while(YES) {
    ptr = _bufnxt[fd];
    if(ptr < _bufend[fd]) {++_bufnxt[fd]; return (*ptr);}
    if(_bufeof[fd]) {_seteof(fd); return (EOF);}
    max = (ptr = _bufend[fd] = _bufptr[fd]) + _bufsiz[fd];
    do {         /* avoid DMA problem on physical 64K boundary */
      if((max - ptr) < 512) chunk = max - ptr;
      else                  chunk = 512;
      ptr += (got = _bdos2(READ<<8, fd, chunk, ptr));
      if(got < chunk) {_bufeof[fd] = YES; break;}
      } while(ptr < max);
    _bufend[fd] = ptr;
    _bufnxt[fd] = _bufptr[fd];
    _bufuse[fd] = IN;
    }
  }

/*
** Binary-Stream output of one byte to fd.
*/
_write(ch, fd) int ch, fd; {
  if(_bufuse[fd]) return(_writebuf(ch, fd));
  if(_bdos2(WRITE<<8, fd, 1, &ch) != 1) {
    _seterr(fd);
    return (EOF);
    }
  return (ch);
  }

/*
** Empty buffer if necessary, and store ch in buffer.
*/
_writebuf(ch, fd) int ch, fd; {
  char *ptr;
  if(_bufuse[fd] == IN && _backup(fd)) return (EOF);
  while(YES) {
    ptr = _bufnxt[fd];
    if(ptr < (_bufptr[fd] + _bufsiz[fd])) {
      *ptr = ch;
      ++_bufnxt[fd];
      _bufuse[fd] = OUT;
      return (ch);
      }
    if(_flush(fd)) return (EOF);
    }
  }

/*
** Flush buffer to DOS if dirty buffer.
** Reset buffer pointers in any case.
*/
_flush(fd) int fd; {
  int i, j, k, chunk;
  if(_bufuse[fd] == OUT) {
    i = _bufnxt[fd] - _bufptr[fd];
    k = 0;
    while(i > 0) {     /* avoid DMA problem on physical 64K boundary */
      if(i < 512) chunk = i;
      else        chunk = 512;
      k += (j = _bdos2(WRITE<<8, fd, chunk, _bufptr[fd] + k));
      if(j < chunk) {_seterr(fd); return (EOF);}
      i -= j;
      }
    }
  _empty(fd, YES);
  return (NULL);
  }

/*
** Adjust DOS file position to current point.
*/
_adjust(fd) int fd; {
  if(_bufuse[fd] == OUT) return (_flush(fd));
  if(_bufuse[fd] == IN ) return (_backup(fd));
  }

/*
** Backup DOS file position to current point.
*/
_backup(fd) int fd; {
  int hi, lo;
  if(lo = _bufnxt[fd] - _bufend[fd]) {
    hi = -1;
    if(!_seek(FROM_CUR, fd, &hi, &lo)) {
      _seterr(fd);
      return (EOF);
      }
    }
  _empty(fd, YES);
  return (NULL);
  }

/*
** Set buffer controls to empty status.
*/
_empty(fd, mt) int fd, mt; {
  _bufnxt[fd] = _bufend[fd] = _bufptr[fd];
  _bufeof[fd] = NO;
  if(mt) _bufuse[fd] = EMPTY;
  }

/*
** Return fd's open mode, else NULL.
*/
_mode(fd) char *fd; {
  if(fd < MAXFILES) return (_status[fd]);
  return (NULL);
  }

/*
** Set eof status for fd and
*/
_seteof(fd) int fd; {
  _status[fd] |= EOFBIT;
  }

/*
** Clear eof status for fd.
*/
_clreof(fd) int fd; {
  _status[fd] &= ~EOFBIT;
  }

/*
** Set error status for fd.
*/
_seterr(fd) int fd; {
  _status[fd] |= ERRBIT;
  }

/*
** Clear error status for fd.
*/
_clrerr(fd) int fd; {
  _status[fd] &= ~ERRBIT;
  }

/*
** Allocate n bytes of (possibly zeroed) memory.
** Entry: n = Size of the items in bytes.
**    clear = "true" if clearing is desired.
** Returns the address of the allocated block of memory
** or NULL if the requested amount of space is not available.
*/
_alloc(n, clear) unsigned n, clear; {
  char *oldptr;
  if(n < avail(YES)) {
    if(clear) pad(_memptr, NULL, n);
    oldptr = _memptr;
    _memptr += n;
    return (oldptr);
    }
  return (NULL);
  }

/*
** Issue extended BDOS function and return result. 
** Entry: ax = function code and sub-function
**        bx, cx, dx = other parameters
*/
_bdos2(ax, bx, cx, dx) int ax, bx, cx, dx; {
#asm
  push bx         ; preserve secondary register
  mov  dx,[bp+4]
  mov  cx,[bp+6]
  mov  bx,[bp+8]
  mov  ax,[bp+10] ; load DOS function number
  int  21h        ; call bdos
  jnc  __bdos21   ; no error
  neg  ax         ; make error code negative  
__bdos21:
  pop  bx         ; restore secondary register
#endasm
  }

/*
** Issue LSEEK call
*/
_seek(org, fd, hi, lo) int org, fd, hi, lo; {
#asm
  push bx         ; preserve secondary register
  mov  bx,[bp+4]
  mov  dx,[bx]    ; get lo part of destination
  mov  bx,[bp+6]
  mov  cx,[bx]    ; get hi part of destination
  mov  bx,[bp+8]  ; get file descriptor
  mov  al,[bp+10] ; get origin code for seek
  mov  ah,42h     ; move-file-pointer function
  int  21h        ; call bdos
  jnc  __seek1    ; error?
  xor  ax,ax      ; yes, return false
  jmp  __seek2 
__seek1:          ; no, set hi and lo
  mov  bx,[bp+4]  ; get address of lo
  mov  [bx],ax    ; store low part of new position
  mov  bx,[bp+6]  ; get address of hi
  mov  [bx],dx    ; store high part of new position
  mov  ax,1       ; return true
__seek2:
  pop  bx         ; restore secondary register
#endasm
  }

/*
** Test for keyboard input
*/
_hitkey() {
#asm
  mov  ah,1       ; sub-service = test keyboard
  int  16h        ; call bdos keyboard services
  jnz  __hit1
  xor ax,ax       ; nothing there, return false
  jmp  __hit2
__hit1:
  mov  ax,1       ; character ready, return true
__hit2:
#endasm
  }

/*
** Return next keyboard character
*/
_getkey() {
#asm
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
#endasm
  }

>>> CTELL.C 566
#include "stdio.h"
#include "clib.h"
extern int _bufuse[];
/*
** Return offset to current 128-byte record.
*/
ctell(fd) int fd; {
  int hi, lo;
  if(!_mode(fd) || !_bufuse[fd]) return (-1);
  if(_adjust(fd)) return (-1);
  hi = lo = 0;
  _seek(FROM_CUR, fd, &hi, &lo);
  return ((hi << 9) | ((lo >> 7) & 511));
  }

/*
** Return offset to next byte in current 128-byte record.
*/
ctellc(fd) int fd; {
  int hi, lo;
  if(!_mode(fd) || !_bufuse[fd]) return (-1);
  if(_adjust(fd)) return (-1);
  hi = lo = 0;
  _seek(FROM_CUR, fd, &hi, &lo);
  return (lo & 127);
  }

>>> DTOI.C 370
#include "stdio.h"
/*
** dtoi -- convert signed decimal string to integer nbr
**         returns field length, else ERR on error
*/
dtoi(decstr, nbr)  char *decstr;  int *nbr;  {
  int len, s;
  if((*decstr)=='-') {s=1; ++decstr;} else s=0;
  if((len=utoi(decstr, nbr))<0) return ERR;
  if(*nbr<0) return ERR;
  if(s) {*nbr = -*nbr; return ++len;} else return len;
  }

>>> EXIT.C 447
#include "stdio.h"
#include "clib.h"
/*
** Close all open files and exit to DOS. 
** Entry: ec = exit code.
*/
exit(ec) int ec; {
  int fd;  char str[4];
  ec &= 255;
  if(ec) {
    left(itou(ec, str, 4));
    fputs("Exit Code: ", stderr);
    fputs(str, stderr);
    fputs("\n", stderr);
    }
  for(fd = 0; fd < MAXFILES; ++fd) fclose(fd);
  _bdos2((RETDOS<<8)|ec, NULL, NULL, NULL);
#asm
_abort: jmp    _exit
        public _abort
#endasm
  }

>>> FCLOSE.C 336
#include "stdio.h"
#include "clib.h"
/*
** Close fd 
** Entry: fd = file descriptor for file to be closed.
** Returns NULL for success, otherwise ERR
*/
extern int _status[];
fclose(fd) int fd; {
  if(!_mode(fd) || _flush(fd)) return (ERR);
  if(_bdos2(CLOSE<<8, fd, NULL, NULL) == -6) return (ERR);
  return (_status[fd] = NULL);
  }

>>> FEOF.C 214
#include "clib.h"
extern int _status[];
/*
** Test for end-of-file status.
** Entry: fd = file descriptor
** Returns non-zero if fd is at eof, else zero.
*/
feof(fd) int fd; {
  return (_status[fd] & EOFBIT);
  }

>>> FERROR.C 152
#include "stdio.h"
#include "clib.h"
extern _status[];
/*
** Test for error status on fd.
*/
ferror(fd) int fd; {
  return (_status[fd] & ERRBIT);
  }

>>> FGETC.C 1083
#include "stdio.h"
#include "clib.h"

extern int _nextc[];

/*
** Character-stream input of one character from fd.
** Entry: fd = File descriptor of pertinent file.
** Returns the next character on success, else EOF.
*/
fgetc(fd) int fd; {
  int ch;                   /* must be int so EOF will flow through */
  if(_nextc[fd] != EOF) {   /* an ungotten byte pending? */
    ch = _nextc[fd];
    _nextc[fd] = EOF;
    return (ch & 255);      /* was cooked the first time */
    }
  while(1) {
    ch = _read(fd);
    if(iscons(fd)) {
      switch(ch) {          /* extra console cooking */
        case ABORT:  exit(2);
        case    CR: _write(CR, stderr); _write(LF, stderr); break;
        case   DEL:  ch = RUB;
        case   RUB:
        case  WIPE:  break;
        default:    _write(ch, stderr);
        }
      }
    switch(ch) {            /* normal cooking */
          default:  return (ch);
      case DOSEOF: _seteof(fd); return (EOF);
      case     CR:  return ('\n');
      case     LF:
      }
    }
  }
#asm
_getc:  jmp     _fgetc
        public  _getc
#endasm

>>> FGETS.C 1659
#include "stdio.h"
#include "clib.h"
/*
** Gets an entire string (including its newline
** terminator) or size-1 characters, whichever comes
** first. The input is terminated by a null character.
** Entry: str  = Pointer to destination buffer.
**        size = Size of the destination buffer.
**        fd   = File descriptor of pertinent file.
** Returns str on success, else NULL.
*/
fgets(str, size, fd) char *str; unsigned size, fd; {
  return (_gets(str, size, fd, 1));
  }

/*
** Gets an entire string from stdin (excluding its newline
** terminator) or size-1 characters, whichever comes
** first. The input is terminated by a null character.
** The user buffer must be large enough to hold the data.
** Entry: str  = Pointer to destination buffer.
** Returns str on success, else NULL.
*/
gets(str) char *str; {
  return (_gets(str, 32767, stdin, 0));
  }

_gets(str, size, fd, nl) char *str; unsigned size, fd, nl; {
  int backup; char *next;
  next = str;
  while(--size > 0) {
    switch (*next = fgetc(fd)) {
      case  EOF: *next = NULL;
                 if(next == str) return (NULL);
                 return (str);
      case '\n': *(next + nl) = NULL;
                 return (str);
      case  RUB: if(next > str) backup = 1; else backup = 0;
                 goto backout;
      case WIPE: backup = next - str;
        backout: if(iscons(fd)) {
                   ++size;
                   while(backup--) {
                     fputs("\b \b", stderr);
                     --next; ++size;
                     }
                   continue;
                   }
        default: ++next;
      }
    }
  *next = NULL;
  return (str);
  }

>>> FOPEN.C 485
#include "stdio.h"
#include "clib.h"
/*
** Open file indicated by fn.
** Entry: fn   = Null-terminated DOS file name.
**        mode = "a"  - append
**               "r"  - read
**               "w"  - write
**               "a+" - append update
**               "r+" - read   update
**               "w+" - write  update
** Returns a file descriptor on success, else NULL.
*/
fopen(fn, mode) char *fn, *mode; {
  int fd;
  if(!_open(fn, mode, &fd)) return (NULL);
  return (fd);
  }

>>> FPRINTF.C 2133
#include "stdio.h"
/*
** fprintf(fd, ctlstring, arg, arg, ...) - Formatted print.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
fprintf(argc) int argc; {
  int *nxtarg;
  nxtarg = CCARGC() + &argc;
  return(_print(*(--nxtarg), --nxtarg));
  }

/*
** printf(ctlstring, arg, arg, ...) - Formatted print.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
printf(argc) int argc; {
  return(_print(stdout, CCARGC() + &argc - 1));
  }

/*
** _print(fd, ctlstring, arg, arg, ...)
** Called by fprintf() and printf().
*/
_print(fd, nxtarg) int fd, *nxtarg; {
  int  arg, left, pad, cc, len, maxchr, width;
  char *ctl, *sptr, str[17];
  cc = 0;                                         
  ctl = *nxtarg--;                          
  while(*ctl) {
    if(*ctl!='%') {fputc(*ctl++, fd); ++cc; continue;}
    else ++ctl;
    if(*ctl=='%') {fputc(*ctl++, fd); ++cc; continue;}
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
    if(!left) while(width--) {fputc(pad,fd); ++cc;}
    while(len--) {fputc(*sptr++,fd); ++cc; }
    if(left) while(width--) {fputc(pad,fd); ++cc;}  
    }
  return(cc);
  }

>>> FPUTC.C 544
#include "stdio.h"
#include "clib.h"
extern int _status[];
/*
** Character-stream output of a character to fd.
** Entry: ch = Character to write.
**        fd = File descriptor of perinent file.
** Returns character written on success, else EOF.
*/
fputc(ch, fd) int ch, fd; {
  switch(ch) {
    case  EOF: _write(DOSEOF, fd); break;
    case '\n': _write(CR, fd); _write(LF, fd); break;
      default: _write(ch, fd);
    }
  if(_status[fd] & ERRBIT) return (EOF);
  return (ch);
  }
#asm
_putc:  jmp     _fputc
        public  _putc
#endasm

>>> FPUTS.C 264
#include "stdio.h"
#include "clib.h"
/*
** Write a string to fd. 
** Entry: string = Pointer to null-terminated string.
**        fd     = File descriptor of pertinent file.
*/
fputs(string, fd) char *string; int fd; {
  while(*string) fputc(*string++, fd) ;
  }

>>> FREAD.C 887
#include "clib.h"
extern int _status[];
/*
** Item-stream read from fd.
** Entry: buf = address of target buffer
**         sz = size of items in bytes
**          n = number of items to read
**         fd = file descriptor
** Returns a count of the items actually read.
** Use feof() and ferror() to determine file status.
*/
fread(buf, sz, n, fd) unsigned char *buf; unsigned sz, n, fd; {
  return (read(fd, buf, n*sz)/sz);
  }

/*
** Binary-stream read from fd.
** Entry:  fd = file descriptor
**        buf = address of target buffer
**          n = number of bytes to read
** Returns a count of the bytes actually read.
** Use feof() and ferror() to determine file status.
*/
read(fd, buf, n) unsigned fd, n; unsigned char *buf; {
  unsigned cnt;
  cnt = 0;
  while(n--) {
    *buf++ = _read(fd);
    if(_status[fd] & (ERRBIT | EOFBIT)) break;
    ++cnt;
    }
  return (cnt);
  }

>>> FREE.C 374
extern char *_memptr;
/*
** free(ptr) - Free previously allocated memory block.
** Memory must be freed in the reverse order from which
** it was allocated.
** ptr    = Value returned by calloc() or malloc().
** Returns ptr if successful or NULL otherwise.
*/
free(ptr) char *ptr; {
   return (_memptr = ptr);
   }
#asm
_cfree: jmp     _free
        public  _cfree
#endasm

>>> FREOPEN.C 799
#include <stdio.h>
#include "clib.h"
/*
** Close previously opened fd and reopen it. 
** Entry: fn   = Null-terminated DOS file name.
**        mode = "a"  - append
**               "r"  - read
**               "w"  - write
**               "a+" - append update
**               "r+" - read   update
**               "w+" - write  update
**        fd   = File descriptor of pertinent file.
** Returns the original fd on success, else NULL.
*/
extern int _status[];
freopen(fn, mode, fd) char *fn, *mode; int fd; {
  int tfd;
  if(fclose(fd)) return (NULL);
  if(!_open(fn, mode, &tfd)) return (NULL);
  if(fd != tfd) {
    if(_bdos2(FORCE<<8, tfd, fd, NULL) < 0) return (NULL);
    _status[fd] = _status[tfd];
    _status[tfd] = 0;       /* leaves DOS using two handles */
    }
  return (fd);
  }

>>> FSCANF.C 2479
#include "stdio.h"
/*
** fscanf(fd, ctlstring, arg, arg, ...) - Formatted read.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
fscanf(argc) int argc; {
  int *nxtarg;
  nxtarg = CCARGC() + &argc;
  return (_scan(*(--nxtarg), --nxtarg));
  }

/*
** scanf(ctlstring, arg, arg, ...) - Formatted read.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
scanf(argc) int argc; {
  return (_scan(stdin, CCARGC() + &argc - 1));
  }

/*
** _scan(fd, ctlstring, arg, arg, ...) - Formatted read.
** Called by fscanf() and scanf().
*/
_scan(fd,nxtarg) int fd, *nxtarg; {
  char *carg, *ctl;
  unsigned u;
  int  *narg, wast, ac, width, ch, cnv, base, ovfl, sign;
  ac = 0;
  ctl = *nxtarg--;
  while(*ctl) {
    if(isspace(*ctl)) {++ctl; continue;}
    if(*ctl++ != '%') continue;
    if(*ctl == '*') {narg = carg = &wast; ++ctl;}
    else             narg = carg = *nxtarg--;
    ctl += utoi(ctl, &width);
    if(!width) width = 32767;
    if(!(cnv = *ctl++)) break;
    while(isspace(ch = fgetc(fd))) ;
    if(ch == EOF) {if(ac) break; else return(EOF);}
    ungetc(ch,fd);
    switch(cnv) {
      case 'c':
        *carg = fgetc(fd);
        break;
      case 's':
        while(width--) {
          if((*carg = fgetc(fd)) == EOF) break;
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
        while(width-- && !isspace(ch=fgetc(fd)) && ch!=EOF) {
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

>>> FWRITE.C 959
#include "clib.h"
extern int _status[];
/*
** Item-stream write to fd.
** Entry: buf = address of source buffer
**         sz = size of items in bytes
**          n = number of items to write
**         fd = file descriptor
** Returns a count of the items actually written or
** zero if an error occurred.
** May use ferror(), as always, to detect errors.
*/
fwrite(buf, sz, n, fd) unsigned char *buf; unsigned sz, n, fd; {
  if(write(fd, buf, n*sz) == -1) return (0);
  return (n);
  }

/*
** Binary-stream write to fd.
** Entry:  fd = file descriptor
**        buf = address of source buffer
**          n = number of bytes to write
** Returns a count of the bytes actually written or
** -1 if an error occurred.
** May use ferror(), as always, to detect errors.
*/
write(fd, buf, n) unsigned fd, n; unsigned char *buf; {
  unsigned cnt;
  cnt = n;
  while(cnt--) {
    _write(*buf++, fd);
    if(_status[fd] & ERRBIT) return (-1);
    }
  return (n);
  }

>>> GETARG.C 626
#include "stdio.h"
/*
** Get command line argument. 
** Entry: n    = Number of the argument.
**        s    = Destination string pointer.
**        size = Size of destination string.
**        argc = Argument count from main().
**        argv = Argument vector(s) from main().
** Returns number of characters moved on success,
** else EOF.
*/
getarg(n, s, size, argc, argv)
  int n; char *s; int size, argc, argv[]; {
  char *str;
  int i;
  if(n < 0 | n >= argc) {
    *s = NULL;
    return EOF;
    }
  i = 0;
  str=argv[n];
  while(i<size) {
    if((s[i]=str[i])==NULL) break;
    ++i;
    }
  s[i]=NULL;
  return i;
  }

>>> GETCHAR.C 111
#include "stdio.h"
/*
** Get next character from standard input. 
*/
getchar() {
  return (fgetc(stdin));
  }

>>> IS.C 2057
/*
** All character classification functions except isascii().
** Integer argument (c) must be in ASCII range (0-127) for
** dependable answers.
*/

#define ALNUM     1
#define ALPHA     2
#define CNTRL     4
#define DIGIT     8
#define GRAPH    16
#define LOWER    32
#define PRINT    64
#define PUNCT   128
#define BLANK   256
#define UPPER   512
#define XDIGIT 1024

int _is[128] = {
 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x004, 0x104, 0x104, 0x104, 0x104, 0x104, 0x004, 0x004,
 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x140, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
 0x459, 0x459, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
 0x253, 0x253, 0x253, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
 0x073, 0x073, 0x073, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x004
 };

isalnum (c) int c; {return (_is[c] & ALNUM );} /* 'a'-'z', 'A'-'Z', '0'-'9' */
isalpha (c) int c; {return (_is[c] & ALPHA );} /* 'a'-'z', 'A'-'Z' */
iscntrl (c) int c; {return (_is[c] & CNTRL );} /* 0-31, 127 */
isdigit (c) int c; {return (_is[c] & DIGIT );} /* '0'-'9' */
isgraph (c) int c; {return (_is[c] & GRAPH );} /* '!'-'~' */
islower (c) int c; {return (_is[c] & LOWER );} /* 'a'-'z' */
isprint (c) int c; {return (_is[c] & PRINT );} /* ' '-'~' */
ispunct (c) int c; {return (_is[c] & PUNCT );} /* !alnum && !cntrl && !space */
isspace (c) int c; {return (_is[c] & BLANK );} /* HT, LF, VT, FF, CR, ' ' */
isupper (c) int c; {return (_is[c] & UPPER );} /* 'A'-'Z' */
isxdigit(c) int c; {return (_is[c] & XDIGIT);} /* '0'-'9', 'a'-'f', 'A'-'F' */

>>> ISASCII.C 108
/*
** return 'true' if c is an ASCII character (0-127)
*/
isascii(c) unsigned c; {
  return (c < 128);
  }

>>> ISATTY.C 374
/*
** Return "true" if fd is a device, else "false"
*/
isatty(fd) int fd; {
fd;               /* fetch handle */
#asm
  push bx         ; save 2nd reg
  mov  bx,ax      ; place handle
  mov  ax,4400h   ; ioctl get info function
  int 21h         ; call BDOS
  pop  bx         ; restore 2nd reg
  mov  ax,dx      ; fetch info bits
  and  ax,80h     ; isdev bit
#endasm
  }


>>> ISCONS.C 769
/*
** Determine if fd is the console.
*/
#include <stdio.h>
extern int _cons[];

iscons(fd) int fd; {
  if(_cons[fd] == NULL) {
    if(_iscons(fd)) _cons[fd] = 2;
    else            _cons[fd] = 1;
    }
  if(_cons[fd] == 1) return (NO);
  return (YES);
  }

/*
** Call DOS only the first time for a file.
*/
_iscons(fd) int fd; {
  fd;             /* fetch handle */
#asm
  push bx         ; save 2nd reg
  mov  bx,ax      ; place handle
  mov  ax,4400h   ; ioctl get info function
  int 21h         ; call BDOS
  pop  bx         ; restore 2nd reg
  mov  ax,dx      ; fetch info bits
  and  ax,83h     ; keep device and console bits
  cmp  ax,80h     ; device and console?
  jg   __cons1
  xor  ax,ax      ; return false if not device and console
__cons1:
#endasm
  }
>>> ITOA.C 276
/*
** itoa(n,s) - Convert n to characters in s 
*/
itoa(n, s) char *s; int n; {
  int sign;
  char *ptr;
  ptr = s;
  if ((sign = n) < 0) n = -n;
  do {
    *ptr++ = n % 10 + '0';
    } while ((n = n / 10) > 0);
  if (sign < 0) *ptr++ = '-';
  *ptr = '\0';
  reverse(s);
  }

>>> ITOAB.C 425
/*
** itoab(n,s,b) - Convert "unsigned" n to characters in s using base b.
**                NOTE: This is a non-standard function.
*/
itoab(n, s, b) int n; char *s; int b; {
  char *ptr;
  int lowbit;
  ptr = s;
  b >>= 1;
  do {
    lowbit = n & 1;
    n = (n >> 1) & 32767;
    *ptr = ((n % b) << 1) + lowbit;
    if(*ptr < 10) *ptr += '0'; else *ptr += 55;
    ++ptr;
    } while(n /= b);
  *ptr = 0;
  reverse (s);
  }

>>> ITOD.C 623
#include "stdio.h"
/*
** itod -- convert nbr to signed decimal string of width sz
**         right adjusted, blank filled; returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itod(nbr, str, sz)  int nbr;  char str[];  int sz;  {
  char sgn;
  if(nbr<0) {nbr = -nbr; sgn='-';}
  else sgn=' ';
  if(sz>0) str[--sz]=NULL;
  else if(sz<0) sz = -sz;
  else while(str[sz]!=NULL) ++sz;
  while(sz) {
    str[--sz]=(nbr%10+'0');
    if((nbr=nbr/10)==0) break;
    }
  if(sz) str[--sz]=sgn;
  while(sz>0) str[--sz]=' ';
  return str;
  }

>>> ITOO.C 541
/*
** itoo -- converts nbr to octal string of length sz
**         right adjusted and blank filled, returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itoo(nbr, str, sz)  int nbr;  char str[];  int sz;  {
  int digit;
  if(sz>0) str[--sz]=0;
  else if(sz<0) sz = -sz;
  else while(str[sz]!=0) ++sz;
  while(sz) {
    digit=nbr&7; nbr=(nbr>>3)&8191;
    str[--sz]=digit+48;
    if(nbr==0) break;
    }
  while(sz) str[--sz]=' ';
  return str;
  }

>>> ITOU.C 621
#include "stdio.h"
/*
** itou -- convert nbr to unsigned decimal string of width sz
**         right adjusted, blank filled; returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itou(nbr, str, sz)  int nbr;  char str[];  int sz;  {
  int lowbit;
  if(sz>0) str[--sz]=NULL;
  else if(sz<0) sz = -sz;
  else while(str[sz]!=NULL) ++sz;
  while(sz) {
    lowbit=nbr&1;
    nbr=(nbr>>1)&32767;  /* divide by 2 */
    str[--sz]=((nbr%5)<<1)+lowbit+'0';
    if((nbr=nbr/5)==0) break;
    }
  while(sz) str[--sz]=' ';
  return str;
  }

>>> ITOX.C 596
/*
** itox -- converts nbr to hex string of length sz
**         right adjusted and blank filled, returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itox(nbr, str, sz)  int nbr;  char str[];  int sz;  {
  int digit, offset;
  if(sz>0) str[--sz]=0;
  else if(sz<0) sz = -sz;
  else while(str[sz]!=0) ++sz;
  while(sz) {
    digit=nbr&15; nbr=(nbr>>4)&4095;
    if(digit<10) offset=48; else offset=55;
    str[--sz]=digit+offset;
    if(nbr==0) break;
    }
  while(sz) str[--sz]=' ';
  return str;
  }

>>> LEFT.C 166
/*
** left -- left adjust and null terminate a string
*/
left(str) char *str; {
  char *str2;
  str2=str;
  while(*str2==' ') ++str2;
  while(*str++ = *str2++);
  }

>>> LEXCMP.C 1380

char _lex[128] = {
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  /* NUL - /       */
      10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
      30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
      40, 41, 42, 43, 44, 45, 46, 47,
      65, 66, 67, 68, 69, 70, 71, 72, 73, 74,  /* 0-9           */
      48, 49, 50, 51, 52, 53, 54,              /* : ; < = > ? @ */
      75, 76, 77, 78, 79, 80, 81, 82, 83, 84,  /* A-Z           */
      85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
      95, 96, 97, 98, 99,100,
      55, 56, 57, 58, 59, 60,                  /* [ \ ] ^ _ `   */
      75, 76, 77, 78, 79, 80, 81, 82, 83, 84,  /* a-z           */
      85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
      95, 96, 97, 98, 99,100,
      61, 62, 63, 64,                          /* { | } ~       */
     127};                                     /* DEL           */

/*
** lexcmp(s, t) - Return a number <0, 0, or >0
**                as s is <, =, or > t.
*/
lexcmp(s, t) char *s, *t; {
  while(lexorder(*s, *t) == 0)
    if(*s++) ++t;
    else return (0);
  return (lexorder(*s, *t));
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
lexorder(c1, c2) int c1, c2; {
  return(_lex[c1] - _lex[c2]);
  }

>>> MALLOC.C 237
#include "stdio.h"
/*
** Memory allocation of size bytes.
** size  = Size of the block in bytes.
** Returns the address of the allocated block,
** else NULL for failure.
*/
malloc(size) unsigned size; {
  return (_alloc(size, NO));
  }

>>> OTOI.C 368
#include "stdio.h"
/*
** otoi -- convert unsigned octal string to integer nbr
**          returns field size, else ERR on error
*/
otoi(octstr, nbr)  char *octstr;  int *nbr;  {
  int d,t; d=0;
  *nbr=0;
  while((*octstr>='0')&(*octstr<='7')) {
    t=*nbr;
    t=(t<<3) + (*octstr++ - '0');
    if ((t>=0)&(*nbr<0)) return ERR;
    d++; *nbr=t;
    }
  return d;
  }

>>> PAD.C 123
/*
** Place n occurrences of ch at dest.
*/
pad(dest, ch, n) char *dest; unsigned n, ch; {
  while(n--) *dest++ = ch;
  }

>>> POLL.C 390
#include "stdio.h"
#include "clib.h"
/*
** Poll for console input or interruption
*/
poll(pause) int pause; {
  int i;
  if(i = _hitkey())  i = _getkey();
  if(pause) {
    if(i == PAUSE) {
      i = _getkey();           /* wait for next character */
      if(i == ABORT) exit(2);  /* indicate abnormal exit */
      return (0);
      }
    if(i == ABORT) exit(2);
    }
  return (i);
  }

>>> PUTCHAR.C 122
#include "stdio.h"
/*
** Write character to standard output. 
*/
putchar(ch) int ch; {
  return (fputc(ch, stdout));
  }

>>> PUTS.C 144
#include "stdio.h"
/*
** Write string to standard output. 
*/
puts(string) char *string; {
  fputs(string, stdout);
  fputc('\n', stdout);
  }

>>> RENAME.C 711
#include "stdio.h"
#include "clib.h"
/*
** Rename a file.
**  from = address of old filename.
**    to = address of new filename.
**  Returns NULL on success, else ERR.
*/
rename(from, to) char *from, *to; {
  if(_rename(from, to)) return (NULL);
  return (ERR);
  }

_rename(old, new) char *old, *new; {
#asm
  push ds         ; ds:dx points to old name
  pop  es         ; es:di points to new name
  mov  di,[bp+4]  ; get "new" offset
  mov  dx,[bp+6]  ; get "old" offset
  mov  ah,56h     ; rename function
  int  21h        ; call bdos
  jnc  __ren1     ; error?
  xor  ax,ax      ; yes, return false
  jmp  __ren2 
__ren1:           ; no, set hi and lo
  mov  ax,1       ; return true
__ren2:
#endasm
  }

>>> REVERSE.C 170
/*
** reverse string in place 
*/
reverse(s) char *s; {
  char *j;
  int c;
  j = s + strlen(s) - 1;
  while(s < j) {
    c = *s;
    *s++ = *j;
    *j-- = c;
    }
  }

>>> REWIND.C 89
/*
** Rewind file to beginning. 
*/
rewind(fd) int fd; {
  return(cseek(fd, 0, 0));
  }

>>> SIGN.C 154
/*
** sign -- return -1, 0, +1 depending on the sign of nbr
*/
sign(nbr)  int nbr;  {
  if(nbr > 0)  return 1;
  if(nbr == 0) return 0;
  return -1;
  }

>>> STRCAT.C 177
/*
** concatenate t to end of s 
** s must be large enough
*/
strcat(s, t) char *s, *t; {
  char *d;
  d = s;
  --s;
  while (*++s) ;
  while (*s++ = *t++) ;
  return (d);
  }

>>> STRCHR.C 177
/*
** return pointer to 1st occurrence of c in str, else 0
*/
strchr(str, c) char *str, c; {
  while(*str) {
    if(*str == c) return (str);
    ++str;
    }
  return (0);
  }

>>> STRCMP.C 185
/*
** return <0,   0,  >0 a_ording to
**       s<t, s=t, s>t
*/
strcmp(s, t) char *s, *t; {
  while(*s == *t) {
    if(*s == 0) return (0);
    ++s; ++t;
    }
  return (*s - *t);
  }

>>> STRCPY.C 113
/*
** copy t to s 
*/
strcpy(s, t) char *s, *t; {
  char *d;
  d = s;
  while (*s++ = *t++) ;
  return (d);
  }

>>> STRLEN.C 357
/*
** return length of string s (fast version)
*/
strlen(s) char *s; {
  #asm
  xor al,al        ; set search value to zero
  mov cx,65535     ; set huge maximum
  mov di,[bp+4]    ; get address of s
  cld              ; set direction flag forward
  repne scasb      ; scan for zero
  mov ax,65534
  sub ax,cx        ; calc and return length
  #endasm
  }

>>> STRNCAT.C 255
/*
** concatenate n bytes max from t to end of s 
** s must be large enough
*/
strncat(s, t, n) char *s, *t; int n; {
  char *d;
  d = s;
  --s;
  while(*++s) ;
  while(n--) {
    if(*s++ = *t++) continue;
    return(d);
    }
  *s = 0;
  return(d);
  }

>>> STRNCMP.C 333
/*
** strncmp(s,t,n) - Compares two strings for at most n
**                  characters and returns an integer
**                  >0, =0, or <0 as s is >t, =t, or <t.
*/
strncmp(s, t, n) char *s, *t; int n; {
  while(n && *s==*t) {
    if (*s == 0) return (0);
    ++s; ++t; --n;
    }
  if(n) return (*s - *t);
  return (0);
  }

>>> STRNCPY.C 253
/*
** copy n characters from sour to dest (null padding)
*/
strncpy(dest, sour, n) char *dest, *sour; int n; {
  char *d;
  d = dest;
  while(n-- > 0) {
    if(*d++ = *sour++) continue;
    while(n-- > 0) *d++ = 0;
    }
  *d = 0;
  return (dest);
  }

>>> STRRCHR.C 315
/*
** strrchr(s,c) - Search s for rightmost occurrance of c.
** s      = Pointer to string to be searched.
** c      = Character to search for.
** Returns pointer to rightmost c or NULL.
*/
strrchr(s, c) char *s, c; {
  char *ptr;
  ptr = 0;
  while(*s) {
    if(*s==c) ptr = s;
    ++s;
    }
  return (ptr);
  }

>>> TOASCII.C 77
/*
** return ASCII equivalent of c
*/
toascii(c) int c; {
  return (c);
  }

>>> TOLOWER.C 131
/*
** return lower-case of c if upper-case, else c
*/
tolower(c) int c; {
  if(c<='Z' && c>='A') return (c+32);
  return (c);
  }

>>> TOUPPER.C 137
/*
** return upper-case of c if it is lower-case, else c
*/
toupper(c) int c; {
  if(c<='z' && c>='a') return (c-32);
  return (c);
  }

>>> UNGETC.C 295
#include "stdio.h"
extern _nextc[];
/*
** Put c back into file fd.
** Entry:  c = character to put back
**        fd = file descriptor
** Returns c if successful, else EOF.
*/
ungetc(c, fd) int c, fd; {
  if(!_mode(fd) || _nextc[fd]!=EOF || c==EOF) return (EOF);
  return (_nextc[fd] = c);
  }

>>> UNLINK.C 473
#include "stdio.h"
#include "clib.h"
/*
** Unlink (delete) the named file. 
** Entry: fn = Null-terminated DOS file path\name.
** Returns NULL on success, else ERR.
*/
unlink(fn) char *fn; {
  fn;           /* load fn into ax */
#asm
  mov dx,ax     ; put fn in its place
  mov ah,41h    ; delete function code
  int 21h
  mov ax,0
  jnc __unlk    ; return NULL
  mov ax,-2     ; return ERR
__unlk:
#endasm
  }
#asm
_delete: jmp    _unlink
         public _delete
#endasm

>>> UTOI.C 365
#include "stdio.h"
/*
** utoi -- convert unsigned decimal string to integer nbr
**          returns field size, else ERR on error
*/
utoi(decstr, nbr)  char *decstr;  int *nbr;  {
  int d,t; d=0;
  *nbr=0;
  while((*decstr>='0')&(*decstr<='9')) {
    t=*nbr;t=(10*t) + (*decstr++ - '0');
    if ((t>=0)&(*nbr<0)) return ERR;
    d++; *nbr=t;
    }
  return d;
  }

>>> XTOI.C 729
#include stdio.h
/*
** xtoi -- convert hex string to integer nbr
**         returns field size, else ERR on error
*/
xtoi(hexstr, nbr) char *hexstr; int *nbr; {
  int d, b;  char *cp;
  d = *nbr = 0; cp = hexstr;
  while(*cp == '0') ++cp;
  while(1) {
    switch(*cp) {
      case '0': case '1': case '2':
      case '3': case '4': case '5':
      case '6': case '7': case '8':
      case '9':                     b=48; break;
      case 'A': case 'B': case 'C':
      case 'D': case 'E': case 'F': b=55; break;
      case 'a': case 'b': case 'c':
      case 'd': case 'e': case 'f': b=87; break;
       default: return (cp - hexstr);
      }
    if(d < 4) ++d; else return (ERR);
    *nbr = (*nbr << 4) + (*cp++ - b);
    }
  }
