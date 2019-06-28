/*                                   VAL.C                                  */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  This program is released to the public domain by the author on         |
  |  February 15, 1989 at the Software Development Conference,              |
  |  San Francisco, CA.  With the understanding that there are no           |
  |  warranties of any sort either expressed or implied, you may use the    |
  |  code in whole or part for any lawful purpose.  The author does not     |
  |  warrant or guarantee that this code is fit for any particular purpose. |
  |  You are under no obligation to acknowledge the author when using this  |
  |  code, but you may do so if you desire.  The author neither seeks nor   |
  |  desires any contributions for this code.                               |
  |                                                                         |
  |  Should you desire to communicate with the author, the preferred method |
  |  is via the author's bulletin board, Pontchippi.  Pontchippi is an      |
  |  OPUS board.  You may contact it directly at 504-244-1417 or via net    |
  |  mail to node 390/105.  The board has a 9600 baud US Robotics Courier   |
  |  HST modem.  Should you contact the board directly, access to the board |
  |  is granted immediately up completion of a very simple questionnaire.   |
  |                                                                         |
  |  The author wishes to acknowledge and thank Jack Purdum and Tim Leslie  |
  |  of Ecosoft for their assistance and support.  In addition to Jack and  |
  |  Tim, Steve Russell of SLR Systems was also of invaluable assistance.   |
  |  All helped in this project by debugging and offering their expert      |
  |  advice and guidance.  They always had time for me seemingly whenever   |
  |  I requested it.                                                        |
  |                                                                         |
  |  Finally, this program is dedicated to the memory of my father-in-law,  |
  |  Mariano "Val" Valentino.  The only request the author makes for        |
  |  using the program is that the program name be left as "VAL" in his     |
  |  memory.                                                                |
  |                                                                         |
  |  David Troendle                                                         |
  |  New Orleans, LA                                                        |
  |  February 10, 1989                                                      |
  |                                                                         |
  |  TURBO-C 2.0 Compile command: TCC -ms VAL.C ASMSUBS.C                   |
  +-------------------------------------------------------------------------+*/

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
#include "globals.h"

#include "dictary.c"
#include "endlink.c"
#include "execfile.c"
#include "files.c"
#include "fixup.c"
#include "initlink.c"
#include "library.c"
#include "linkerio.c"
#include "list.c"
#include "main.c"
#include "map.c"
#include "memory.c"
#include "object.c"
#include "order.c"
#include "string.c"
#include "timer.c"
#include "tmodule.c"
#include "token.c"
#include "userinp.c"
