@ECHO OFF
set LIBPROJECT=libshapefile

setlocal enabledelayedexpansion
for /f %%a in (%~dp0..\..\src\shapefile\VERSION) do (
    echo VERSION: %%a
    set LIBVER=%%a
    goto :libver
)
:libver
echo update for: %LIBPROJECT%-%LIBVER%

set DISTLIBDIR="%~dp0..\..\dist\%LIBPROJECT%-%LIBVER%"


if "%1" == "x64_debug" (
    copy "%~dp0..\..\src\shapefile\shapefile_api.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\shapefile\shapefile_def.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\common\unitypes.h" "%DISTLIBDIR%\include\common\"
    copy "%~dp0..\..\src\common\bo.h" "%DISTLIBDIR%\include\common\"

    copy "%~dp0target\x64\Debug\%LIBPROJECT%.lib" "%DISTLIBDIR%\lib\win64\Debug\"
    copy "%~dp0target\x64\Debug\%LIBPROJECT%.pdb" "%DISTLIBDIR%\lib\win64\Debug\"
)


if "%1" == "x64_release" (
    copy "%~dp0..\..\src\shapefile\shapefile_api.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\shapefile\shapefile_def.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\common\unitypes.h" "%DISTLIBDIR%\include\common\"
    copy "%~dp0..\..\src\common\bo.h" "%DISTLIBDIR%\include\common\"

    copy "%~dp0target\x64\Release\%LIBPROJECT%.lib" "%DISTLIBDIR%\lib\win64\Release\"
)


if "%1" == "x86_debug" (
    copy "%~dp0..\..\src\shapefile\shapefile_api.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\shapefile\shapefile_def.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\common\unitypes.h" "%DISTLIBDIR%\include\common\"
    copy "%~dp0..\..\src\common\bo.h" "%DISTLIBDIR%\include\common\"

    copy "%~dp0target\Win32\Debug\%LIBPROJECT%.lib" "%DISTLIBDIR%\lib\win86\Debug\"
    copy "%~dp0target\Win32\Debug\%LIBPROJECT%.pdb" "%DISTLIBDIR%\lib\win86\Debug\"
)


if "%1" == "x86_release" (
    copy "%~dp0..\..\src\shapefile\shapefile_api.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\shapefile\shapefile_def.h" "%DISTLIBDIR%\include\shapefile\"
    copy "%~dp0..\..\src\common\unitypes.h" "%DISTLIBDIR%\include\common\"
    copy "%~dp0..\..\src\common\bo.h" "%DISTLIBDIR%\include\common\"

    copy "%~dp0target\Win32\Release\%LIBPROJECT%.lib" "%DISTLIBDIR%\lib\win86\Release\"
)