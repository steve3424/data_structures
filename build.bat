@echo off

mkdir exe
pushd exe
cl -FC -Zi ..\main.cpp user32.lib gdi32.lib dsound.lib
popd 
