@echo off

mkdir exe
pushd exe
cl -FC -Zi ..\win32_handmade.cpp user32.lib gdi32.lib dsound.lib
popd 
