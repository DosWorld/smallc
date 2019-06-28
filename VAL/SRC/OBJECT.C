/*                                 OBJECT.C                                */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          process_object_modules                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void process_object_modules()
BeginDeclarations
file_info_ptr                          obj_file;
EndDeclarations
BeginCode
 object_module_start_time = Now;
 file_open_for_write(temp_file);
 TraverseList(obj_file_list, obj_file)
  BeginTraverse
   file_open_for_read(obj_file);
   obj_tmodule();
   file_close_for_read();
  EndTraverse;
 return;
EndCode
