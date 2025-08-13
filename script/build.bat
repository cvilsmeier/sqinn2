REM Must have MSVC Build Tools installed
rd /S /Q bin
md bin
cl.exe /Fe:bin\sqinn.exe src\*.c
