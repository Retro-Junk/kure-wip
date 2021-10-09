@echo off
copy /y KULT.EXE ..\..\..\dbx\disk\kueg
pushd ..\..\..\dbx
dosbox -c "cd kueg" -c "td.exe kult.exe"
popd
