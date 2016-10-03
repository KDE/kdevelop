setlocal EnableDelayedExpansion
setlocal enableextensions

if DEFINED VS140COMNTOOLS (
    SET "base=%VS140COMNTOOLS%"
) else ( IF DEFINED VS120COMNTOOLS (
    SET "base=%VS120COMNTOOLS%"
) else ( IF DEFINED VS110COMNTOOLS (
    SET "base=%VS110COMNTOOLS%"
) else ( IF DEFINED VS100COMNTOOLS (
    SET "base=%VS100COMNTOOLS%"
) else (
    START CMD /C "The Microsoft Visual C++ compiler was not found on your system, you might not be able to compile programs. && PAUSE"
) )))

SET script="!base!/../../VC/vcvarsall.bat"^
CALL %script%

FOR /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\KDE\KDevelop" /v Install_Dir`) DO (
    set appdir=%%A %%B
)

if NOT DEFINED appdir (
    FOR /F "usebackq tokens=3*" %%A IN (`REG QUERY "HKEY_LOCAL_MACHINE\Software\Wow6432Node\KDE\KDevelop" /v Install_Dir`) DO (
        set appdir=%%A %%B
    )
)

START "" "%appdir%/bin/kdevelop.exe"
