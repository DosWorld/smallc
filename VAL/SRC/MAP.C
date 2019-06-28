/*                                 MAP.C                                   */

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               link_map                                  |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void link_map()
BeginDeclarations
file_info_ptr                          file;
#define File                           (*file)
group_entry_ptr                        grp;
#define Grp                            (*grp)
bit_16                                 i;
lseg_ptr                               lseg;
#define Lseg                           (*lseg)
lseg_ptr                               last_location_lseg;
public_entry_ptr                       pub;
#define Pub                            (*pub)
segment_entry_ptr                      seg;
#define Seg                            (*seg)
bit_32                                 stop_address;
EndDeclarations
BeginCode
 map_start_time = Now;
 If lst_file_list.first IsNull
  Then  /*  No map file to be created. */
   return;
  EndIf;
 file_open_for_write(lst_file_list.first);
 print("\n");
 print(" Start  Stop   Length Name                   Class\n");
 TraverseList(segment_list, seg)
  BeginTraverse
   stop_address = Seg.address+Seg.length;
   If Seg.length Exceeds 0L
    Then
     stop_address--;
    EndIf;
   print(" %05lXH %05lXH %05lXH %-22Fs %-22Fs\n",
         Seg.address,
         stop_address,
         Seg.length,
         (*Seg.segment_name).symbol,
         (*Seg.class_name).symbol);
  EndTraverse;

 If map.set IsTrue
  Then
   If First(group_list) IsNotNull
    Then
     print("\n");
     print(" Origin   Group\n");
     TraverseList(group_list, grp)
      BeginTraverse
       print(" %04X:%1X   %Fs\n",
             CanonicFrame((*Grp.first_segment).address),
             Bit_16((*Grp.first_segment).address & 0xFL),
             (*Grp.group_name).symbol);
      EndTraverse;
    EndIf;

    public_sort_array = (public_entry_ptr_array)
                          allocate_memory
                           (Addr(dynamic_pool),
                            Bit_32(sizeof(public_entry_ptr)) *
                             Bit_32(MAX_PUBLICS_IN_LIBRARY));
   n_publics_to_sort = 0;
   TraverseList(external_list, pub)
    BeginTraverse
     LoopIf(Pub.type_entry IsNot internal);
     public_sort_array[n_publics_to_sort++] = pub;
    EndTraverse;
   TraverseList(lib_file_list, file)
    BeginTraverse
     TraverseList(File.external_list, pub)
      BeginTraverse
       LoopIf(Pub.type_entry IsNot internal);
       public_sort_array[n_publics_to_sort++] = pub;
      EndTraverse;
    EndTraverse;
   If n_publics_to_sort Exceeds 0
    Then
     sort_publics_by_name(0, n_publics_to_sort-1);
     print("\n");
     print("  Address         Publics by Name\n");
     print("\n");
     For i=0; i LessThan n_publics_to_sort; i++
      BeginFor
       pub = public_sort_array[i];
       print(" %04X:%04X  ",
             CanonicFrame(public_frame_address(pub)),
             Bit_16(public_target_address(pub) -
                    public_frame_address(pub)));
       If Pub.Internal.lseg IsNull
        Then
         print("Abs  ");
        Else
         print("     ");
        EndIf;
       print("%Fs\n", Pub.symbol);
      EndFor;
     sort_publics_by_value(0, n_publics_to_sort-1);
     print("\n");
     print("  Address         Publics by Value\n");
     print("\n");
     For i=0; i LessThan n_publics_to_sort; i++
      BeginFor
       pub = public_sort_array[i];
       print(" %04X:%04X  ",
             CanonicFrame(public_frame_address(pub)),
             Bit_16(public_target_address(pub) -
                    public_frame_address(pub)));
       If Pub.Internal.lseg IsNull
        Then
         print("Abs  ");
        Else
         print("     ");
        EndIf;
       print("%Fs\n", Pub.symbol);
      EndFor;
    EndIf;


   print("\n");
   If start_address_found IsTrue
    Then
     print("Program entry point at %04X:%04X\n",
            initial_CS,
            initial_IP);
    EndIf;
  EndIf;

 If detail_level.val Exceeds 0
  Then
   print("\n");
   print("Next Uninitialized Byte(%05lX), EXE header Relocation Items(%u)\n",
         highest_uninitialized_byte,
         n_relocation_items);
   print("\n");
   print("Segment order expression:\n");
   print("\t\"%Fs\"\n", String(ordering.val));
   TraverseList(segment_list, seg)
    BeginTraverse
     print("\n");
     print("Segment(%Fs) Class(%Fs)",
           (*Seg.segment_name).symbol,
           (*Seg.class_name).symbol);
     If Seg.owning_group IsNotNull
      Then
       print(" Group(%Fs)",
             (*(*Seg.owning_group).group_name).symbol);
      EndIf;
     print(" Combine(%s)\n",
           combine_text[Seg.combine]);
     print("\tStart(%05lX) Length(%05lX) Next Uninitialized Byte(%05lX)\n",
           Seg.address,
           Seg.length,
           Seg.highest_uninitialized_byte);
     If detail_level.val Exceeds 1
      Then
       TraverseList(Seg.lsegs, lseg)
        BeginTraverse
         LoopIf(Lseg.length IsZero);
         print("\n");
         print("File(%Fs) Next Uninitialized Byte(%05lX)\n",
               (*Lseg.file).filename,
               Lseg.highest_uninitialized_byte+Lseg.address);
         print("\tModule(%Fs) Address(%05lX) Length(%05lX) Align(%s)\n",
               (*Lseg.tmodule).symbol,
               Lseg.address,
               Lseg.length,
               align_text[Lseg.align]);
         If (detail_level.val Exceeds 2)             AndIf 
            (Lseg.align IsNot absolute_segment)      AndIf
            (Seg.combine IsNot blank_common_combine)
          Then
           map_memory(Lseg.data, Lseg.address, Lseg.length);
          EndIf;
        EndTraverse;
      EndTraverse;
    EndIf;
   If (detail_level.val Exceeds 3) AndIf (exefile IsTrue)
    Then
     print("\n");
     print("EXE file header:\n");
     map_memory(BytePtr(exe_header), 0L, exe_header_size);
    EndIf;
   If detail_level.val Exceeds 4
    Then
     last_location_lseg = Null;
     print("\n");
     print("Fixups:\n");
     file_open_for_read(temp_file);
     file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
     While temp_file_header.rec_typ IsNotZero
      BeginWhile
       If temp_file_header.rec_typ IsNot FIXUPP_record
        Then
         file_position(Bit_32(infile.byte_position) +
                       infile.start_of_buffer_position +
                       Bit_32(temp_file_header.rec_len));
         file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
         ContinueLoop;
        Else
         file_read(BytePtr(Addr(fixup)), temp_file_header.rec_len);
        EndIf;
       If last_location_lseg IsNot temp_file_header.lseg
        Then
         lseg = temp_file_header.lseg;
         seg  = Lseg.segment;
         print("\n");
         print("Fixups for File(%Fs), Module(%Fs), Segment(%Fs)\n",
               (*Lseg.file).filename,
               (*Lseg.tmodule).symbol,
               (*Seg.segment_name).symbol);
        EndIf;
       last_location_lseg = temp_file_header.lseg;
       print("\tLocation(%05lX) Type(%s) Mode(%s)\n",
             (*temp_file_header.lseg).address +
              Bit_32(temp_file_header.offset),
             type_text[fixup.location_type],
             mode_text[fixup.mode]);
       print("\t\tFrame(");
       Using fixup.frame_method
        BeginCase
         When 0:
          lseg = (lseg_ptr) fixup.frame_referent;
          seg  = Lseg.segment;
          print("Seg(%Fs)", (*Seg.segment_name).symbol);
          break;
         When 1:
          grp = (group_entry_ptr) fixup.frame_referent;
          print("Grp(%Fs)", (*Grp.group_name).symbol);
          break;
         When 2:
          pub = (public_entry_ptr) fixup.frame_referent;
          print("Ext(%Fs)", Pub.symbol);
          break;
         When 3:
          print("Abs(%04lX)", ((bit_32)(fixup.frame_referent)));
          break;
         When 4:
          print("Location");
          break;
         When 5:
          print("Target");
          break;
         When 6:
          print("None");
          break;
        EndCase;
       print(") Target(");
       Using fixup.target_method
        BeginCase
         When 0:
          lseg = (lseg_ptr) fixup.target_referent;
          seg  = Lseg.segment;
          print("Seg(%Fs)+%04lX",
                (*Seg.segment_name).symbol,
                Bit_32(fixup.target_offset) +
                 (Lseg.address - Seg.address));
          break;
         When 1:
          grp = (group_entry_ptr) fixup.target_referent;
          print("Grp(%Fs)+%04lX",
                (*Grp.group_name).symbol,
                Bit_32(fixup.target_offset));
          break;
         When 2:
          pub = (public_entry_ptr) fixup.target_referent;
          print("Ext(%Fs)+%04lX",
                Pub.symbol,
                Bit_32(fixup.target_offset));
          break;
         When 3:
          print("Abs(%04lX)", ((bit_32)(fixup.target_referent)));
          break;
        EndCase;
       print(")\n");
       file_read(BytePtr(Addr(temp_file_header)), sizeof(temp_file_header));
      EndWhile;
     file_close_for_read();
    EndIf;
  EndIf;
 file_close_for_write();
 return;
EndCode
#undef File
#undef Grp
#undef Pub
#undef Seg
#undef Lseg

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                               map_memory                                |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void map_memory(byte_ptr data, bit_32 address, bit_32 length)
BeginDeclarations
bit_16                                 i;
bit_32                                 line_address;
char                                   ascii[20];
EndDeclarations
BeginCode
 line_address = address & 0xFFFF0L;
 While length Exceeds 0
  BeginWhile
   print("%05X: ", line_address);
   far_set(BytePtr(ascii), 0, 20);
   For i=0; i<16; i++
    BeginFor
     If i Is 8
      Then
       print(" :");
       strcat(ascii, " : ");
      EndIf;
     If (line_address LessThan address) OrIf (length IsZero)
      Then
       print(" ..");
       ascii[strlen(ascii)] = ' ';
       line_address++;
      Else
       print(" %02X", *data);
       If isprint(*data)
        Then
         ascii[strlen(ascii)] = (char) *data;
        Else
         ascii[strlen(ascii)] = '.';
        EndIf;
       data++;
       line_address++;
       address++;
       length--;
      EndIf;
    EndFor;
   print("  %s\n", ascii);
  EndWhile;
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          sort_publics_by_name                           |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void sort_publics_by_name(bit_16 left, bit_16 right)
BeginDeclarations
bit_16                                 i;
bit_16                                 j;
public_entry_ptr                       temp;
EndDeclarations
BeginCode
 If left NotLessThan right
  Then
   return;
  EndIf;
 i = left;
 j = right;
 While i LessThan j
  BeginWhile
   While i LessThan j
    BeginWhile
     If far_compare((*public_sort_array[i]).symbol,
                    (*public_sort_array[j]).symbol,
                    (*public_sort_array[i]).length+1) Exceeds 0
      Then
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       ExitLoop;
      EndIf;
     j--;
    EndWhile;
   While i LessThan j
    BeginWhile
     If far_compare((*public_sort_array[i]).symbol,
                    (*public_sort_array[j]).symbol,
                    (*public_sort_array[i]).length+1) Exceeds 0
      Then
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       ExitLoop;
      EndIf;
     i++;
    EndWhile;
  EndWhile;
  If i Exceeds 0
   Then
    sort_publics_by_name(left, i-1);
   EndIf;
 sort_publics_by_name(i+1,  right);
 return;
EndCode

/*+-------------------------------------------------------------------------+
  |                                                                         |
  |                          sort_publics_by_value                          |
  |                                                                         |
  +-------------------------------------------------------------------------+*/
void sort_publics_by_value(bit_16 left, bit_16 right)
BeginDeclarations
bit_16                                 i;
bit_16                                 j;
public_entry_ptr                       temp;
EndDeclarations
BeginCode
 If left NotLessThan right
  Then
   return;
  EndIf;
 i = left;
 j = right;
 While i LessThan j
  BeginWhile
   While i LessThan j
    BeginWhile
     If (((*public_sort_array[i]).Internal.lseg IsNull) AndIf
         ((*public_sort_array[j]).Internal.lseg IsNotNull)) OrIf
        (public_target_address(public_sort_array[i]) Exceeds
         public_target_address(public_sort_array[j]))
      Then
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       ExitLoop;
      EndIf;
     j--;
    EndWhile;
   While i LessThan j
    BeginWhile
     If (((*public_sort_array[i]).Internal.lseg IsNull) AndIf
         ((*public_sort_array[j]).Internal.lseg IsNotNull)) OrIf
        (public_target_address(public_sort_array[i]) Exceeds
         public_target_address(public_sort_array[j]))
      Then
       temp                 = public_sort_array[i];
       public_sort_array[i] = public_sort_array[j];
       public_sort_array[j] = temp;
       ExitLoop;
      EndIf;
     i++;
    EndWhile;
  EndWhile;
  If i Exceeds 0
   Then
    sort_publics_by_value(left, i-1);
   EndIf;
 sort_publics_by_value(i+1,  right);
 return;
EndCode

/*
 Start  Stop   Length Name                   Class
 00000H 016F7H 016F8H _TEXT                  CODE
 016F8H 016F8H 00000H C_ETEXT                ENDCODE
 01700H 01741H 00042H NULL                   BEGDATA
 01742H 01967H 00226H _DATA                  DATA
 01968H 01975H 0000EH CDATA                  DATA
 01976H 01976H 00000H XIFB                   DATA
 01976H 01976H 00000H XIF                    DATA
 01976H 01976H 00000H XIFE                   DATA
 01976H 01976H 00000H XIB                    DATA
 01976H 01976H 00000H XI                     DATA
 01976H 01976H 00000H XIE                    DATA
 01976H 01976H 00000H XPB                    DATA
 01976H 01977H 00002H XP                     DATA
 01978H 01978H 00000H XPE                    DATA
 01978H 01978H 00000H XCB                    DATA
 01978H 01978H 00000H XC                     DATA
 01978H 01978H 00000H XCE                    DATA
 01978H 01978H 00000H XCFB                   DATA
 01978H 01978H 00000H XCF                    DATA
 01978H 01978H 00000H XCFE                   DATA
 01978H 01978H 00000H CONST                  CONST
 01978H 0197FH 00008H HDR                    MSG
 01980H 01A4DH 000CEH MSG                    MSG
 01A4EH 01A4FH 00002H PAD                    MSG
 01A50H 01A50H 00001H EPAD                   MSG
 01A52H 01A77H 00026H _BSS                   BSS
 01A78H 01A78H 00000H XOB                    BSS
 01A78H 01A78H 00000H XO                     BSS
 01A78H 01A78H 00000H XOE                    BSS
 01A80H 02085H 00606H c_common               BSS
 02090H 0288FH 00800H STACK                  STACK

 Origin   Group
 0170:0   DGROUP

  Address         Publics by Name

 0170:005C       STKHQQ

  Address         Publics by Value

 0000:0010       _main
 0000:9876  Abs  __acrtmsg
 0000:9876  Abs  __acrtused

Program entry point at 0000:0294
*/
