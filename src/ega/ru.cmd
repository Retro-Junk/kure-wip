@echo off
copy /y KULT.EXE ..\..\..\dbx\disk\kueg
pushd ..\..\..\dbx
dosbox -c "cd kueg" -c kult.exe
popd
