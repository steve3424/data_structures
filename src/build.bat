@echo off

set CompilerFlags=-nologo -MT -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4281 -wd4100 -wd4189 -wd4505 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fm
set LinkerFlags=-opt:ref  user32.lib gdi32.lib dsound.lib winmm.lib

mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl %CompilerFlags% ..\src\win32_handmade.cpp /link -subsystem:windows,5.1 %LinkerFlags%

REM 64-bit build
cl %CompilerFlags% ..\src\win32_handmade.cpp /link %LinkerFlags%

popd 
