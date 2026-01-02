REM echo Copying CEF runtime files...
REM 
REM set CEF_ROOT=$(SolutionDir)3rd-party\cef_binary_143.0.7499.170_windows64 
REM set DEST=$(SolutionDir)$(Platform)\$(Configuration)\cef 
REM 
REM if not exist "%DEST%" mkdir "%DEST%" 
REM if not exist "%DEST%\Resources" mkdir "%DEST%\Resources" 
REM 
REM xcopy /E /I /Y "%CEF_ROOT%\$(Configuration)\libcef.dll" "%DEST%" 
REM xcopy /E /I /Y "%CEF_ROOT%\$(Configuration)\chrome_elf.dll" "%DEST%" 
REM 
REM xcopy /E /I /Y "%CEF_ROOT%\Resources" "%DEST%\resources" 
REM 
REM echo CEF runtime copying complete.

@echo off
setlocal

echo [CEF] Copying runtime files...

REM ---- Arguments ----
REM %1 = CEF root directory
REM %2 = Output directory (with trailing slash)

set "CEF_ROOT=%~1"
set "OUT_DIR=%~2"
set "CONFIG=%~3"

REM ---- Validate ----
if not exist "%CEF_ROOT%" (
  echo [CEF] ERROR: CEF_ROOT not found: %CEF_ROOT%
  exit /b 1
)

if not exist "%OUT_DIR%" (
  echo [CEF] ERROR: OUT_DIR not found: %OUT_DIR%
  exit /b 1
)

REM ---- Destination ----
set "DEST=%OUT_DIR%"

if not exist "%DEST%" mkdir "%DEST%"
if not exist "%DEST%\Resources" mkdir "%DEST%\Resources"

REM ---- Copy binaries ----
copy /Y "%CEF_ROOT%\bin\%CONFIG%\libcef.dll" "%DEST%"
copy /Y "%CEF_ROOT%\bin\%CONFIG%\chrome_elf.dll" "%DEST%"

REM ---- Copy resources (includes locales) ----
xcopy /E /I /Y "%CEF_ROOT%\Resources" "%DEST%\Resources"

echo [CEF] Runtime copy complete.
endlocal
exit /b 0
