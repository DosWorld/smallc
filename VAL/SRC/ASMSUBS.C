/*                                ASMSUBS.C                                 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <string.h>

#include "langext.h"
#include "defines.h"
#include "types.h"
#include "subs.h"

#pragma inline

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               checksum                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 checksum(bit_16 len, byte *sym)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 For i=0; i LessThan len; i++
  BeginFor
   sum += sym[i];
  EndFor;
 return(sum); */

 asm            mov     si,sym
 asm            mov     cx,len
 asm            xor     ax,ax
 asm            mov     bx,ax
hash_loop:
 asm            mov     bl,[si]
 asm            add     ax,bx
 asm            inc     si
 asm            loop    hash_loop
 return(_AX);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_compare                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
int_16 far_compare (byte_ptr left, byte_ptr right, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   If *left IsNotEqualTo *right
    Then
     If *left LessThan *right
      Then
       return(-1);
      Else
       return(1);
      EndIf;
    EndIf;
   left++;
   right++;
  EndWhile;
 return(0); */

 asm            cld
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,right
 asm            lds     si,left
 asm    rep     cmpsb
 asm            pop     ds
 asm            jb      left_less_than_right
 asm            ja      left_greater_than_right

 return(0);
left_less_than_right:
 return(-1);
left_greater_than_right:
 return(1);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            far_index                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 far_index(byte_ptr dest, byte c)
BeginDeclarations
bit_16                                 i;
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 For i=0; (*dest++ IsNotEqualTo c) AndIf (i IsNotEqualTo 0xFFFF); i++
  BeginFor
  EndFor;
 return(i); */

 asm            xor     cx,cx
 asm            mov     al,c
 asm            les     di,dest
search_loop:
 asm            cmp     al,es:[di]
 asm            je      search_done
 asm            inc     di
 asm            inc     cx
 asm            jne     search_loop
 asm            dec     cx
search_done:
 asm            mov     i,cx

 return(i);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              far_match                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 far_match (byte_ptr pattern, byte_ptr s, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len Exceeds 0
  BeginWhile
   If *pattern Is '*'
    Then
     return(True);
    EndIf;
   If (*pattern IsNot '?') AndIf (*pattern IsNot *source)
    Then
     return(False);
    EndIf;
   source++;
   pattern++;
  EndWhile;
 return(True); */

 asm            mov     cx,len
 asm            les     di,pattern
 asm            push    ds
 asm            lds     si,s
pattern_loop:
 asm            mov     al,es:[di]
 asm            cmp     al,'?'
 asm            je      a_match
 asm            cmp     al,'*'
 asm            je      succeeded
 asm            cmp     al,[si]
 asm            jne     failed
a_match:
 asm            inc     si
 asm            inc     di
 asm            loop    pattern_loop
succeeded:
 asm            pop     ds
 return(True);
failed:
 asm            pop     ds
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move                                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move (byte_ptr dest, byte_ptr source, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   *dest++ = *source++;
  EndWhile; */

 asm            cld
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,dest
 asm            lds     si,source
 asm    rep     movsb
 asm            pop     ds

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_move_left                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_move_left (byte_ptr dest, byte_ptr source, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 dest   += len - 1;
 source += len - 1;
 While len-- IsNotZero
  BeginWhile
   *dest-- = *source--;
  EndWhile; */

 asm            std
 asm            push    ds
 asm            mov     cx,len
 asm            les     di,dest
 asm            lds     si,source
 asm            add     di,cx
 asm            add     si,cx
 asm            dec     si
 asm            dec     di
 asm    rep     movsb
 asm            pop     ds

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                far_set                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_set (byte_ptr dest, byte source, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   *dest++ = source;
  EndWhile; */

 asm            cld
 asm            mov     cx,len
 asm            les     di,dest
 asm            mov     al,source
 asm            stosb
 asm            dec     cx
 asm    rep     stosb

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          far_to_lower                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
byte_ptr far_to_lower (byte_ptr dest, bit_16 len)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
 While len-- IsNotZero
  BeginWhile
   tolower(*dest++);
  EndWhile; */

 asm            mov     cx,len
 asm            les     di,dest
lower_case_loop:
 asm            mov     al,es:[di]
 asm            cmp     al,'A'
 asm            jb      next_byte
 asm            cmp     al,'Z'
 asm            ja      next_byte
 asm            add     al,'a'-'A'
 asm            mov     es:[di],al
next_byte:
 asm            inc     di
 asm            loop    lower_case_loop

 return(dest);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         library_directory_hash                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void library_directory_hash(byte_ptr      sym,
                            bit_16        len,
                            bit_16       *starting_block,
                            bit_16       *delta_block,
                            bit_16       *starting_entry,
                            bit_16       *delta_entry)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
BeginDeclarations
byte                                  *beg_str;
byte                                  *end_str;
bit_16                                 c;
byte                                   temp[33];
EndDeclarations
BeginCode
 beg_str = temp;
 end_str = Addr(temp[len]);
 temp[0] = Byte(len);
 far_move(BytePtr(Addr(temp[1]), sym, len);
 *starting_block =
 *delta_block    =
 *starting_entry =
 *delta_entry    = 0;
 While len-- Exceeds 0
  BeginWhile
   c = (Bit_16(*beg_str++) And 0xFF) Or 0x20;
   *starting_block = c Xor ((*starting_block ShiftedLeft   2)  Or
                            (*starting_block ShiftedRight 14));
   *delta_entry    = c Xor ((*delta_entry    ShiftedRight  2)  Or
                            (*delta_entry    ShiftedLeft  14));
   c = (Bit_16(*end_str--) And 0xFF) Or 0x20;
   *delta_block    = c Xor ((*delta_block    ShiftedLeft   2)  Or
                            (*delta_block    ShiftedRight 14));
   *starting_entry = c Xor ((*starting_entry ShiftedRight  2)  Or
                            (*starting_entry ShiftedLeft  14));
  EndWhile; */
 asm            xor     ax,ax
 asm            mov     cl,2
 asm            xor     bx,bx
 asm            xor     dx,dx
 asm            mov     bl,len
 asm            mov     dl,len
 asm            or      bx,0x20
 asm            or      dx,0x20
 asm            les     si,sym
 asm            mov     di,len
 asm            dec     di
up_loop:
 asm            mov     al,es:[si]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            inc     si
 asm            dec     di
 asm            jnz     up_loop
 asm            mov     di,starting_block
 asm            mov     [di],bx
 asm            mov     di,delta_entry
 asm            mov     [di],dx
 asm            mov     di,len
 asm            xor     bx,bx
 asm            xor     dx,dx
down_loop:
 asm            mov     al,es:[si]
 asm            or      al,0x20
 asm            rol     bx,cl
 asm            ror     dx,cl
 asm            xor     bx,ax
 asm            xor     dx,ax
 asm            dec     si
 asm            dec     di
 asm            jnz     down_loop
 asm            mov     di,delta_block
 asm            mov     [di],bx
 asm            mov     di,starting_entry
 asm            mov     [di],dx
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              word_checksum                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 word_checksum(bit_16 len, bit_16 address, byte_ptr data)
BeginDeclarations
EndDeclarations
BeginCode
/* The following assembly code is the equivalent of the following C code:
BeginDeclarations
Structure word_struct
 BeginStructure
  bit_8                                low_byte;
  bit_8                                high_byte;
 EndStructure;
bit_16                                 sum;
Structure word_struct                  word;
bit_16                                *word_ptr;
EndDeclarations
 word = (bit_16 *) Addr(word);
 For i=0; i LessThan len; i++
  BeginFor
   *word_ptr = 0;
   If (address-- And 1)
    Then
     word.high_byte = *data++;
    Else
     word.low_byte = *data++;
    EndIf;
   sum += *word_ptr;
  EndFor;
 return(sum); */

 asm            xor     ax,ax
 asm            les     di,data
 asm            mov     bx,address
 asm            mov     cx,len
/* Handle case where we are starting at an odd location */
 asm            and     bx,1
 asm            je      at_even_location
 asm            mov     ah,es:[di]
 asm            inc     di
 asm            dec     cx
at_even_location:
/* Loop while we have at least two bytes left */
 asm            cmp     cx,2
 asm            jb      at_end_of_sum
 asm            add     ax,es:[di]
 asm            inc     di
 asm            inc     di
 asm            dec     cx
 asm            loop    at_even_location
at_end_of_sum:
 asm            cmp     cx,1
 asm            jne     checksum_done
 asm            add     al,es:[di]
 asm            adc     ah,0
checksum_done:
 return(_AX);
EndCode
