@echo off
pushd "%~dp0"
@echo on
copy "foobar2000\components\foo_WMircP.dll" "%programfiles%\foobar2000\components"
copy "foobar2000\components\foo_WMircP.pdb" "%programfiles%\foobar2000\components"
@echo off
popd
pause