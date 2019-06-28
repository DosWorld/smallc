cd text
call edit REGISTER.F
fmt <REGISTER.F >REGISTER -np
call edit LABEL.F
cd ..\test
call edit READ.ME
call edit HISTORY
call edit CLIBARC.LST
del ..\CLIB.ARC
ar -u ..\CLIB.ARC <CLIBARC.LST
cd ..
copy test\READ.ME
copy test\HISTORY
copy test\CCC.BAT
copy test\CC.EXE
copy test\CC.H
copy test\CC1.C
copy test\CC2.C
copy test\CC3.C
copy test\CC4.C
copy test\STDIO.H
copy test\NOTICE.H
copy test\CLIB.LIB
copy test\CLIBARC.LST
copy test\AR.EXE
copy test\AR.C

