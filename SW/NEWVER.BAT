cd test
call edit SW-LIB.LST
call edit SW-ARC.LST
call edit READ.ME
call edit ..\text\HISTORY
call edit ..\text\LABEL.F
del SW.ARC
ar -u SW.ARC <SW-ARC.LST
cd ..
copy test\READ.ME
copy test\SW-LIB.LST
copy test\SW-ARC.LST
copy test\SW.ARC
copy test\SW.LIB
copy test\WTEST.EXE
copy test\MTEST.EXE

