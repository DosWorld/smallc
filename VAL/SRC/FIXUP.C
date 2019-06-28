/*                                 FIXUP.C                                 */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_FIXUPP_record                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FIXUPP_record()
BeginDeclarations
byte_ptr                               byte_location;
int_32                                 IP_distance_to_target;
bit_16                                 fbval;
bit_32                                 foval;
bit_32                                 frame_address;
bit_32                                 location_address;
bit_32                                 target_address;
bit_16 far                            *word_location;
EndDeclarations
BeginCode
 file_read(BytePtr(Addr(fixup)), sizeof(fixup));
 frame_address  = frame();
 target_address = target();
 If ((target_address LessThan frame_address) OrIf
     (target_address Exceeds (frame_address + 65535L))) AndIf
    (fixup.frame_method IsNot 6) AndIf (frame_absolute IsFalse)
  Then
   linker_error(4, "Fixup error:\n"
                   "\t Module:  \"%Fs\"\n"
                   "\t   File:  \"%Fs\"\n"
                   "\tSegment:  \"%Fs\"\n"
                   "\t Offset:  %u\n"
                   "\t  Error:  Target not within frame.\n",
                   (*(*temp_file_header.lseg).tmodule).symbol,
                   (*(*temp_file_header.lseg).file).filename,
                (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                   temp_file_header.offset);
  EndIf;
 byte_location  = 
              Addr((*temp_file_header.lseg).data[temp_file_header.offset]);
 word_location  = (bit_16 far *) byte_location;
 If fixup.mode IsZero
  Then /* Self-relative fixup */
   location_address = (*temp_file_header.lseg).address +
                      Bit_32(temp_file_header.offset) +
                      1L;
   If fixup.location_type Is offset_location
    Then
     location_address++;
    EndIf;
   If (location_address LessThan frame_address) OrIf
      (location_address Exceeds (frame_address + 65535L)) OrIf
      (frame_absolute IsTrue)
    Then
     linker_error(4, "Fixup error:\n"
                     "\t Module:  \"%Fs\"\n"
                     "\t   File:  \"%Fs\"\n"
                     "\tSegment:  \"%Fs\"\n"
                     "\t Offset:  %u\n"
                     "\t  Error:  Location not within frame.\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                 (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                     temp_file_header.offset);
    EndIf;
   Using fixup.location_type
    BeginCase
     When lobyte_location:
      location_address = (*temp_file_header.lseg).address +
                         Bit_32(temp_file_header.offset) +
                         1L;
      IP_distance_to_target = Int_32(target_address) -
                              Int_32(location_address);
      If (IP_distance_to_target < -128L) OrIf
         (IP_distance_to_target > 127L)
       Then
        linker_error(4, "Byte self-relative fixup error:\n"
                        "\t Module:  \"%Fs\"\n"
                        "\t   File:  \"%Fs\"\n"
                        "\tSegment:  \"%Fs\"\n"
                        "\t Offset:  %u\n"
                        "\t  Error:  Distance to target out of range.\n",
                        (*(*temp_file_header.lseg).tmodule).symbol,
                        (*(*temp_file_header.lseg).file).filename,
                (*(*(*temp_file_header.lseg).segment).segment_name).symbol,
                        temp_file_header.offset);
       EndIf;
      *byte_location += Bit_8(IP_distance_to_target);
      break;
     When offset_location:
      IP_distance_to_target = target_address - location_address;
      *word_location += Bit_16(IP_distance_to_target);
      break;
     When base_location:       /* Undefined action */
     When pointer_location:    /* Undefined action */
     When hibyte_location:     /* Undefined action */
      break;
    EndCase;
  Else /* Segment-relative fixup */
   fbval          = Bit_16(frame_address ShiftedRight 4);
   foval          = target_address - frame_address;
   If (frame_absolute IsFalse)                     AndIf
      (exefile IsFalse)                            AndIf
      ((fixup.location_type Is base_location)      OrIf
       (fixup.location_type Is pointer_location))
    Then  /* Count the relocation items we should not be getting. */
     n_relocation_items++;
    EndIf;
   Using fixup.location_type
    BeginCase
     When lobyte_location:
      *byte_location += Bit_8(foval);
      break;
     When offset_location:
      *word_location += Bit_16(foval);
      break;
     When base_location:
      *word_location += fbval;
      If exefile IsTrue
       Then
        Exe_header.relocation_table[Exe_header.n_relocation_items++] =
         segment_offset(temp_file_header.lseg, temp_file_header.offset);
       EndIf;
      break;
     When pointer_location:
      *word_location++ += Bit_16(foval);
      *word_location   += fbval;
      If exefile IsTrue
       Then
        Exe_header.relocation_table[Exe_header.n_relocation_items++] =
         segment_offset(temp_file_header.lseg, temp_file_header.offset+2);
       EndIf;
      break;
     When hibyte_location:
      *byte_location += Bit_8(foval ShiftedRight 8);
      break;
    EndCase;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_FORREF_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_FORREF_record()
BeginDeclarations
bit_16                                 len;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
bit_16                                 offset;
bit_8                                  size;
EndDeclarations
BeginCode
 lseg          = temp_file_header.lseg;
 len           = temp_file_header.rec_len;
 file_read(BytePtr(object_file_element), len);
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[len]);
 size             = *obj_ptr.b8++;
 Using size
  BeginCase
   When 0:
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      offset = *obj_ptr.b16++;
      Lseg.data[offset] += *obj_ptr.b8++;
     EndWhile;
    break;
   When 1:
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      offset = *obj_ptr.b16++;
      *((bit_16 far *) Addr(Lseg.data[offset])) += *obj_ptr.b16++;
     EndWhile;
    break;
   When 2:
    While obj_ptr.b8 IsNot end_of_record.b8
     BeginWhile
      offset = *obj_ptr.b16++;
      *((bit_32 far *) Addr(Lseg.data[offset])) += *obj_ptr.b32++;
     EndWhile;
    break;
   Otherwise:
    linker_error(4, "Translator error:\n"
                    "\t Module:  \"%Fs\"\n"
                    "\t   File:  \"%Fs\"\n"
                    "\tSegment:  \"%Fs\"\n"
                    "\t  Error:  Invalid FORREF record.\n",
                    (*(*temp_file_header.lseg).tmodule).symbol,
                    (*(*temp_file_header.lseg).file).filename,
            (*(*(*temp_file_header.lseg).segment).segment_name).symbol);
    break;
  EndCase;
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LEDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LEDATA_record()
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
EndDeclarations
BeginCode
 lseg          = temp_file_header.lseg;
 lseg_data_ptr = Addr(Lseg.data[temp_file_header.offset]);
 file_read(lseg_data_ptr, temp_file_header.rec_len);
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                            fixup_LIDATA_IDB                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_IDB()
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
       fixup_LIDATA_IDB();
      EndFor;
    EndFor;
  Else  /* Handle non-recursive case:  Content is data. */
   len = Bit_16(*obj_ptr.b8++);
   For i=0; i<repeat_count; i++
    BeginFor
     far_move(lseg_data_ptr, obj_ptr.b8, len);
     lseg_data_ptr += len;
    EndFor;
   obj_ptr.b8 += len;
  EndIf;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                           fixup_LIDATA_record                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void fixup_LIDATA_record()
BeginDeclarations
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
EndDeclarations
BeginCode
 lseg          = temp_file_header.lseg;
 lseg_data_ptr = Addr(Lseg.data[temp_file_header.offset]);
 file_read(BytePtr(object_file_element), temp_file_header.rec_len);
 obj_ptr.b8       = object_file_element;
 end_of_record.b8 = Addr(obj_ptr.b8[temp_file_header.rec_len]);
 While obj_ptr.b8 IsNot end_of_record.b8
  BeginWhile
   fixup_LIDATA_IDB();
  EndWhile;
 return;
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 frame                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 frame()
BeginDeclarations
bit_32                                 frame_address;
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 Using fixup.frame_method
  BeginCase
   When 0:  /* Frame is segment relative */
    lseg           = (lseg_ptr) fixup.frame_referent;
    frame_absolute = Lseg.align Is absolute_segment;
    seg            = Lseg.segment;
    frame_address  = Seg.address;
    break;
   When 1:  /* Frame is group relative */
    grp            = (group_entry_ptr) fixup.frame_referent;
    seg            = Grp.first_segment;
    lseg           = Seg.lsegs.first;
    frame_absolute = Lseg.align Is absolute_segment;
    frame_address  = Seg.address;
    break;
   When 2:  /* Frame is relative to external */
    pub = (public_entry_ptr) fixup.frame_referent;
    frame_address = public_frame_address(pub);
    break;
   When 3:  /* Frame is absolute */
    frame_absolute = True;
    frame_address  = Bit_32(fixup.frame_referent);
    break;
   When 4:  /* Frame is segment containing location */
    lseg           = temp_file_header.lseg;
    seg            = Lseg.segment;
    frame_absolute = Lseg.align Is absolute_segment;
    frame_address  = Seg.address;
    break;
   When 5:  /* Frame is defined by target */
    Using fixup.target_method
     BeginCase
      When 0:  /* Target is segment relative */
       lseg           = (lseg_ptr) fixup.target_referent;
       seg            = Lseg.segment;
       frame_absolute = Lseg.align Is absolute_segment;
       frame_address  = Seg.address;
       break;
      When 1:  /* Target is group relative */
       grp = (group_entry_ptr) fixup.target_referent;
       seg            = Grp.first_segment;
       lseg           = Seg.lsegs.first;
       frame_absolute = Lseg.align Is absolute_segment;
       frame_address  = Seg.address;
       break;
      When 2:  /* Target is relative to an external */
       pub = (public_entry_ptr) fixup.target_referent;
       frame_address = public_frame_address(pub);
       break;
      When 3:  /* Target is absolute */
       frame_absolute = True;
       frame_address  = Bit_32(fixup.target_referent);
       break;
     EndCase;
    break;
   When 6:  /* No frame */
    frame_absolute = False;
    frame_address = 0L;
    break;
  EndCase;
 return(frame_address & 0xFFFFFFF0L);
EndCode
#undef Grp
#undef Lseg
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               pass_two                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void pass_two()
BeginDeclarations
EndDeclarations
BeginCode
 fixup_start_time = Now;
/*+-------------------------------------------------------------------------+
  |                                                                         |
  |      First, we will figure out how long the EXE header will be.         |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
 If exefile IsTrue
  Then
   exe_header_size  = Bit_32(sizeof(EXE_header_type)) - 
                      Bit_32(sizeof(bit_32)) + 
                     (Bit_32(sizeof(bit_32)) * Bit_32(n_relocation_items));
   If align_exe_header.val IsTrue
    Then
     exe_header_size += AlignmentGap(exe_header_size, 0xFL);
    Else
     exe_header_size += AlignmentGap(exe_header_size, 0x1FFL);
    EndIf;
   exe_header       = (EXE_header_ptr)
                       allocate_memory(Addr(static_pool),
                                       exe_header_size);
   far_set(BytePtr(exe_header), 0, Bit_16(exe_header_size));
  EndIf;
 file_open_for_read(temp_file);
 file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
 While temp_file_header.rec_typ IsNotZero
  BeginWhile
   Using temp_file_header.rec_typ
    BeginCase
     When FIXUPP_record:
      fixup_FIXUPP_record();
      break;
     When FORREF_record:
      fixup_FORREF_record();
      break;
     When LEDATA_record:
      fixup_LEDATA_record();
      break;
     When LIDATA_record:
      fixup_LIDATA_record();
      break;
     Otherwise:
      linker_error(16, "Internal logic error:  Invalid temp file record.\n");
      break;
    EndCase;
   file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
  EndWhile;
 file_close_for_read();
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          public_frame_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_frame_address(public_entry_ptr pub)
BeginDeclarations
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 frame_absolute = False;
 If Pub.type_entry IsNot internal
  Then
   seg = (*temp_file_header.lseg).segment;
   If Not fixup.external_error_detected
    Then
     linker_error(4, "\tModule \"%Fs\" in file \"%Fs\"\n"
                     "\treferences unresolved external \"%Fs\"\n"
                     "\tat offset %04XH in segment \"%Fs\".\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                     Pub.symbol,
                     temp_file_header.offset,
                     (*Seg.segment_name).symbol);
     fixup.external_error_detected = True;
    EndIf;
   address = 0L;
  Else
   If Pub.Internal.group IsNull
    Then
     If Pub.Internal.lseg IsNull
      Then
       frame_absolute = True;
       address        = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
      Else
       frame_absolute = (*Pub.Internal.lseg).align Is absolute_segment;
       address        = (*(*Pub.Internal.lseg).segment).address;
      EndIf;
    Else
     frame_absolute = 
               (*(*(*Pub.Internal.group).first_segment).lsegs.first).align Is
               absolute_segment;
     address        = (*(*Pub.Internal.group).first_segment).address;
    EndIf;
  EndIf;
 return(address);
EndCode
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                         public_target_address                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 public_target_address(public_entry_ptr pub)
BeginDeclarations
bit_32                                 address;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
EndDeclarations
BeginCode
 If Pub.type_entry IsNot internal
  Then
   seg = (*temp_file_header.lseg).segment;
   If Not fixup.external_error_detected
    Then
     linker_error(4, "\tModule \"%Fs\" in file \"%Fs\"\n"
                     "\treferences unresolved external \"%Fs\"\n"
                     "\tat offset %04XH in segment \"%Fs\".\n",
                     (*(*temp_file_header.lseg).tmodule).symbol,
                     (*(*temp_file_header.lseg).file).filename,
                     Pub.symbol,
                     temp_file_header.offset,
                     (*Seg.segment_name).symbol);
     fixup.external_error_detected = True;
    EndIf;
   address = 0L;
  Else
   If Pub.Internal.lseg IsNull
    Then
     address = (Bit_32(Pub.Internal.frame) ShiftedLeft 4);
    Else
     address = (*Pub.Internal.lseg).address;
    EndIf;
  EndIf;
 return(address + Bit_32(Pub.Internal.offset));
EndCode
#undef Pub
#undef Seg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                              segment_offset                             |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 segment_offset(lseg_ptr lseg, bit_16 offset)
BeginDeclarations
#define Lseg                           (*lseg)
EndDeclarations
BeginCode
 return ((Frame(lseg) ShiftedLeft 12L) Or (Bit_32(offset) + Target(lseg)));
EndCode
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                                 target                                   |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
bit_32 target()
BeginDeclarations
group_entry_ptr                        grp;
#define Grp                            (*grp)
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
public_entry_ptr                       pub;
#define Pub                            (*pub)
bit_32                                 target_address;
EndDeclarations
BeginCode
 Using fixup.target_method
  BeginCase
   When 0:  /* Target is segment relative */
    lseg = (lseg_ptr) fixup.target_referent;
    target_address = Lseg.address + Bit_32(fixup.target_offset);
    break;
   When 1:  /* Target is group relative */
    grp = (group_entry_ptr) fixup.target_referent;
    target_address = (*Grp.first_segment).address +
                      Bit_32(fixup.target_offset);
    break;
   When 2:  /* Target is relative to an external */
    pub = (public_entry_ptr) fixup.target_referent;
    target_address = public_target_address(pub) +
                      Bit_32(fixup.target_offset);
    break;
   When 3:  /* Target is absolute */
    target_address = Bit_32(fixup.target_referent) +
                      Bit_32(fixup.target_offset);
    break;
  EndCase;
 return(target_address);
EndCode
#undef Grp
#undef Lseg
#undef Pub
