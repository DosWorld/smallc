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
call format        %1:
goto go

:del
del %1:*.*

:go
echo on
copy read.me         %1:
copy text\register   %1:
copy text\history    %1:
copy d:\ocs\order    %1:
copy d:\sc\ar.c      %1:
copy d:\sc\ar.exe    %1:
copy *.lib           %1:
copy *.arc           %1:
copy *.lst           %1:
copy *.exe           %1:
copy book\manual.out %1:
dir %1:
goto end

:try
echo "try: DISK <drive> [D]"
pause

:end

