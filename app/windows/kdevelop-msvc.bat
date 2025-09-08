@echo off

setlocal enabledelayedexpansion
setlocal enableextensions

REM load Visual Studio 2017 developer command prompt if VS150COMNTOOLS isn't set. Read Windows registry in case VS is not installed on C:\  
for /f "usebackq tokens=3*" %%a in (`reg query "HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\VisualStudio\SxS\VS7" /s`) do (
     set vs15_path=%%a %%b
     if exist "!vs15_path!Common7\Tools\VsDevCmd.bat" (
	set "VS150COMNTOOLS=!vs15_path!Common7\Tools\"
	goto :end
     )
   )
)
:end

set base=
if DEFINED VS150COMNTOOLS (
    set "base=%VS150COMNTOOLS%"
) else ( IF DEFINED VS140COMNTOOLS (
    set "base=%VS140COMNTOOLS%"
) else ( IF DEFINED VS120COMNTOOLS (
    set "base=%VS120COMNTOOLS%"
) else ( IF DEFINED VS110COMNTOOLS (
    set "base=%VS110COMNTOOLS%"
) else ( IF DEFINED VS100COMNTOOLS (
    set "base=%VS100COMNTOOLS%"
) ))))

if NOT DEFINED base (
    START CMD /C "echo The Microsoft Visual C++ compiler was not found on your system, you might not be able to compile programs. && PAUSE"
)

echo Found VS Install: %base%

if DEFINED VS150COMNTOOLS (
    REM Note: VS2017 has a different directory layout compared to previous versions
    set "vcvarsall=%base%..\..\VC\Auxiliary\Build\vcvarsall.bat"
    REM Choosing architecture
    echo Note: Refer to !vcvarsall! for more information
    echo(
    echo Define which compiler for VS2017 to use. Possible architectures are:
    echo  x86_amd64
    echo  x64
    echo  ...
    set /p "arch= Type an arch and press enter...: "

    set script="!vcvarsall!" !arch!
) else ( IF DEFINED base (
    set script="!base!\..\..\VC\vcvarsall.bat"
) )

if DEFINED script (
    call %script%
)

for /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\KDE e.V.\KDevelop" /v Install_Dir 2^>nul`) DO (
    set appdir=%%A %%B
)

if NOT DEFINED appdir (
    for /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\Wow6432Node\KDE e.V.\KDevelop" /v Install_Dir 2^>nul`) DO (
        set appdir=%%A %%B
    )
)

start "" "%appdir%\bin\kdevelop.exe"
