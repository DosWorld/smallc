/*                                 TOKEN.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       complete_a_filename_token                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void complete_a_filename_token()
BeginDeclarations
EndDeclarations
BeginCode
 copy_string(token, next_token);
 scan_out_token();
 If token_type Is switch_end_token_type
  Then
   concat_string(token, next_token);
   scan_out_token();
   If token_type Is filename_token_type
    Then
     concat_string(token, next_token);
     scan_out_token();
    EndIf;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          eat_white_space                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void eat_white_space()
BeginDeclarations
EndDeclarations
BeginCode
 While token_break_char Is ' '
  BeginWhile
   token_get_char();
  EndWhile;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     get_free_token_source_element                       |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
token_stack_ptr get_free_token_source_element()
BeginDeclarations
token_stack_ptr                        elem;
EndDeclarations
BeginCode
 Pop token_stack_free_list InTo elem EndPop;
 If elem IsNull
  Then
   elem = (token_stack_ptr) 
           allocate_memory(Addr(static_pool),
                           Bit_32(sizeof(token_stack_type)));
  EndIf;
 return(elem);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        get_filename_token                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void get_filename_token(bit_16            required,
                        file_info_list   *list)
BeginDeclarations
#define List                           (*list)
token_stack_ptr                        source_element;
#define Source_element                 (*source_element)
EndDeclarations
BeginCode
 Loop
  BeginLoop
   required = (required) AndIf (List.first IsNull);
   If token_type Is text_token_type
    Then
     linker_error(8, "Input syntax error:  \"%Fs\" out of place.\n",
                     String(next_token));
    EndIf;
   If token_type Is indirect_file_token_type
    Then
     scan_out_token();
     If token_type IsNot filename_token_type
      Then
       linker_error(8,"Input syntax error:  Expected filename after '@'.\n");
      EndIf;
     complete_a_filename_token();
     source_element                    = get_free_token_source_element();
     Source_element.break_char         = token_break_char;
     Source_element.source_file        = input_open(token);
     Source_element.token_string       = Null;
     Source_element.token_string_index = 0;
     Push source_element OnTo token_stack EndPush;
     token_break_char                  = ' ';
     scan_out_token();
     ContinueLoop;
    EndIf;
   If token_type Is switch_token_type
    Then
     process_switch();
     ContinueLoop;
    EndIf;
   If token_type Is continuation_token_type
    Then
     scan_out_token();
     If token_type Is line_end_token_type
      Then
       scan_out_token();
      EndIf;
     ContinueLoop;
    EndIf;
   If token_type Is filename_token_type
    Then
     complete_a_filename_token();
     more_tokens = True;
     return;
    EndIf;
   If ((token_type Is end_of_command_line_token_type)  AndIf
       (List.first IsNull))                            OrIf
      ((required)                                      AndIf
       ((token_type Is separator_token_type)           OrIf
        (token_type Is line_end_token_type)))
    Then
     If (*token_stack.first).source_file IsNot stdin
      Then
       source_element                    = get_free_token_source_element();
       Source_element.source_file        = stdin;
       Source_element.token_string       = Null;
       Source_element.token_string_index = 0;
       Push source_element OnTo token_stack EndPush;
       token_break_char                  = ' ';
      EndIf;
     If default_prompt IsNotNull
      Then
       linker_message(default_prompt, String(default_filename));
       prompt_next_stdin = False;
       default_prompt    = Null;
      EndIf;
     scan_out_token();
     ContinueLoop;
    EndIf;
   If List.first IsNull
    Then
     If (token_type Is separator_token_type)  OrIf
        (token_type Is terminator_token_type)
      Then
       If prompting_for Is 2
        Then
         If comfile.val IsTrue
          Then
           default_extension = com_extension_string;
          Else
           If sysfile.val IsTrue
            Then
             default_extension = sys_extension_string;
            Else
             default_extension = exe_extension_string;
            EndIf;
           EndIf;
         change_extension(default_filename, default_extension);
        EndIf;
       copy_string(token, default_filename);
       more_tokens = False;
       return;
      EndIf;
    Else
     If (token_type Is separator_token_type)           OrIf
        (token_type Is terminator_token_type)          OrIf
        (token_type Is end_of_command_line_token_type)
      Then
       copy_string(token, null_string);
       more_tokens = False;
       return;
     EndIf;
    EndIf;
   If token_type Is line_end_token_type
    Then
     If List.first IsNull
      Then
       If prompting_for Is 2
        Then
         If comfile.val IsTrue
          Then
           default_extension = com_extension_string;
          Else
           If sysfile.val IsTrue
            Then
             default_extension = sys_extension_string;
            Else
             default_extension = exe_extension_string;
            EndIf;
           EndIf;
         change_extension(default_filename, default_extension);
        EndIf;
       copy_string(token, default_filename);
      Else
       copy_string(token, null_string);
      EndIf;
     If (*token_stack.first).source_file Is stdin
      Then
       Pop token_stack InTo source_element EndPop;
       Push source_element OnTo token_stack_free_list EndPush;
      EndIf;
     prompt_next_stdin = False;
     more_tokens = False;
     return;
    EndIf;
  EndLoop;
 return;
EndCode
#undef List
#undef Source_element

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             input_open                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
FILE *input_open(string_ptr fn)
BeginDeclarations
FILE                                  *infile;
EndDeclarations
BeginCode
 infile = fopen((char *)near_string(fn), "r");
 If infile Is NULL
  Then
   linker_error(8, "Could not open file \"%Fs\" for input.\n",
                   String(fn));
  EndIf;
 return(infile);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             process_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_switch()
BeginDeclarations
switch_table_ptr                       current_switch;
#define Current_switch                 (*current_switch)
EndDeclarations
BeginCode
 current_switch = switch_table;
 scan_out_token();
 copy_string(token, next_token);
 lowercase_string(token);
 While Current_switch.full_name IsNotNull
  BeginWhile
   If compare_string(token, string((byte *)Current_switch.abbr_name)) IsZero
    Then
     Current_switch.switch_processor(current_switch);
     return;
    EndIf;
   If (Length(token) NotLessThan Current_switch.min_length) AndIf
      (far_compare(String(token), (byte *) Current_switch.full_name,
        Length(token)) IsZero)
    Then
     Current_switch.switch_processor(current_switch);
     return;
    EndIf;
   current_switch++;
  EndWhile;
 linker_error(8,"Syntax error:  \"%Fs\" is an unknown switch.\n",
                String(token));
 return;
EndCode
#undef Current_switch

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_bit_16_switch                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_bit_16_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 If token_type IsNot switch_end_token_type
  Then
   linker_error(8,"Syntax error:  \":\" did not follow switch \"%s\"\n",
                  Current_switch.full_name);
  EndIf;
 scan_out_token();
 copy_string(token, next_token);
  If (Not token_is_number)                                OrIf
     (token_numeric_value LessThan    Affected_thing.min) OrIf
     (token_numeric_value GreaterThan Affected_thing.max)
   Then
   linker_error(8,"Syntax error:  Switch \"%s\" requires a numeric value\n"
                  "               between %u and %u\n",
                  Current_switch.full_name,
                  Affected_thing.min, Affected_thing.max);
   Else
    Affected_thing.val = token_numeric_value;
    Affected_thing.set = True;
   EndIf;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             scan_opt_bit_16                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_opt_bit_16(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 Affected_thing.set = True;
 If token_type IsNot switch_end_token_type
  Then
   Affected_thing.val = Affected_thing.def;
   return;
  EndIf;
 scan_out_token();
 copy_string(token, next_token);
  If (Not token_is_number)                                OrIf
     (token_numeric_value LessThan    Affected_thing.min) OrIf
     (token_numeric_value GreaterThan Affected_thing.max)
   Then
   linker_error(8,"Syntax error:  Switch \"%s\" requires a numeric value\n"
                  "               between %u and %u\n",
                  Current_switch.full_name,
                  Affected_thing.min, Affected_thing.max);
   Else
    Affected_thing.val = token_numeric_value;
   EndIf;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         scan_out_token                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_out_token()
BeginDeclarations
bit_16                                 paren_count;
EndDeclarations
BeginCode
 eat_white_space();
 copy_string(next_token, null_string);
 token_is_hex_number =
 token_is_number     = False;
 Using token_break_char
  BeginCase
   When '\n':
    prompt_next_stdin = True;
    concat_char_to_string(next_token, token_break_char);
    If token_stack.first Is token_stack.last
     Then
      token_type       = end_of_command_line_token_type;
     Else
      token_type       = line_end_token_type;
     EndIf;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When ',':
    concat_char_to_string(next_token, token_break_char);
    token_type       = separator_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When ';':
    concat_char_to_string(next_token, token_break_char);
    token_type       = terminator_token_type;
    break;
   When '+':
    concat_char_to_string(next_token, token_break_char);
    token_type       = continuation_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When '/':
    concat_char_to_string(next_token, token_break_char);
    token_type       = switch_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When ':':
    concat_char_to_string(next_token, token_break_char);
    token_type       = switch_end_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When '@':
    concat_char_to_string(next_token, token_break_char);
    token_type       = indirect_file_token_type;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   When '(':
    paren_count = 1;
    token_type  = text_token_type;
    concat_char_to_string(next_token, token_break_char);
    While paren_count IsNotZero
     BeginWhile
      token_get_char();
      If token_break_char IsNot '\n'
       Then
        concat_char_to_string(next_token, token_break_char);
       Else
        If (*token_stack.first).source_file Is stdin
         Then
          linker_message("continue parenthesized text:  ");
         EndIf;
       EndIf;
      If token_break_char Is '('
       Then
        paren_count++;
       EndIf;
      If token_break_char Is ')'
       Then
        paren_count--;
       EndIf;
     EndWhile;
    token_break_char = ' ';  /* Make it look like we advanced a character */
    break;
   Otherwise:
    token_is_number     = True;
    token_numeric_value = 0;
    While (token_break_char IsNot ',')  AndIf
          (token_break_char IsNot ';')  AndIf
          (token_break_char IsNot '+')  AndIf
          (token_break_char IsNot '/')  AndIf
          (token_break_char IsNot '@')  AndIf
          (token_break_char IsNot ':')  AndIf
          (token_break_char IsNot ' ')  AndIf
          (token_break_char IsNot '\n')
     BeginWhile
      concat_char_to_string(next_token, token_break_char);
      If (Length(next_token) Is 2) AndIf (String(next_token)[0] Is '0') AndIf
         ((String(next_token)[1] Is 'x') OrIf (String(next_token)[1] Is 'X'))
       Then
        token_is_hex_number = True;
       Else
        If token_is_hex_number IsFalse
         Then
          token_is_number = token_is_number AndIf
                            isdigit(token_break_char);
          If token_is_number
           Then
            token_numeric_value = (token_numeric_value * 10) +
                                  Bit_16(token_break_char - '0');
           EndIf;
         Else
          token_is_hex_number =
          token_is_number     = token_is_number AndIf
                                isxdigit(token_break_char);
          If token_is_number
           Then
            If isdigit(token_break_char)
             Then
              token_numeric_value = (token_numeric_value * 16) +
                                    Bit_16(token_break_char - '0');
             Else
              token_numeric_value = (token_numeric_value * 16) +
                                 Bit_16(toupper(token_break_char) - 'A' + 10);
             EndIf;
           EndIf;
         EndIf;
       EndIf;
      token_get_char();
     EndWhile;
    token_type = filename_token_type;
    break;
  EndCase;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_help_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_help_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
FILE                                  *help_file;
EndDeclarations
BeginCode
 Affected_thing.val = True;
 help_file = fopen(CharPtr(near_string(help_filename)), "r");
 If help_file IsNull
  Then
   printf("Could not open help file \"%Fs\".\n", String(help_filename));
  Else
   While fgets(CharPtr(object_file_element), MAX_ELEMENT_SIZE, help_file) 
         IsNotNull
    BeginWhile
     If *CharPtr(object_file_element) Is '\f'
       Then
        linker_message("Press [RETURN] to continue...");
        gets(CharPtr(object_file_element));
       Else
        linker_message(CharPtr(object_file_element));
      EndIf;
    EndWhile;
   fclose(help_file);
  EndIf;
 exit(0);
 return;
EndCode
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_reset_bit_16                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_reset_bit_16(switch_table_ptr current_switch)
BeginDeclarations
#define Affected_thing (*((bit_16_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 Affected_thing.set = False;
 Affected_thing.val = Affected_thing.def;
 return;
EndCode
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           scan_reset_switch                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_reset_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 Affected_thing.val = False;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            scan_set_switch                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_set_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((boolean_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 Affected_thing.val = True;
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Current_switch
#undef Affected_thing

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            scan_text_switch                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void scan_text_switch(switch_table_ptr current_switch)
BeginDeclarations
#define Current_switch                 (*current_switch)
#define Affected_thing (*((text_switch_ptr)(*current_switch).affected_thing))
EndDeclarations
BeginCode
 scan_out_token();
 copy_string(token, next_token);
 If token_type IsNot switch_end_token_type
  Then
   linker_error(8,"Syntax error:  \":\" did not follow switch \"%s\"\n",
                  Current_switch.full_name);
  EndIf;
 scan_out_token();
 copy_string(token, next_token);
 If token_type IsNot text_token_type
  Then
   linker_error(8, "Syntax error:  Parenthesized text did not follow\n"
                   "\t\"%s\" switch.  Instead found \"%Fs\".\n",
                  Current_switch.full_name, String(token));
  EndIf;
 Affected_thing.val = duplicate_string(Addr(static_pool), next_token);
 scan_out_token();
 copy_string(token, next_token);
 return;
EndCode
#undef Affected_thing
#undef Current_switch

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           token_get_char                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void token_get_char()
BeginDeclarations
int_16                                 c;
token_stack_ptr                        tos;
#define Tos                            (*tos)
#define Tos_string                     (*tos).token_string
EndDeclarations
BeginCode
 Loop
  BeginLoop
   If ((*token_stack.first).source_file Is stdin) AndIf
      (prompt_next_stdin)
    Then
     linker_message("continue:  ");
     prompt_next_stdin = False;
    EndIf;
   tos = token_stack.first;
   If tos IsNull
    Then
     token_break_char = ';';
     return;
    EndIf;
   If Tos.source_file IsNull
    Then /* Input is from a string */
     If Tos.token_string_index LessThan Length(Tos_string)
      Then
       token_break_char = String(Tos_string)[Tos.token_string_index++];
       ExitLoop;
      Else
       If token_stack.first Is token_stack.last
        Then
         token_break_char = '\n';
         return;
        Else
         Pop token_stack InTo tos EndPop;
         Push tos OnTo token_stack_free_list EndPush;
         ContinueLoop;
        EndIf;
      EndIf;
    Else /* Input is from a file */
     c = fgetc(Tos.source_file);
     If c Is EOF
      Then
       If Tos.source_file IsNot stdin
        Then
         fclose(Tos.source_file);
        EndIf;
       Pop token_stack InTo tos EndPop;
       token_break_char = Tos.break_char;
       Push tos OnTo token_stack_free_list EndPush;
       ExitLoop;
      Else
       token_break_char = Byte(c);
       ExitLoop;
      EndIf;
    EndIf;
  EndLoop;
 If token_break_char Is '\r'
  Then
   token_break_char = '\n';
  EndIf;
 If token_break_char Is '\t'
  Then
   token_break_char = ' ';
  EndIf;
 return;
EndCode
#undef Tos
#undef Tos_string
