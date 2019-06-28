/*                                ENDLINK.C                               */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           end_linker                                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void end_linker(bit_16 return_code)
BeginDeclarations
EndDeclarations
BeginCode
 If statistics.val IsTrue
  Then
   linker_statistics();
  EndIf;
 exit(return_code);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           linker_statistics                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void linker_statistics()
BeginDeclarations
file_info_ptr                          file;
#define File                           (*file)
EndDeclarations
BeginCode
 statistics_start_time = Now;
 statistics.val        = False;  /* Prevent recursive call. */
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Memory Usage Statistics                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If link_step Exceeds 0
  Then
   linker_message("\n"
                  "Memory Usage Statistics:\n"
                  "---Memory Pool---  --Size--  --Used--  --Free--  %%Free\n");
   edit_number_string(temp_string, "%lu", free_pool.pool_size);
   linker_message("%-17s  %8Fs\n",
                  free_pool.pool_id,
                  String(temp_string));
   edit_number_string(temp_string, "%lu", static_pool.pool_size);
   linker_message("%-17s  %8Fs  ",
                  static_pool.pool_id,
                  String(temp_string));
   edit_number_string(temp_string, "%lu", static_pool.used_bytes);
   linker_message("%8Fs  ",
                  String(temp_string));
   edit_number_string(temp_string, "%lu",
                      static_pool.pool_size - static_pool.used_bytes);
   linker_message("%8Fs  %4u%%\n",
                  String(temp_string),
                  Bit_16((100L*(static_pool.pool_size -
                                static_pool.used_bytes)) /
                         static_pool.pool_size));
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Object File Statistics                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If (link_step Exceeds 4) AndIf (obj_file_list.first IsNotNull)
  Then
   linker_message("\n"
                  "Object File Statistics:\n");
   linker_message("--Size--  -------------Object File-------------\n");
   TraverseList(obj_file_list, file)
    BeginTraverse
     edit_number_string(temp_string, "%lu", File.file_size);
     linker_message("%8Fs  %Fs\n", String(temp_string), File.filename);
    EndTraverse;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                        Library File Statistics                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If (link_step Exceeds 4) AndIf (lib_file_list.first IsNotNull)
  Then
   linker_message("\n"
                  "Library File Statistics:\n");
   linker_message("--Size--  Modules  Passes  "
                  "-------------Library File------------\n");
   TraverseList(lib_file_list, file)
    BeginTraverse
     edit_number_string(temp_string, "%lu", File.file_size);
     linker_message("%8Fs  ",
                    String(temp_string));
     edit_number_string(temp_string, "%u",  File.module_count);
     linker_message("%7Fs  ",
                    String(temp_string));
     edit_number_string(temp_string, "%u",  File.pass_count);
     linker_message("%6Fs  %Fs\n",
                    String(temp_string),
                    File.filename);
    EndTraverse;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                       Time Usage Statistics                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 total_time = 0;
 linker_message("\n"
                "Time Usage Statistics:\n");
 If link_step Exceeds 0
  Then
   total_time += user_input_start_time - linker_start_time;
   linker_message("  Primary linker initialization time:%7s\n",
                  elapsed_time(linker_start_time,
                               user_input_start_time));
  EndIf;
 If link_step Exceeds 1
  Then
   total_time += secondary_init_start_time - user_input_start_time;
   linker_message("                     User input time:%7s\n",
                  elapsed_time(user_input_start_time,
                               secondary_init_start_time));
  EndIf;
 If link_step Exceeds 2
  Then
   total_time += library_directory_start_time -
                 secondary_init_start_time;
   linker_message("Secondary linker initialization time:%7s\n",
                  elapsed_time(secondary_init_start_time,
                               library_directory_start_time));
  EndIf;
 If link_step Exceeds 3
  Then
   total_time += object_module_start_time - library_directory_start_time;
   linker_message("   Library directory processing time:%7s\n",
                  elapsed_time(library_directory_start_time,
                               object_module_start_time));
  EndIf;
 If link_step Exceeds 4
  Then
   total_time += library_processing_start_time - object_module_start_time;
   linker_message("       Object module processing time:%7s\n",
                  elapsed_time(object_module_start_time,
                               library_processing_start_time));
  EndIf;
 If link_step Exceeds 5
  Then
   total_time += order_start_time - library_processing_start_time;
   linker_message("             Library processing time:%7s\n",
                  elapsed_time(library_processing_start_time,
                               order_start_time));
  EndIf;
 If link_step Exceeds 6
  Then
   total_time += fixup_start_time - order_start_time;
   linker_message("  Segment ordering and aligning time:%7s\n",
                  elapsed_time(order_start_time,
                               fixup_start_time));
  EndIf;
 If link_step Exceeds 7
  Then
   total_time += exec_image_start_time - fixup_start_time;
   linker_message("                          Fixup time:%7s\n",
                  elapsed_time(fixup_start_time,
                               exec_image_start_time));
  EndIf;
 If link_step Exceeds 8
  Then
   total_time += map_start_time - exec_image_start_time;
   linker_message("         Executable image write time:%7s\n",
                  elapsed_time(exec_image_start_time,
                               map_start_time));
  EndIf;
 If link_step Exceeds 9
  Then
   total_time += statistics_start_time - map_start_time;
   linker_message("                      Map write time:%7s\n",
                  elapsed_time(map_start_time,
                               statistics_start_time));
  EndIf;
 linker_end_time = Now;
 total_time += linker_end_time - statistics_start_time;
 linker_message("           Statistics reporting time:%7s\n",
                elapsed_time(statistics_start_time,
                             linker_end_time));
 linker_message("                                      ------\n"
                "                 Total Elaspsed Time:%7s\n",
                elapsed_time(0L, total_time));

 return;
EndCode
#undef File
