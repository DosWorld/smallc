if x%1 == xf goto go
if x%1 == xF goto go
call format            b:
:go
copy read.me           b:
copy text\register     b:
copy d:\ocs\order      b:
copy history           b:
copy *.h               b:
copy *.c               b:
copy proof.bat         b:
copy dict              b:
copy \sc\text\synopsis b:c.txt
copy \st\text\synopsis b:tools.txt
copy \sw\text\synopsis b:windows.txt

