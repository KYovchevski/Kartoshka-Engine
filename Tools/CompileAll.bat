@echo off



cd ../Shaders
for %%f in (*.hlsl) do (echo %%~dpnf
	CALL %~dp0/ShaderCompile.bat %~dp0/glslc.exe %%~dpnf %1
)  

pause