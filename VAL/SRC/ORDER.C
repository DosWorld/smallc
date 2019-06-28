/*                                 ORDER.C                                 */

/*
  order_expression::  term   {('or'  | '+' | '|') term}
              term::  factor {('and' | '*' | '&') factor}
            factor::  ('!' | '~' | '-' | 'not')* primary
           primary::  'true'                       |
                      'false'                      |
                      ('segment' '[' name ']' )    |
                      ('class'   '[' name ']' )    |
                      ('group'   '[' name ']' )    |
                      ( '(' order_expression ')' )
*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          align_active_segment                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void align_active_segment()
BeginDeclarations
bit_32                                 gap;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_32                                 mask;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If (*Active_segment.lsegs.first).align IsZero
  Then /* Don't align absolute segments */
   return;
  EndIf;
 Active_segment.highest_uninitialized_byte = 0L;
 TraverseList(Active_segment.lsegs, lseg)
  BeginTraverse
   LoopIf(Lseg.align Is absolute_segment);
   If Active_segment.combine Is common_combine
    Then  /* Finally, we know how big the common area is, so allocate
             memory for it.  */
     Lseg.data = allocate_memory(Addr(static_pool), Lseg.length);
    EndIf;
   mask                    = align_mask[Lseg.align];
   gap                     = AlignmentGap(next_available_address, mask);
   next_available_address += gap;
   Lseg.address            = next_available_address;
   next_available_address += Lseg.length;
   If Lseg.highest_uninitialized_byte IsNotZero
    Then
     highest_uninitialized_byte                =
     Active_segment.highest_uninitialized_byte =
      Lseg.address + Lseg.highest_uninitialized_byte;
    EndIf;
  EndTraverse;
 Active_segment.address = (*Active_segment.lsegs.first).address;
 Active_segment.length  = (*Active_segment.lsegs.last).address +
                          (*Active_segment.lsegs.last).length -
                          Active_segment.address;
 If Active_segment.highest_uninitialized_byte IsZero
  Then
   Active_segment.highest_uninitialized_byte =
    (*Active_segment.lsegs.first).address;
  EndIf;
 If (Active_segment.owning_group IsNotNull) AndIf
    ((*Active_segment.owning_group).first_segment IsNull)
  Then
   (*Active_segment.owning_group).first_segment = active_segment;
  EndIf;
 If (DOSSEG.val IsTrue) AndIf 
    (Active_segment.owning_group IsNotNull) AndIf
    ((*Active_segment.owning_group).group_name Is DGROUP_lname)
  Then
   If (edata_segment IsNull) AndIf
      (Active_segment.class_name Is BSS_lname)
    Then
     edata_segment = active_segment;
     pub = lookup_public(6, (byte *) "_edata", 0);
     If (Pub.type_entry Is external) OrIf (Pub.type_entry Is unused)
      Then
       Pub.type_entry      = internal;
       Pub.Internal.group  = Active_segment.owning_group;
       Pub.Internal.lseg   = Active_segment.lsegs.first;
       Pub.Internal.frame  = 0;
       Pub.Internal.offset = 0;
      Else
       linker_error(4, "Could not generate symbol \"_edata\" "
                       "when \"/DOSSEG\" set\n"
                       "because it was explicitly defined.\n");
      EndIf;
    EndIf;
   If (end_segment IsNull) AndIf
      (Active_segment.class_name Is STACK_lname)
    Then
     end_segment = active_segment;
     pub = lookup_public(4, (byte *) "_end", 0);
     If (Pub.type_entry Is external) OrIf (Pub.type_entry Is unused)
      Then
       Pub.type_entry      = internal;
       Pub.Internal.group  = Active_segment.owning_group;
       Pub.Internal.lseg   = Active_segment.lsegs.first;
       Pub.Internal.frame  = 0;
       Pub.Internal.offset = 0;
      Else
       linker_error(4, "Could not generate symbol \"_end\" "
                       "when \"/DOSSEG\" set\n"
                       "because it was explicitly defined.\n");
      EndIf;
    EndIf;
  EndIf;
 return;
EndCode
#undef Lseg
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            get_order_token                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_order_token()
BeginDeclarations
EndDeclarations
BeginCode
 While token_break_char Is ' '
  BeginWhile
   order_token_get_char();
  EndWhile;
 copy_string(token, null_string);
 If IsIdentifier(token_break_char)
  Then
   While IsIdentifier(token_break_char)
    BeginWhile
     concat_char_to_string(token, token_break_char);
     order_token_get_char();
    EndWhile;
   lowercase_string(token);
  Else
   If token_break_char Is '['
    Then
     While token_break_char IsNot ']'
      BeginWhile
       concat_char_to_string(token, token_break_char);
       order_token_get_char();
      EndWhile;
     order_token_get_char();
     If case_ignore.val
      Then
       lowercase_string(token);
      EndIf;
    Else
     concat_char_to_string(token, token_break_char);
     order_token_get_char();
    EndIf;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         order_and_align_segments                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_and_align_segments()
BeginDeclarations
byte_ptr                               start_of_expression;
EndDeclarations
BeginCode
 order_start_time = Now;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  | Before we can start ordering segments, we have to get some housekeeping |
  | done first.                                                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 order_prologue();

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |              OK, lets get to ordering and aligning segments.            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 First(segments_ordered_list) =
 Last(segments_ordered_list)  = Null;
 highest_uninitialized_byte = 0L;
 start_of_expression = String(ordering.val);
 start_of_expression++;
 While *start_of_expression IsNot '\000'
  BeginWhile
   ExitIf(segment_list.first IsNull);
   First(segments_unordered_list) =
   Last(segments_unordered_list)  = Null;
   While segment_list.first IsNotNull
    BeginWhile
     Pop segment_list InTo active_segment EndPop;
     order_expression_char_ptr = start_of_expression;
     token_break_char = ' ';
     If codeview_information_present AndIf
        (((Active_segment.segment_name Is codeview_segment_TYPES) AndIf
          (Active_segment.class_name Is codeview_class_DEBTYP)) OrIf
         ((Active_segment.segment_name Is codeview_segment_SYMBOLS) AndIf
          (Active_segment.class_name Is codeview_class_DEBSYM)))
      Then /* Eat the codeview segment */
      Else /* Process all non-codeview segments */
       If order_expression()
        Then
         Insert active_segment AtEnd InList segments_ordered_list EndInsert;
         align_active_segment();
        Else
         Insert active_segment AtEnd InList segments_unordered_list EndInsert;
        EndIf;
      EndIf;
    EndWhile;
   start_of_expression = order_expression_char_ptr;
   start_of_expression--;
   segment_list = segments_unordered_list;
  EndWhile;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Make one final pass accepting all remaining segments.          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 While segment_list.first IsNotNull
  BeginWhile
   Pop segment_list InTo active_segment EndPop;
   Insert active_segment AtEnd InList segments_ordered_list EndInsert;
   align_active_segment();
  EndWhile;
 segment_list = segments_ordered_list;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            order_expression                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_expression()
BeginDeclarations
bit_16                                 left_operand;
EndDeclarations
BeginCode
 get_order_token();
 left_operand = order_term();
 While TokenIs(or_string)   OrIf
       TokenIs(plus_string) OrIf
       TokenIs(bar_string)
  BeginWhile
   get_order_token();
   left_operand |= order_term();
  EndWhile;
 return(left_operand);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_factor                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_factor()
BeginDeclarations
bit_16                                 unary_not;
EndDeclarations
BeginCode
 unary_not = False;
 While TokenIs(exclamation_string) OrIf
       TokenIs(tilde_string)       OrIf
       TokenIs(minus_string)       OrIf
       TokenIs(not_string)
  BeginWhile
   get_order_token();
   unary_not ^= True;
  EndWhile;
 return(unary_not ^ order_primary());
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             order_primary                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_primary()
BeginDeclarations
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 operand;
EndDeclarations
BeginCode
 If TokenIs(true_string)
  Then
   get_order_token();
   return(True);
  EndIf;
 If TokenIs(false_string)
  Then
   get_order_token();
   return(False);
  EndIf;
 If TokenIs(open_paren_string)
  Then
   operand = order_expression();
   If TokenIs(close_paren_string)
    Then
     get_order_token();
     return(operand);
    Else
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    EndIf;
  EndIf;
 If TokenStartsWith(segment_string)
  Then
   get_order_token();
   If *String(token) IsNot '['
    Then
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    EndIf;
   cut_string(token, 0, 1);
   operand = match_pattern(token,
                           string((*Active_segment.segment_name).symbol));
   get_order_token();
   return(operand);
  EndIf;
 If TokenStartsWith(group_string)
  Then
   get_order_token();
   If *String(token) IsNot '['
    Then
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    EndIf;
   cut_string(token, 0, 1);
   group = Active_segment.owning_group;
   If group IsNull
    Then
     operand = False;
    Else
     operand =
      match_pattern(token, string((*Group.group_name).symbol));
    EndIf;
   get_order_token();
   return(operand);
  EndIf;
 If TokenStartsWith(class_string)
  Then
   get_order_token();
   If *String(token) IsNot '['
    Then
     linker_error(8, "Expression syntax error:\n"
                     "\t\"%Fs\"\n",
                     String(ordering.val));
    EndIf;
   cut_string(token, 0, 1);
   operand = match_pattern(token,
                           string((*Active_segment.class_name).symbol));
   get_order_token();
   return(operand);
  EndIf;
 linker_error(8, "Expression syntax error:\n"
                 "\t\"%Fs\"\n",
                 String(ordering.val));
 return(False);
EndCode
#undef Group

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              order_prologue                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_prologue()
BeginDeclarations
group_entry_ptr                        group;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       next_communal;
bit_16                                 offset;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 size;
EndDeclarations
BeginCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  | Compute the address base for the executable image.  For .COM files this |
  | will be 100H, for .EXE and .SYS files, this will be 0.                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If comfile.val IsTrue
  Then
   address_base = 0x100L;
  Else
   address_base = 0L;
  EndIf;
 next_available_address = 0L;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |   The ordering expression for the segments must be known.  If the       |
  |   "/ORDER:(expression)" switch was specified, then it will be used      |
  |   regardless or whether or not "/DOSSEG" is set.  If the "/ORDER"       |
  |   switch was not specified and "/DOSSEG" was, then the appropriate      |
  |   expression is used.  Otherwise, the linker will just take the         |
  |   segments in the order they were encountered.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If ordering.val IsNull
  Then
   If DOSSEG.val IsTrue
    Then
     ordering.val = duplicate_string(Addr(static_pool),
      string((byte *) ("(seg[*code]|seg[*CODE], "
                       "!(gr[dgroup]|gr[DGROUP]), "
                       "cl[begdata]|cl[BEGDATA], "
                       "cl[*data]|cl[*DATA], "
                       "!(cl[*bss]|cl[*BSS]|cl[*stack]|cl[*STACK]), "
                       "cl[*bss]|cl[*BSS], "
                       "cl[*stack]|cl[*STACK])")));
    Else
     ordering.val = duplicate_string(Addr(static_pool),
                                     string((byte *) ("(true)")));
    EndIf;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Check to insure a stack segment of appropriate size is present.  For   |
  |  .COM and .SYS files, no stack segment is required.  For .EXE files,    |
  |  one is required.  For .EXE files, we must insure that a stack of at    |
  |  least the size specified in the "/STACK" switch is available.          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If exefile IsFalse
  Then
   If stack_segment_found IsTrue
    Then
     linker_error(4, "Stack segment found for a non .EXE file.\n");
    EndIf;
  Else
   If (stack_segment_found IsFalse) AndIf
      (stack.set           IsFalse)
    Then
     linker_error(4, "No stack segment for .EXE file.\n");
    Else
     If (stack.set IsTrue) AndIf 
        (Bit_16(Largest_stack_seg.length) LessThan stack.val)
      Then
       obj_generate_segment(generated_lname,
                            none_lname,
                            stack_combine,
                            2,              /* word aligned */
                            none_lname,
                            exe_file_list.first,
                            0L,             /* not absolute segment */
                            Bit_32(stack.val));
      EndIf;
    EndIf;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle near communals as follows:  If there is at least one near       |
  |  communal, create the segment "c_common" in group "DGROUP" with         |
  |  class "BSS".  Then, for each communal place it in "c_common" in        |
  |  ascending order while changing the dictionary entry from               |
  |  "near_communal" to "internal".                                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If near_communals IsNotNull
  Then
   length = 0L;
   For pub=near_communals; pub IsNotNull; pub=Pub.Communal.next_communal
    BeginFor
     LoopIf(Pub.type_entry IsNot near_communal);
     length += Pub.Communal.element_size;
    EndFor;
   If length Exceeds 65536L
    Then
     linker_error(8, "Near communal size exceeds 64K by %lu bytes.\n",
                     length-65536L);
    EndIf;
   lseg = obj_generate_segment(c_common_lname,
                               BSS_lname,
                               blank_common_combine,
                               3,              /* paragraph aligned */
                               none_lname,
                               exe_file_list.first,
                               0L,             /* not absolute segment */
                               length);
   Lseg.highest_uninitialized_byte = length;
   group                        = lookup_group(DGROUP_lname);
   (*Lseg.segment).owning_group = group;
   offset = 0;
   For pub=near_communals; pub IsNotNull; pub=next_communal
    BeginFor
     next_communal        = Pub.Communal.next_communal;
     LoopIf(Pub.type_entry IsNot near_communal);
     size                 = Bit_16(Pub.Communal.element_size);
     Pub.type_entry       = internal;
     Pub.Internal.group   = group;
     Pub.Internal.lseg    = lseg;
     Pub.Internal.offset  = offset;
     Pub.Internal.frame   = 0;
     offset              += size;
    EndFor;
  EndIf;
 
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle far communals as follows:  Packing as many far communals as     |
  |  will fit (64K), create private segments with the segment and class     |
  |  name of "FAR_BSS".  They do NOT go in DRGROUP.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 lseg   = Null;
 offset = 0;
 For pub=far_communals; pub IsNotNull; pub=next_communal
  BeginFor
   next_communal = Pub.Communal.next_communal;
   LoopIf(Pub.type_entry IsNot far_communal); 
   length = Pub.Communal.element_size * Pub.Communal.element_count;
   If length Exceeds 65536L
    Then
     Pub.Communal.next_communal = huge_communals;
     huge_communals             = pub;
     ContinueLoop;
    EndIf;
   If (lseg IsNull) OrIf ((length + Bit_32(offset)) Exceeds 65536L)
    Then
     lseg = obj_generate_segment(FAR_BSS_lname,
                                 FAR_BSS_lname,
                                 blank_common_combine,
                                 3,              /* paragraph aligned */
                                 none_lname,
                                 exe_file_list.first,
                                 0L,             /* not absolute segment */
                                 0L);
     offset = 0;
    EndIf;
   Pub.type_entry                   = internal;
   Pub.Internal.group               = Null;
   Pub.Internal.lseg                = lseg;
   Pub.Internal.offset              = offset;
   Pub.Internal.frame               = 0;
   offset                          += Bit_16(length);
   Lseg.highest_uninitialized_byte += length;
   Lseg.length                     += length;
   (*Lseg.segment).length          += length;
  EndFor;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |  Handle huge communals as follows:  Taking as many segments as          |
  |  required, create private segments with the segment and class           |
  |  name of "HUGE_BSS".  They do NOT go in DRGROUP.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 For pub=huge_communals; pub IsNotNull; pub=next_communal
  BeginFor
   next_communal = Pub.Communal.next_communal;
   length = Pub.Communal.element_size * Pub.Communal.element_count;
   lseg = obj_generate_segment(HUGE_BSS_lname,
                               HUGE_BSS_lname,
                               blank_common_combine,
                               3,              /* paragraph aligned */
                               none_lname,
                               exe_file_list.first,
                               0L,             /* not absolute segment */
                               0L);
   If Pub.Communal.element_size Exceeds 65536L
    Then
     linker_error(4, "Communal \"%Fs\" has element size exceeding 64K.\n",
                     Pub.symbol);
     offset = 0;
    Else
     offset = Bit_16(65536L Mod Pub.Communal.element_size);
    EndIf;
   length                          += Bit_32(offset);
   Pub.type_entry                   = internal;
   Pub.Internal.group               = Null;
   Pub.Internal.lseg                = lseg;
   Pub.Internal.offset              = offset;
   Pub.Internal.frame               = 0;
   Lseg.highest_uninitialized_byte += length;
   Lseg.length                     += length;
   (*Lseg.segment).length          += length;
  EndFor;

 return;
EndCode
#undef Pub
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               order_term                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 order_term()
BeginDeclarations
bit_16                                 left_operand;
EndDeclarations
BeginCode
 left_operand = order_factor();
 While TokenIs(and_string)       OrIf
       TokenIs(star_string)      OrIf
       TokenIs(ampersand_string)
  BeginWhile
   get_order_token();
   left_operand &= order_factor();
  EndWhile;
 return(left_operand);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           order_token_get_char                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void order_token_get_char()
BeginDeclarations
EndDeclarations
BeginCode
 If token_break_char Is '\000'
  Then
   linker_error(8, "Expression syntax error:\n"
                   "\t\"%Fs\"\n",
                   String(ordering.val));
  EndIf;
 token_break_char = *order_expression_char_ptr++;
 return;
EndCode
