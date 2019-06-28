echo off
echo ---------------------------------------------------------------
echo This is a crude form of spelling checker which illustrates
echo how some of the various Small-Tools programs can be used in
echo concert to accomplish a task.  In step 6, EDT is invoked on
echo PROOF3.TMP which contains words not found in DICT.  If you
echo note and delete the misspelled words and write the remaining
echo (good) words back to PROOF3.TMP and exit from EDT, the next
echo step will create a new dictionary augmented with the new words.
echo ---------------------------------------------------------------
echo Command: PROOF document
echo ---------------------------------------------------------------
pause
echo on
trn <%1 >proof1.tmp ~a-zA-Z:-:' :n
srt <proof1.tmp >proof2.tmp -u
del  proof1.tmp
mrg  proof2.tmp dict -1 >proof3.tmp
del  proof2.tmp
edt  proof3.tmp
pause enter control-c to skip dict update
mrg  proof3.tmp dict >dict.new
del  proof3.tmp

