@echo off
setlocal

set "NUGET_EXE=%~1"
set "NUPKG_PATH=%~2"
set "DEST_ROOT=%~3"

set "PKG_NAME=cef.runtime.win-x64"

echo [CEF] Checking NuGet extraction...

if exist "%DEST_ROOT%\%PKG_NAME%\include" (
    echo [CEF] Already extracted.
    exit /b 0
)

echo [CEF] Extracting NuGet package...
"%NUGET_EXE%" install "%NUPKG_PATH%" -OutputDirectory "%DEST_ROOT%" -NonInteractive

if errorlevel 1 (
    echo [CEF] ERROR: NuGet extraction failed.
    exit /b 1
)

echo [CEF] Extraction complete.
endlocal
exit /b 0