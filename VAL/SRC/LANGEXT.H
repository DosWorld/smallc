/*                                 LANGEXT.H                               */

/* Fixup some C syntax I really don't like
   De gustibus non est desputandum
   (Concerning taste there is not argument -- or
    Only fools argue where taste is concerned) */


/* Block basic C commands */

#define If                   if (
#define Then                 ) {
#define Else                 } else {
#define ElseIf               } else if (
#define EndIf                }

#define Using                switch (
#define BeginCase            ) {
#define When                 case
#define Otherwise            default
#define EndCase              }

#define Loop                 while(1)
#define BeginLoop            {
#define EndLoop              }
#define ExitLoop             break
#define ContinueLoop         continue
#define ExitIf(cond)         if (cond) break
#define LoopIf(cond)         if (cond) continue

#define While                while (
#define BeginWhile           ) {
#define EndWhile             }

#define Repeat               do
#define BeginRepeat          {
#define RepeatIf             } while ((
#define Until                } while (!(
#define EndRepeat            ))

#define For                  for (
#define BeginFor             ) {
#define EndFor               }

#define BeginDeclarations    {
#define EndDeclarations
#define BeginCode
#define EndCode              }

#define Type                 typedef

#define Structure           struct
#define BeginStructure      {
#define EndStructure        }

#define Union               union
#define BeginUnion          {
#define EndUnion            }

#define Enumeration         enum
#define BeginEnumeration    {
#define EndEnumeration      }

/* Extended C commands */

/* Linked List commands */

#define ListTypeOf(exp) \
Structure exp##_list_struct \
 BeginStructure \
  exp##_ptr                            first; \
  exp##_ptr                            last; \
 EndStructure; \
Type Structure exp##_list_struct       exp##_list

Structure Generic_Element_struct
 BeginStructure
  Structure Generic_Element_struct far   *next;
 EndStructure;

Type Structure Generic_Element_struct  Generic_Element_type;
Type Generic_Element_type far         *Generic_Element_ptr;

ListTypeOf(Generic_Element);

#define TraverseList(lst,elm)  for(elm=lst.first; elm IsNotNull; Next(elm))
#define BeginTraverse        {
#define EndTraverse          }

#define Insert               ListInsert((Generic_Element_ptr)
#define After                ,0,(Generic_Element_ptr)
#define AtEnd                ,1,Null
#define AtBeginning          ,2,Null
#define InList               ,(Generic_Element_list far *) &(
#define EndInsert            ))

#define Delete               ListDelete((Generic_Element_ptr)
#define FromList             ,(Generic_Element_list far *) &(
#define EndDelete            ))

#define Push                 ListInsert((Generic_Element_ptr)
#define OnTo                 ,2,Null,(Generic_Element_list far *) &(
#define EndPush              ))

#define Pop                  ListPop((Generic_Element_list far *) &(
#define InTo                 ),(Generic_Element_ptr *) &(
#define EndPop               ))

#define LastInList(ptr)      ((*ptr).next == NULL)
#define Next(ptr)            ptr = (*ptr).next
#define First(list)          list.first
#define Last(list)           list.last

/* Make operations less cryptic */

/* Logical operators */
#define AndIf                &&
#define OrIf                 ||
#define Not                  !
#define Is                   ==
#define IsNot                !=
#define IsEqualTo            ==
#define Equals               ==
#define IsNotEqualTo         !=
#define LessThan             <
#define NotLessThan          >=
#define LessThanOrEqualTo    <=
#define GreaterThan          >
#define Exceeds              >
#define Positive             > 0
#define Negative             < 0
#define IsZero               == 0
#define IsNull               == 0
#define IsNotNull            != 0
#define IsEmpty              == 0
#define IsNotEmpty           != 0
#define IsNotZero            != 0
#define IsTrue               != 0
#define IsFalse              == 0
#define NotGreaterThan       <=
#define GreaterThanOrEqualTo >=
#define IsIdentifier(x)      (((x>='A') && (x<='Z')) || \
                              ((x>='a') && (x<='z')) || \
                               (x=='_'))
#define IsNumber(x)          ((x>='0') && (x<='9'))

/* Bitwise operators */
#define And                  &
#define Or                   |
#define Xor                  ^
#define Complement           ~
/* Some other operators */
#define Addr(exp)            &(exp)
#define Mod                  %
#define ShiftedLeft          <<
#define ShiftedRight         >>

/* Some useful constants */

#define False                0
#define Null                 0
#define True                 1

/* Some helpful types */
Type unsigned char           bit_8;
Type unsigned int            bit_16;
Type unsigned long           bit_32;
Type unsigned char           byte;
Type byte far               *byte_ptr;
Type void far               *far_ptr;
Type signed char             int_8;
Type signed int              int_16;
Type signed long             int_32;
Type void  /* near */       *near_ptr;
Type char  /* near */       *char_ptr;

Structure string_struct
 BeginStructure
  bit_16                     max_length;
  bit_16                     length;
  byte                       text[1];
 EndStructure;

Type Structure string_struct string_type;
Type string_type far        *string_ptr;

#define String(str)          ((byte_ptr) ((*str).text))
#define Length(str)          (*str).length
#define MaxLength(str)       (*str).max_length
#define FirstCharIn(str)     *String(str)
#define LastCharIn(str)      String(str)[Length(str)-1]

/* Some helpful type casts */

#define Bit_8(exp)           ((bit_8)      (exp))
#define Bit_16(exp)          ((bit_16)     (exp))
#define Bit_32(exp)          ((bit_32)     (exp))
#define Int_8(exp)           ((int_8)      (exp))
#define Int_16(exp)          ((int_16)     (exp))
#define Int_32(exp)          ((int_32)     (exp))
#define CharPtr(exp)         ((char *)     (exp))
#define Byte(exp)            ((byte)       (exp))
#define BytePtr(exp)         ((byte_ptr )  (exp))
#define NearPtr(exp)         ((near_ptr)   (exp))
#define FarPtr(exp)          ((far_ptr)    (exp))
#define StringPtr(exp)       ((string_ptr) (exp))
#define Offset(exp)          ((bit_16)     (exp))
#define Segment(exp)         ((bit_16)     (((bit_32) (exp)) >> 16L))
#define Low(exp)             ((bit_16)     (exp))
#define High(exp)            ((bit_16)     (((bit_32) (exp)) >> 16L))
#define MakeFarPtr(seg,off)  ((far_ptr)    ((((bit_32) (seg)) << 16L)|(off)))
