Rem 1 - glslc.exe directory
Rem 2 - .hlsl file directory without the file extension
@echo off

glslc.exe %1.glsl -o %1.spv

copy %1.spv %2
rem echo Finished compiling %2.spv