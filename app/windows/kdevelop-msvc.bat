@echo off

setlocal enabledelayedexpansion
setlocal enableextensions

if DEFINED VS140COMNTOOLS (
    set "base=%VS140COMNTOOLS%"
) else ( IF DEFINED VS120COMNTOOLS (
    set "base=%VS120COMNTOOLS%"
) else ( IF DEFINED VS110COMNTOOLS (
    set "base=%VS110COMNTOOLS%"
) else ( IF DEFINED VS100COMNTOOLS (
    set "base=%VS100COMNTOOLS%"
) else (
    START CMD /C "The Microsoft Visual C++ compiler was not found on your system, you might not be able to compile programs. && PAUSE"
) )))

set script="!base!\..\..\VC\vcvarsall.bat"
call %script%

for /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\KDE\KDevelop" /v Install_Dir 2^>nul`) DO (
    set appdir=%%A %%B
)

if NOT DEFINED appdir (
    for /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\Wow6432Node\KDE\KDevelop" /v Install_Dir 2^>nul`) DO (
        set appdir=%%A %%B
    )
)

start "" "%appdir%\bin\kdevelop.exe"
