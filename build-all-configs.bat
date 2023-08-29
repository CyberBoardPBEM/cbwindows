echo on

rem change the following if your Visual Studio is installed somewhere else.
set VSROOTDIR="C:\Program Files\Microsoft Visual Studio"

rem execute in bat's directory
pushd %~dp0

call %VSROOTDIR%"\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 goto fail_vc_x64
echo on

cmake -G Ninja -B out/build/x64-Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x64-Debug

cmake -G Ninja -B out/build/x64-Release -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build out/build/x64-Release

call %VSROOTDIR%"\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
if errorlevel 1 goto fail_vc_x86
echo on

cmake -G Ninja -B out/build/x86-Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out/build/x86-Debug

cmake -G Ninja -B out/build/x86-Release -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build out/build/x86-Release

goto done

:fail_vc_x64
echo vcvars64.bat not found
goto done

:fail_vc_x86
echo vcvars32.bat not found
goto done

:done
popd
