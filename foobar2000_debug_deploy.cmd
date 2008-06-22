@echo off
pushd "~dp0"
copy "SDK-2008-05-27\Debug\foo_foobarMircP.*" "%programfiles%\foobar2000\components"
popd
pause