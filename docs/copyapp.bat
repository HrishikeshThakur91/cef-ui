setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/bootstrap.exe C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/cefsimple.exe
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/chrome_elf.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/chrome_elf.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/d3dcompiler_47.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/d3dcompiler_47.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/libcef.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/libcef.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/libEGL.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/libEGL.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/libGLESv2.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/libGLESv2.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/v8_context_snapshot.bin C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/v8_context_snapshot.bin
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/vk_swiftshader.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/vk_swiftshader.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/vk_swiftshader_icd.json C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/vk_swiftshader_icd.json
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/vulkan-1.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/vulkan-1.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/dxil.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/dxil.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/bin/Debug/dxcompiler.dll C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/dxcompiler.dll
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/Resources/chrome_100_percent.pak C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/chrome_100_percent.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/Resources/chrome_200_percent.pak C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/chrome_200_percent.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/Resources/resources.pak C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/resources.pak
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_if_different C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/Resources/icudtl.dat C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/icudtl.dat
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
"C:\Program Files\CMake\bin\cmake.exe" -E copy_directory C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/Resources/locales C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug/locales
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd
setlocal
icacls C:\Workspace\cef-ui2\cef-ui\packages\cef.runtime.win-x64.143.0.7499.170/build_dir/tests/cefsimple/Debug /grant *S-1-15-2-2:(OI)(CI)(RX)
if %errorlevel% neq 0 goto :cmEnd
:cmEnd
endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone
:cmErrorLevel
exit /b %1
:cmDone
if %errorlevel% neq 0 goto :VCEnd