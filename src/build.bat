@echo off

set SharedCompilerFlags=/I "..\include" -nologo -MT -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4127 -wd4201 -wd4281 -wd4100 -wd4189 -wd4505 -wd4706 -DDEBUG=1 -FC -Z7
set SharedLinkerFlags=/LIBPATH:"..\lib" -incremental:no opengl32.lib glew32s.lib soil2.lib

mkdir ..\build
pushd ..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
cl %SharedCompilerFlags% ..\src\engine.cpp -Fmengine.map -LD /link /PDB:engine_%random%.pdb /EXPORT:GameLoadPlatformAPI /EXPORT:GameGlewInit /EXPORT:GameUpdateAndRender %SharedLinkerFlags%
cl %SharedCompilerFlags% ..\src\win32_main.cpp -Fmmain.map /link -opt:ref user32.lib gdi32.lib dsound.lib winmm.lib %SharedLinkerFlags%
popd 
