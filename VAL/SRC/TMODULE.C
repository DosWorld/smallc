/*                                 TMODULE.C                               */

/*
Object modules are parsed via recursive descent as defined below:

     obj_t_module::      obj_THEADR obj_seg_grp {obj_component} obj_modtail

     obj_seg_grp::       {obj_LNAMES | obj_SEGDEF | obj_EXTDEF}
                         {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF}

     obj_component::     obj_data | obj_debug_record

     obj_data::          obj_content_def | obj_thread_def | obj_COMDEF |
                         obj_TYPDEF | obj_PUBDEF | obj_EXTDEF |
                         obj_FORREF | obj_MODPUB | obj_MODEXT


     obj_debug_record::  obj_LINNUM

     obj_content_def::   obj_data_record {obj_FIXUPP}

     obj_thread_def::    obj_FIXUPP  (containing only thread fields)

     obj_data_record::   obj_LIDATA | obj_LEDATA

     obj_modtail::       obj_MODEND
*/

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMDEF()
BeginDeclarations
bit_32                                 element_count;
bit_32                                 element_size;
bit_8                                  element_type;
bit_8                                  expected_type;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot COMDEF_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals.val
    Then
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   element_type  = *obj_ptr.b8++;
   Using element_type
    BeginCase
     When 0x61:
      expected_type = far_communal;
      element_count = obj_leaf_descriptor();
      element_size  = obj_leaf_descriptor();
      break;
     When 0x62:
      expected_type = near_communal;
      element_size  = obj_leaf_descriptor();
      element_count = 1L;
      break;
     Otherwise:
      linker_error(12, "Translator error:\n"
                       "\tModule:  \"%Fs\"\n"
                       "\t  File:  \"%Fs\"\n"
                       "\tOffset:  %lu\n"
                       "\t Error:  Communal type of \"%02X\" is illegal.\n",
                       (*tmodule_name).symbol,
                       (*infile.file_info).filename,
                       current_record_offset,
                       element_type);
    EndCase;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry             = expected_type;
     Pub.Communal.element_size  = element_size;
     Pub.Communal.element_count = element_count;
     Using element_type
     BeginCase
      When 0x61:
       Pub.Communal.next_communal = far_communals;
       far_communals              = pub;
       break;
      When 0x62:
       Pub.Communal.next_communal = near_communals;
       near_communals             = pub;
       break;
     EndCase;
    Else
     If Pub.type_entry Is expected_type
      Then
       If (element_size              * element_count)              Exceeds 
          (Pub.Communal.element_size * Pub.Communal.element_count)
        Then /* We need the largest common */
         Pub.Communal.element_size  = element_size;
         Pub.Communal.element_count = element_count;
        EndIf;
      Else
       If (Pub.type_entry Is near_communal) OrIf
          (Pub.type_entry Is far_communal)
        Then
         linker_error(4, "Translator error:\n"
                         "\tModule:  \"%Fs\"\n"
                         "\t  File:  \"%Fs\"\n"
                         "\tOffset:  %lu\n"
                         "\t Error:  Communal \"%Fs\" is declared both near "
                                     "and far.\n",
                         (*tmodule_name).symbol,
                         (*infile.file_info).filename,
                         current_record_offset,
                         Pub.symbol);
        EndIf;
      EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_COMENT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_COMENT()
BeginDeclarations
bit_8                                  comment_class;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot COMENT_record
  Then
   return(False);
  EndIf;
 obj_ptr.b8++;
 comment_class = *obj_ptr.b8++;
 Using comment_class
  BeginCase
   When 158:
    DOSSEG.val = True;
    break;
   When 161:
    codeview_information_present = True;
    break;
   Otherwise:
    break;
  EndCase;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_component                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_component:: obj_data | obj_debug_record */
bit_16 obj_component()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_data() OrIf obj_debug_record()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_content_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_content_def:: obj_data_record {obj_FIXUPP} */
bit_16 obj_content_def()
BeginDeclarations
EndDeclarations
BeginCode
 If Not obj_data_record()
  Then
   return(False);
  EndIf;
 While obj_FIXUPP()
  BeginWhile
  EndWhile;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_data                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_data:: obj_content_def |
              obj_thread_def  |
              obj_TYPDEF      |
              obj_PUBDEF      |
              obj_EXTDEF */

bit_16 obj_data()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_content_def() OrIf
    obj_thread_def()  OrIf
    obj_TYPDEF()      OrIf
    obj_PUBDEF()      OrIf
    obj_EXTDEF()      OrIf
    obj_FORREF()      OrIf
    obj_COMDEF()      OrIf
    obj_MODEXT()      OrIf
    obj_MODPUB()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_data_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/*  obj_data_record:: obj_LIDATA | obj_LEDATA */

bit_16 obj_data_record()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_LIDATA() OrIf obj_LEDATA()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_debug_record                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_debug_record:: obj_LINNUM */

bit_16 obj_debug_record()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_LINNUM()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_EXTDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_EXTDEF()
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot EXTDEF_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals.val
    Then
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    Else
     If (Pub.type_entry Is public_in_library) AndIf
        (Not Pub.Library.requested)
      Then
       library_request_count++;
       (*Pub.Library.lib_file).request_count++;
       Pub.Library.requested = True;
      EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FIXUPP                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FIXUPP()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot FIXUPP_record
  Then
   return(False);
  EndIf;
 FIXUPP_contains_only_threads = True;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If (*obj_ptr.TRD_DAT).type_fixupp_record IsZero
    Then
     obj_FIXUPP_thread();
    Else
     FIXUPP_contains_only_threads = False;
     obj_FIXUPP_fixup();
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_fixup                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_fixup()
BeginDeclarations
FIX_DAT_type                           FIX_DAT;
bit_16                                 frame_method;
LOCAT_type                             LOCAT;
bit_16                                 target_method;
bit_8                                  temp;
bit_16                                 thread_number;
EndDeclarations
BeginCode
/*+-------------------------------------------------------------------------+
  |                                                                         |
  | The LOCAT field in a FIXUPP record has its low and high bytes swapped   |
  | because the high order bit must be 0 for threads and 1 for fixups.      |
  | Since that bit could not be placed in the offset, the bytes were        |
  | swapped instead.                                                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 temp                  = obj_ptr.b8[0];
 obj_ptr.b8[0]         = obj_ptr.b8[1];
 obj_ptr.b8[1]         = temp;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          Pick up the two required fields (LOCAT and FIX_DAT)            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 LOCAT                 = *obj_ptr.LOCAT++;
 FIX_DAT               = *obj_ptr.FIX_DAT++;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |          A fixup consists of a location, mode, frame and target.        |
  |                         Process the location part.                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup_index           = LOCAT.data_record_offset;
 fixup.location_type   = LOCAT.loc;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the mode part.                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 fixup.mode            = LOCAT.m;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If FIX_DAT.f IsZero
  Then  /* Frame is specified explicitly */
   frame_method         = FIX_DAT.frame;
   fixup.frame_method   = frame_method;
   Using frame_method
    BeginCase
     When 0:
      fixup.frame_referent = (void far *) snames[obj_index_segment()];
      break;
     When 1:
      fixup.frame_referent = (void far *) gnames[obj_index_group()];
      break;
     When 2:
      fixup.frame_referent = (void far *) externals[obj_index_external()];
      break;
     When 3:
      fixup.frame_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      fixup.frame_referent = Null;
      break;
    EndCase;
  Else  /* Frame is specified by a thread */
   thread_number        = FIX_DAT.frame;
   If Not frame_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to frame thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   fixup.frame_referent = frame_thread[thread_number].referent;
   fixup.frame_method   = frame_thread[thread_number].method;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If FIX_DAT.t IsZero
  Then  /* Target is specified explicitly */
   target_method       = FIX_DAT.targt;
   fixup.target_method = target_method;
   Using target_method
    BeginCase
     When 0:  /* Target is the segment referenced by the index */
      fixup.target_referent = (void far *) snames[obj_index_segment()];
      break;
     When 1:  /* Target is the lowest seg in the group referenced 
                 by the index */
      fixup.target_referent = (void far *) gnames[obj_index_group()];
      break;
     When 2:
      fixup.target_referent = (void far *) externals[obj_index_external()];
      break;
     When 3:
      fixup.target_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    EndCase;
  Else  /* Target is specified by a thread */
   thread_number         = FIX_DAT.targt;
   If Not target_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   fixup.target_referent = target_thread[thread_number].referent;
   fixup.target_method   = target_thread[thread_number].method;
  EndIf;

 If FIX_DAT.p IsZero
  Then  /* There is a target displacement */
   fixup.target_offset = *obj_ptr.b16++;
  Else  /* The target displacement is zero */
   fixup.target_offset = 0;
  EndIf;

 fixup.external_error_detected = False;

 If (fixup.mode IsZero) AndIf
                             ((fixup.location_type Is base_location)    OrIf
                              (fixup.location_type Is pointer_location) OrIf
                              (fixup.location_type Is hibyte_location))
  Then /* Undefined fixup action */
   linker_error(4, "Possible translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Base, pointer or hibyte self-relative fixups\n"
                   "\t         are undefined.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If last_LxDATA_record_type Is LEDATA_record
  Then
   If ((fixup.location_type Is base_location)     OrIf
       (fixup.location_type Is pointer_location)) AndIf
      (exefile IsTrue)
    Then /* Base and pointer locations will require a relocation item
            in the EXE header */
     n_relocation_items++;
    EndIf;
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset + fixup_index,
                   BytePtr(Addr(fixup)),
                   sizeof(fixup));
  Else
   If fixup.mode IsZero
    Then
     linker_error(4, "Translator warning:\n"
                     "\tModule:  \"%Fs\"\n"
                     "\t  File:  \"%Fs\"\n"
                     "\tOffset:  %lu\n"
                     "\t Error:  Self-relative fixup not permitted for "
                                 "LIDATA.\n",
                     (*tmodule_name).symbol,
                     (*infile.file_info).filename,
                     current_record_offset);
    Else
     obj_fixup_LIDATA();
    EndIf;
  EndIf;

 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_fixup_LIDATA                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA()
BeginDeclarations
obj_ptr_type                           old_obj_ptr;
EndDeclarations
BeginCode
 LIDATA_index  = 0;
 LIDATA_offset = last_LxDATA_offset;
 old_obj_ptr   = obj_ptr;
 obj_ptr.b8    = Last_LIDATA_record_header.variant_part;
 end_of_last_LIDATA_record.b8 =
  (byte *)
   Addr(Last_LIDATA_record_header.variant_part
    [Last_LIDATA_record_header.rec_len-1]);
 obj_index_segment();
 obj_ptr.b16++;
 While obj_ptr.b8 IsNot end_of_last_LIDATA_record.b8
  BeginWhile
   obj_fixup_LIDATA_IDB();
  EndWhile;
 obj_ptr = old_obj_ptr;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_fixup_LIDATA_IDB                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_fixup_LIDATA_IDB()
BeginDeclarations
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 old_index;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 repeat_count = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 block_count  = *obj_ptr.b16++;  LIDATA_index += sizeof(bit_16);
 content      = obj_ptr.b8;
 old_index    = LIDATA_index;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8 = content;
     LIDATA_index = old_index;
     For j=0; j<block_count; j++
      BeginFor
       obj_fixup_LIDATA_IDB();
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8   = content;
     LIDATA_index = old_index;
     len          = Bit_16(*obj_ptr.b8++);  LIDATA_index += sizeof(bit_8);
     If (fixup_index NotLessThan LIDATA_index)        AndIf
        (fixup_index LessThan   (LIDATA_index + len))
      Then
       write_temp_file(Current_record_header.rec_typ,
                       last_LxDATA_lseg,
                       LIDATA_offset + fixup_index - LIDATA_index,
                       BytePtr(Addr(fixup)),
                       sizeof(fixup));
       If ((fixup.location_type Is base_location)     OrIf
           (fixup.location_type Is pointer_location)) AndIf
          (exefile IsTrue)
        Then /* Base and pointer locations will require a relocation item
                in the EXE header */
         n_relocation_items++;
        EndIf;
      EndIf;
     LIDATA_offset += len;
    EndFor;
   obj_ptr.b8    += len;
   LIDATA_index  += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_FIXUPP_thread                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_FIXUPP_thread()
BeginDeclarations
bit_16                                 method;
bit_16                                 thread;
TRD_DAT_type                           TRD_DAT;
EndDeclarations
BeginCode
 TRD_DAT = *obj_ptr.TRD_DAT++;
 thread  = TRD_DAT.thred;
 method  = TRD_DAT.method;
 If TRD_DAT.d IsZero
  Then  /* This is a target thread */
   target_thread[thread].method = Bit_8(method);
   target_thread[thread].thread_defined = True;
   Using method
    BeginCase
     When 0:
      target_thread[thread].referent =
       (void far *) snames[obj_index_segment()];
      break;
     When 1:
      target_thread[thread].referent =
       (void far *) gnames[obj_index_group()];
      break;
     When 2:
      target_thread[thread].referent =
       (void far *) externals[obj_index_external()];
      break;
     When 3:
      target_thread[thread].referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      target_thread[thread].referent = Null;
      break;
    EndCase;
  Else  /* This is a frame thread */
   frame_thread[thread].method = Bit_8(method);
   frame_thread[thread].thread_defined = True;
   Using method
    BeginCase
     When 0:
      frame_thread[thread].referent =
       (void far *) snames[obj_index_segment()];
      break;
     When 1:
      frame_thread[thread].referent =
       (void far *) gnames[obj_index_group()];
      break;
     When 2:
      frame_thread[thread].referent =
       (void far *) externals[obj_index_external()];
      break;
     When 3:
      frame_thread[thread].referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      frame_thread[thread].referent = Null;
      break;
    EndCase;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_FORREF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_FORREF()
BeginDeclarations
bit_16                                 len;
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot FORREF_record
  Then
   return(False);
  EndIf;
 segment_index = obj_index_segment();
 len           = Current_record_header.rec_len - 2;
 If segment_index Exceeds 127
  Then
   len--;
  EndIf;
 write_temp_file(Current_record_header.rec_typ,
                 snames[segment_index],
                 0,
                 BytePtr(obj_ptr.b8),
                 len);
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_generate_segment                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lseg_ptr obj_generate_segment(lname_entry_ptr segment_lname,
                              lname_entry_ptr class_lname,
                              combine_type    combine,
                              bit_8           align,
                              lname_entry_ptr tmodule,
                              file_info_ptr   file,
                              bit_32          address,
                              bit_32          length)
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 If combine Is stack_combine
  Then
   length += AlignmentGap(length, 1L); /* Stacks should be an integral
                                          number of words. */
  EndIf;
 seg             = lookup_segment(segment_lname, class_lname, combine);
 If (combine IsNot common_combine) OrIf (Seg.lsegs.first IsNull)
  Then
   Seg.address   = address;
   Seg.length   += length;
   lseg          = (lseg_ptr) 
                    allocate_memory(Addr(static_pool),
                                    Bit_32(sizeof(lseg_type)));
   Lseg.segment  = seg;
   Lseg.tmodule  = tmodule;
   Lseg.file     = file;
   Lseg.address  = address;
   Lseg.length   = length;
   Lseg.align    = align;
   If (combine IsNot common_combine)      AndIf
      (combine IsNot blank_common_combine)
    Then  /* Don't allocate common data yet.  (We will wait until we
             know how long the common block will be.) */
     Lseg.data   = allocate_memory(Addr(static_pool), length);
    EndIf;

   Lseg.highest_uninitialized_byte = 0L;

   Insert lseg AtEnd InList Seg.lsegs EndInsert;
  Else  /* Not the first occurrence of this common */
   lseg = Seg.lsegs.first;
   If length Exceeds Seg.length
    Then  /* Expand common block to be big enough to hold this entry. */
     Seg.length  =
     Lseg.length = length;
    EndIf;
   If align Exceeds Lseg.align
    Then  /* Align to largest boundary. */
     Lseg.align = align;
    EndIf;
  EndIf;
 If Seg.combine Is stack_combine
  Then
   If Not stack_segment_found
    Then
     largest_stack_seg        = seg;
     largest_stack_seg_length = Seg.length;
     stack_segment_found      = True;
    Else
     If Seg.length Exceeds largest_stack_seg_length
      Then
       largest_stack_seg        = seg;
       largest_stack_seg_length = Seg.length;
      EndIf;
    EndIf;
  EndIf;
 return(lseg);
EndCode
#undef Lseg 
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_GRPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_GRPDEF()
BeginDeclarations
group_entry_ptr                        group;
#define Group                          (*group)
bit_16                                 group_index;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot GRPDEF_record
  Then
   return(False);
  EndIf;
 group_index         = obj_index_LNAME();
 group               = lookup_group(lnames[group_index]);
 If n_groups NotLessThan max_groups.val
  Then
   linker_error(12, "Internal limit exceeded:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Too many GRPDEFs.  Max of %u exceeded.\n"
                    "\t         Retry with larger \"/maxgroups:n\" switch.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    max_groups.val);
  EndIf;
 gnames[++n_groups]  = group;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If *obj_ptr.b8++ IsNot 0xFF
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  GRPDEF record has a group component "
                                  "descriptor which\n"
                      "\t         does not start with 0xFF.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   segment_index = obj_index_segment();
   lseg          = snames[segment_index];
   seg           = Lseg.segment;
   If Seg.owning_group IsNull
    Then
     Seg.owning_group = group;
    Else
     If Seg.owning_group IsNot group
      Then
       linker_error(4, "Attempt to place segment \"%Fs\" into group \"%Fs\"\n"
                       "\twhen it is already in group \"%Fs\".\n"
                       "\tRequest to place in group \"%Fs\" ignored.\n",
                       (*Seg.segment_name).symbol, (*Group.group_name).symbol,
                       (*(*Seg.owning_group).group_name).symbol,
                       (*Group.group_name).symbol);
     EndIf;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Seg
#undef Group
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_external                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_external()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_externals
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid external index (%u) with only %u "
                                  "externals defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_externals);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_group                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_group()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_groups
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid group index (%u) with only %u "
                                  "groups defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_groups);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_index_LNAME                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_LNAME()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_lnames
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid LNAME index (%u) with only %u "
                                  "LNAMEs defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_lnames);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_index_segment                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_index_segment()
BeginDeclarations
bit_16                                 index;
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   index = Bit_16(*obj_ptr.b8++);
  Else
   index = (Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
           Bit_16(*obj_ptr.b8++);
  EndIf;
   If index Exceeds n_segments
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Invalid segment index (%u) with only %u "
                                  "segments defined.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      index, n_segments);
    EndIf;
 return(index);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         obj_iterated_data_block                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_iterated_data_block()
BeginDeclarations
bit_16                                 block_count;
bit_8                                 *content;
bit_16                                 i;
bit_16                                 j;
bit_16                                 len;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   content = obj_ptr.b8;
   For i=0; i<repeat_count; i++
    BeginFor
     obj_ptr.b8 = content;
     For j=0; j<block_count; j++
      BeginFor
       obj_iterated_data_block();
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   For i=0; i<repeat_count; i++
    BeginFor
     far_move(Addr(last_LxDATA_Lseg.data[LIDATA_offset]), 
              obj_ptr.b8, len);
     LIDATA_offset += len;
    EndFor;
   obj_ptr.b8 += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                     obj_iterated_data_block_length                      |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_iterated_data_block_length()
BeginDeclarations
bit_16                                 block_count;
bit_16                                 i;
bit_16                                 len;
bit_32                                 length;
bit_16                                 repeat_count;
EndDeclarations
BeginCode
 repeat_count = *obj_ptr.b16++;
 block_count  = *obj_ptr.b16++;
 If repeat_count IsZero
  Then /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Repeat count in LIDATA iterated data block "
                                "is zero.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 length       = 0L;
 If block_count IsNotZero
  Then  /* Handle recursive case:  Content is iterated data block */
   For i=0; i<block_count; i++
    BeginFor
     length     += Bit_32(repeat_count) * obj_iterated_data_block_length();
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len         = Bit_16(*obj_ptr.b8++);
   obj_ptr.b8 += len;
   length      = Bit_32(repeat_count) * Bit_32(len);
  EndIf;
 return(length);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_leaf_descriptor                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_leaf_descriptor()
BeginDeclarations
bit_8                                  element_size;
EndDeclarations
BeginCode
 element_size = *obj_ptr.b8++;
 If element_size LessThan 129
  Then
   return(Bit_32(element_size));
  Else
   If element_size Is 129
    Then
     return(Bit_32(*obj_ptr.b16++));
    Else
     If element_size Is 132
      Then
       obj_ptr.b8--;
       return((*obj_ptr.b32++) And 0x00FFFFFFL);
      Else
       If element_size Is 136
        Then
         return(*obj_ptr.b32++);
        Else
         linker_error(12, "Translator error:\n"
                        "\tModule:  \"%Fs\"\n"
                        "\t  File:  \"%Fs\"\n"
                        "\tOffset:  %lu\n"
                        "\t Error:  Communal element size of %u is illegal.\n",
                        (*tmodule_name).symbol,
                        (*infile.file_info).filename,
                        current_record_offset,
                        element_size);
        EndIf;
      EndIf;
    EndIf;
  EndIf;
 return(0L);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LEDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LEDATA()
BeginDeclarations
bit_32                                 next_byte;
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 offset;
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot LEDATA_record
  Then
   return(False);
  EndIf;
 last_LxDATA_record_type = Current_record_header.rec_typ;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        =
 lseg                    = snames[segment_index];
 len                     = Current_record_header.rec_len - 4;
 If segment_index Exceeds 127
  Then
   len--;
  EndIf;
 last_LxDATA_offset =
 offset             = *obj_ptr.b16++;
 next_byte          = Bit_32(offset) + Bit_32(len);
 If next_byte Exceeds Lseg.length
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If next_byte Exceeds Lseg.highest_uninitialized_byte
  Then
   Lseg.highest_uninitialized_byte = next_byte;
  EndIf;
 If (*last_LxDATA_Lseg.segment).combine IsNot common_combine
  Then
   far_move(Addr(Lseg.data[offset]), obj_ptr.b8, len);
  Else  /* We must save the initialization data out to the tmp file until
           later when we know the length. */
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset,
                   BytePtr(obj_ptr.b8),
                   len);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LIDATA                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LIDATA()
BeginDeclarations
bit_16                                 len;
bit_32                                 LIDATA_length;
bit_32                                 next_byte;
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot LIDATA_record
  Then
   return(False);
  EndIf;
 far_move(BytePtr(last_LIDATA_record), 
          BytePtr(object_file_element),
          Current_record_header.rec_len + sizeof(obj_record_header_type) - 1);
 last_LxDATA_record_type = Current_record_header.rec_typ;
 segment_index           = obj_index_segment();
 last_LxDATA_lseg        = snames[segment_index];
 LIDATA_offset           =
 last_LxDATA_offset      = *obj_ptr.b16++;
 LIDATA_length           = obj_LIDATA_length();
 next_byte               = last_LxDATA_offset + LIDATA_length;
 If next_byte Exceeds last_LxDATA_Lseg.length
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Attempt to initialize past end of LSEG.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If next_byte Exceeds last_LxDATA_Lseg.highest_uninitialized_byte
  Then
   last_LxDATA_Lseg.highest_uninitialized_byte = next_byte;
  EndIf;
 If (*last_LxDATA_Lseg.segment).combine IsNot common_combine
  Then
   While obj_ptr.b8 IsNot end_of_record.b8
   BeginWhile
   obj_iterated_data_block();
   EndWhile;
  Else  /* We must save the initialization data out to the tmp file until
           later when we know the length. */
   len                     = Current_record_header.rec_len - 4;
   If segment_index Exceeds 127
    Then
     len--;
    EndIf;
   write_temp_file(Current_record_header.rec_typ,
                   last_LxDATA_lseg,
                   last_LxDATA_offset,
                   BytePtr(obj_ptr.b8),
                   len);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           obj_LIDATA_length                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 obj_LIDATA_length()
BeginDeclarations
bit_32                                 length;
bit_8                                  *start;
EndDeclarations
BeginCode
 start  = obj_ptr.b8;
 length = 0L;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   length += obj_iterated_data_block_length();
  EndWhile;
 obj_ptr.b8 = start;
 return(length);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LINNUM                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LINNUM()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot LINNUM_record
  Then
   return(False);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_LNAMES                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_LNAMES()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot LNAMES_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_lnames NotLessThan max_lnames.val
    Then
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many LNAMES.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxlnames:n\" switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_lnames.val);
    EndIf;
   lnames[++n_lnames] = obj_name();
  EndWhile;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEND                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEND()
BeginDeclarations
FIX_DAT_type                           END_DAT;
bit_16                                 frame_method;
MOD_TYP_type                           MOD_TYP;
bit_16                                 target_method;
bit_16                                 thread_number;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot MODEND_record
  Then
   return(False);
  EndIf;
 MOD_TYP = *obj_ptr.MOD_TYP++;

 If MOD_TYP.zeros IsNotZero
  Then
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Bits 1 thru 5 of MOD TYP must be zero.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If (MOD_TYP.mattr IsNot 1) AndIf (MOD_TYP.mattr IsNot 3)
  Then  /* We have no starting address */
   return(True);
  EndIf;

 If MOD_TYP.l IsNot 1
  Then
   linker_error(4, "Translator error:\n"
                   "\tModule:  \"%Fs\"\n"
                   "\t  File:  \"%Fs\"\n"
                   "\tOffset:  %lu\n"
                   "\t Error:  Bit 0 of MOD TYP must be one.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename,
                   current_record_offset);
  EndIf;

 If start_address_found IsTrue
  Then
   linker_error(4, "Multiple start address encountered.  The start address\n"
                   "in module \"%Fs\" of file \"%Fs\" has been ignored.\n",
                   (*tmodule_name).symbol,
                   (*infile.file_info).filename);
  EndIf;

 start_address_found = True;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                  Pick up the required field END_DAT.                    |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 END_DAT = *obj_ptr.FIX_DAT++;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the frame part.                         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If END_DAT.f IsZero
  Then  /* Frame is specified explicitly */
   frame_method                 = END_DAT.frame;
   start_address.frame_method   = frame_method;
   Using frame_method
    BeginCase
     When 0:
      start_address.frame_referent =
       (void far *) snames[obj_index_segment()];
      break;
     When 1:
      start_address.frame_referent =
       (void far *) gnames[obj_index_group()];
      break;
     When 2:
      start_address.frame_referent =
       (void far *) externals[obj_index_external()];
      break;
     When 3:
      start_address.frame_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
     Otherwise:
      start_address.frame_referent = Null;
      break;
    EndCase;
  Else  /* Frame is specified by a thread */
   thread_number                = END_DAT.frame;
   If Not frame_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to frame thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   start_address.frame_referent = frame_thread[thread_number].referent;
   start_address.frame_method   = frame_thread[thread_number].method;
  EndIf;

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         Process the target part.                        |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

 If END_DAT.t IsZero
  Then  /* Target is specified explicitly */
   target_method               = END_DAT.targt;
   start_address.target_method = target_method;
   Using target_method
    BeginCase
     When 0:
      start_address.target_referent =
       (void far *) snames[obj_index_segment()];
      break;
     When 1:
      start_address.target_referent =
       (void far *) gnames[obj_index_group()];
      break;
     When 2:
      start_address.target_referent =
       (void far *) externals[obj_index_external()];
      break;
     When 3:
      start_address.target_referent =
       (void far *) (Bit_32(*obj_ptr.b16++) ShiftedLeft 4);
      break;
    EndCase;
  Else  /* Target is specified by a thread */
   thread_number                 = END_DAT.targt;
   If Not target_thread[thread_number].thread_defined
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Reference to target thread %u which has "
                                  "been defined.n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      thread_number);
    EndIf;
   start_address.target_referent = target_thread[thread_number].referent;
   start_address.target_method   = target_thread[thread_number].method;
  EndIf;

 If END_DAT.p IsZero
  Then  /* There is a target displacement */
   start_address.target_offset = *obj_ptr.b16++;
  Else  /* The target displacement is zero */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Only primary fixups allowed in MODEND.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
   start_address.target_offset = 0;
  EndIf;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODEXT                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODEXT()
BeginDeclarations
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot MODEXT_record
  Then
   return(False);
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   If n_externals NotLessThan max_externals.val
    Then
     linker_error(12, "Internal limit exceeded:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Too many externals.  Max of %u exceeded.\n"
                      "\t         Retry with larger \"/maxexternals:n\" "
                                  "switch.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      max_externals.val);
    EndIf;
   len         = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub         = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   obj_name_length();  /* Eat the type index. */
   externals[++n_externals] = pub;
   If Pub.type_entry Is unused
    Then
     Insert pub AtEnd InList external_list EndInsert;
     Pub.type_entry = external;
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_MODPUB                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_MODPUB()
BeginDeclarations
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot MODPUB_record
  Then
   return(False);
  EndIf;
 group_index = obj_index_group();
 segment_index = obj_index_segment();
 If (segment_index IsZero) AndIf (group_index IsZero)
  Then
   frame = *obj_ptr.b16++;
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   len = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub = lookup_public(len, obj_ptr.b8, tmodule_number);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal
    Then
     linker_error(4, "Duplicate definition of public \"%Fs\".\n"
                     "\tDefinition in module \"%Fs\" of file \"%Fs\" "
                     "ignored.\n",
                     Pub.symbol,
                     (*tmodule_name).symbol,(*infile.file_info).filename);
     obj_ptr.b16++;      /* Eat offset. */
     obj_name_length();  /* Eat type index. */
    Else
     If Pub.type_entry Is unused
      Then
       Insert pub AtEnd InList external_list EndInsert;
      EndIf;
     Pub.type_entry       = internal;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     Pub.Internal.offset  = *obj_ptr.b16++;
     obj_name_length();  /* Eat type index. */
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_modtail                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_modtail:: obj_MODEND */

bit_16 obj_modtail()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_MODEND()
  Then
   return(True);
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_name                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
lname_entry_ptr obj_name()
BeginDeclarations
lname_entry_ptr                        name;
bit_16                                 len;
EndDeclarations
BeginCode
 len = obj_name_length();
 If len IsZero
  Then
   name = none_lname;
  Else
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   name        = lookup_lname(len, obj_ptr.b8);
   obj_ptr.b8 += len;
  EndIf;
 return(name);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            obj_name_length                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_name_length()
BeginDeclarations
EndDeclarations
BeginCode
 If *obj_ptr.b8 LessThan 128
  Then
   return(Bit_16(*obj_ptr.b8++));
  Else
   return((Bit_16(*obj_ptr.b8++ - 128) ShiftedLeft 8) +
          (Bit_16(*obj_ptr.b8++)));
  EndIf;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_next_record                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void obj_next_record()
BeginDeclarations
EndDeclarations
BeginCode
 Repeat
  BeginRepeat
   file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
   While (Current_record_header.rec_typ Is LINNUM_record) OrIf
         ((Current_record_header.rec_typ Is COMENT_record) AndIf
          (Current_record_header.rec_len Exceeds MAX_OBJECT_FILE_READ_SIZE))
    BeginWhile
     file_position(Bit_32(infile.byte_position) +
                   infile.start_of_buffer_position +
                   Bit_32(Current_record_header.rec_len));
     file_read(object_file_element,  sizeof(obj_record_header_type) - 1);
    EndWhile;
   current_record_offset = Bit_32(infile.byte_position) +
                           infile.start_of_buffer_position -
                           Bit_32(sizeof(obj_record_header_type)-1);
   If Current_record_header.rec_len Exceeds MAX_OBJECT_FILE_READ_SIZE
    Then
     linker_error(12, "Probable invalid OBJ format "
                      "or possible translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Record too long.\n"
                      "\t         Max record length supported by this "
                                 "linker is %u bytes.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset,
                      MAX_OBJECT_FILE_READ_SIZE);
    EndIf;
   file_read(Current_record_header.variant_part, 
             Current_record_header.rec_len);
   If (objchecksum.val IsTrue) AndIf
      (Bit_8(checksum(Current_record_header.rec_len +
                      sizeof(obj_record_header_type)-1,
                     (byte *) current_record_header)) IsNotZero)
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  Checksum error.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   obj_ptr.b8 = Current_record_header.variant_part;
   end_of_record.b8 =
    (byte *)
     Addr(Current_record_header.variant_part[Current_record_header.rec_len-1]);
   RepeatIf obj_COMENT()
  EndRepeat;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_PUBDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_PUBDEF()
BeginDeclarations
bit_16                                 group_index;
bit_16                                 frame;
bit_16                                 len;
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_16                                 segment_index;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot PUBDEF_record
  Then
   return(False);
  EndIf;
 group_index = obj_index_group();
 segment_index = obj_index_segment();
 If (segment_index IsZero) AndIf (group_index IsZero)
  Then
   frame = *obj_ptr.b16++;
  EndIf;
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   len = obj_name_length();
   If case_ignore.val
    Then
     far_to_lower(BytePtr(obj_ptr.b8), len);
    EndIf;
   pub = lookup_public(len, obj_ptr.b8, 0);
   obj_ptr.b8 += len;
   If Pub.type_entry Is internal
    Then
     linker_error(4, "Duplicate definition of public \"%Fs\".\n"
                     "\tDefinition in module \"%Fs\" of file \"%Fs\" "
                     "ignored.\n",
                     Pub.symbol,
                     (*tmodule_name).symbol,(*infile.file_info).filename);
     obj_ptr.b16++;      /* Eat offset. */
     obj_name_length();  /* Eat type index. */
    Else
     If Pub.type_entry Is unused
      Then
       Insert pub AtEnd InList external_list EndInsert;
      EndIf;
     If (Pub.type_entry Is public_in_library) AndIf
        (Pub.Library.requested)
      Then
       library_request_count--;
       (*Pub.Library.lib_file).request_count--;
      EndIf;
     Pub.type_entry       = internal;
     Pub.Internal.group   = gnames[group_index];
     Pub.Internal.lseg    = snames[segment_index];
     Pub.Internal.frame   = frame;
     Pub.Internal.offset  = *obj_ptr.b16++;
     obj_name_length();  /* Eat type index. */
    EndIf;
  EndWhile;
 obj_next_record();
 return(True);
EndCode
#undef Pub

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_SEGDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_SEGDEF()
BeginDeclarations
acbp_type                              acbp;
bit_32                                 address;
bit_8                                  align;
lname_entry_ptr                        class_lname;
bit_8                                  combine;
bit_32                                 length;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 segment_index;
lname_entry_ptr                        segment_lname;
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot SEGDEF_record
  Then
   return(False);
  EndIf;
 acbp    = *obj_ptr.acbp++;
 align   = Bit_8(acbp.a);
 If align Is absolute_segment
  Then
   address  = (Bit_32(*obj_ptr.b16++) ShiftedLeft 4L);  /* Frame */
   address += Bit_32(*obj_ptr.b8++);                    /* Offset */
  Else
   address = 0L;
  EndIf;
 If align Exceeds dword_aligned
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Align type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    align);
  EndIf;
 combine = Bit_8(acbp.c);
 If (combine Is 4) OrIf (combine Is 7)
  Then /* Treat combine types 4 and 7 the same as 2. */
   combine = public_combine;
  EndIf;
 If (combine Is 1) OrIf (combine Is 3)
  Then /* This is a translator error. */
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Combine type of %u is undefined.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    combine);
  EndIf;
 length = Bit_32(*obj_ptr.b16++);
 If acbp.b IsNotZero
  Then
   If length IsNotZero
    Then
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  SEGDEF has acbp.b of 1 and length not "
                                  "zero.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
   length = 65536L;
  EndIf;
 segment_index         = obj_index_LNAME();
 segment_lname         = lnames[segment_index];
 class_lname           = lnames[obj_index_LNAME()];
 lseg = obj_generate_segment(segment_lname,
                             class_lname,
                             combine,
                             align,
                             tmodule_name,
                             infile.file_info,
                             address,
                             length);
 If n_segments NotLessThan max_segments.val
  Then
   linker_error(12, "Internal limit exceeded:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Too many SEGDEFs.  Max of %u exceeded.\n"
                    "\t         Retry with larger \"/maxsegments:n\" "
                                "switch.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset,
                    max_segments.val);
  EndIf;
 snames[++n_segments]  = lseg;

 obj_next_record();
 return(True);
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_seg_grp                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_seg_grp:: {obj_LNAMES | obj_SEGDEF | obj_EXTDEF}
                 {obj_TYPDEF | obj_EXTDEF | obj_GRPDEF} */
bit_16 obj_seg_grp()
BeginDeclarations
EndDeclarations
BeginCode
 While obj_LNAMES() OrIf obj_SEGDEF() OrIf obj_EXTDEF()
  BeginWhile
  EndWhile;
 While obj_TYPDEF() OrIf obj_EXTDEF() OrIf obj_GRPDEF()
  BeginWhile
  EndWhile;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_THEADR                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_THEADR()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot THEADR_record
  Then
   return(False);
  EndIf;
 tmodule_name = obj_name();
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                             obj_thread_def                              |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_thread_def:: obj_FIXUPP  (containing only thread fields) */

bit_16 obj_thread_def()
BeginDeclarations
EndDeclarations
BeginCode
 If obj_FIXUPP()
  Then
   If FIXUPP_contains_only_threads
    Then
     return(True);
    Else
     linker_error(12, "Translator error:\n"
                      "\tModule:  \"%Fs\"\n"
                      "\t  File:  \"%Fs\"\n"
                      "\tOffset:  %lu\n"
                      "\t Error:  \"THREAD DEF\" FIXUPP encountered which "
                                  "did not contain\n"
                      "\t          only thread defs.\n",
                      (*tmodule_name).symbol,
                      (*infile.file_info).filename,
                      current_record_offset);
    EndIf;
  EndIf;
 return(False);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              obj_tmodule                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

/* obj_t_module:: obj_THEADR obj_seg_grp {obj_component} obj_modtail */

bit_16 obj_tmodule()
BeginDeclarations
EndDeclarations
BeginCode
 far_set(BytePtr(externals), 0,
         sizeof(public_entry_ptr)*(max_externals.val+1));
 far_set(BytePtr(gnames),    0,
         sizeof(group_entry_ptr)*(max_groups.val+1));
 far_set(BytePtr(lnames),    0,
         sizeof(lname_entry_ptr)*(max_lnames.val+1));
 far_set(BytePtr(snames),    0,
         sizeof(lseg_ptr)*(max_segments.val+1));
 far_set(BytePtr(target_thread), 0, sizeof(thread_type)*4);
 far_set(BytePtr(frame_thread),  0, sizeof(thread_type)*4);

 n_externals =
 n_groups    =
 n_lnames    =
 n_segments  = 0;
 tmodule_number++;
 tmodule_name = lookup_lname(31, (byte *) "(THEADR record not encountered)");
 obj_next_record();
 If Not obj_THEADR()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  T-MODULE record missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 If Not obj_seg_grp()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  Segment/Group definition record(s) missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 While obj_component()
  BeginWhile
  EndWhile;
 If Not obj_modtail()
  Then
   linker_error(12, "Translator error:\n"
                    "\tModule:  \"%Fs\"\n"
                    "\t  File:  \"%Fs\"\n"
                    "\tOffset:  %lu\n"
                    "\t Error:  MODEND record missing.\n",
                    (*tmodule_name).symbol,
                    (*infile.file_info).filename,
                    current_record_offset);
  EndIf;
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               obj_TYPDEF                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_16 obj_TYPDEF()
BeginDeclarations
EndDeclarations
BeginCode
 If Current_record_header.rec_typ IsNot TYPDEF_record
  Then
   return(False);
  EndIf;
 obj_next_record();
 return(True);
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              write_temp_file                            |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void write_temp_file(bit_8           rec_typ,
                     lseg_ptr        lseg,
                     bit_16          offset,
                     byte_ptr        data,
                     bit_16          len)
BeginDeclarations
EndDeclarations
BeginCode
 temp_file_header.rec_typ       = rec_typ;
 temp_file_header.rec_len       = len;
 temp_file_header.lseg          = lseg;
 temp_file_header.offset        = offset;
 file_write(BytePtr(Addr(temp_file_header)), 
            Bit_32(sizeof(temp_file_header)));
 If len Exceeds 0
  Then
   file_write(data, Bit_32(len));
  EndIf;
 return;
EndCode
