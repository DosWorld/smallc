echo off
if x%1 == xa goto load
if x%1 == xA goto load
if x%1 == xb goto load
if x%1 == xB goto load
goto try

:load
echo "load diskette in drive %1:"
pause

if x%2 == xd goto del
if x%2 == xD goto del
if x%2 == x  goto fmt
goto try

:fmt
call format            %1:
goto go

:del
del %1:*.*

:go
echo on
copy read.me           %1:
copy history           %1:
copy ccc.bat           %1:
copy cc.exe            %1:
copy cc.h              %1:
copy cc1.c             %1:
copy cc2.c             %1:
copy cc3.c             %1:
copy cc4.c             %1:
copy notice.h          %1:
copy stdio.h           %1:
copy clib.lib          %1:
copy clib.arc          %1:
copy clibarc.lst       %1:
copy ar.exe            %1:
copy ar.c              %1:
copy \sa\asm.exe       %1:
type notice.h
goto end

:try
echo "try: DISK <drive> [D]"
pause

:end

