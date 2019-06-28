echo off
:small
if     %1==s goto small2
if not %1==S goto compact
:small2
tcc -ms -c -O -f- -Lc:\turboc\lib -Ic:\turboc\include %2
goto library

:compact
if     %1==c goto compact2
if not %1==C goto medium
:compact2
tcc -mc -c -O -f- -Lc:\turboc\lib -Ic:\turboc\include %2
goto library

:medium
if     %1==m goto medium2
if not %1==M goto large
:medium2
tcc -mm -c -O -f- -Lc:\turboc\lib -Ic:\turboc\include %2
goto library

:large
if     %1==l goto large2
if not %1==L goto error
:large2
tcc -ml -c -O -f- -Lc:\turboc\lib -Ic:\turboc\include %2
goto library

:error
echo       try: CLIB model filename
goto exit

:library
if errorlevel 1 goto exit
lib sw%1 -+%2;
if errorlevel 1 goto exit
del %2.obj
del sw%1.bak

:exit

