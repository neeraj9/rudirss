@echo off

set vcvars="%programfiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set configuration=Release
set platform=x64

if -%1-==-- (
    echo "Please specify configuration(Release or Debug)."
    exit /b
) else (
    if /i %1==Debug (
        set configuration=Debug
    )
)

if -%2-==-- (
    echo "Please specify platform(x86 or x64)."
    exit /b
) else (
    if /i %2==x86 (
        set vcvars="%programfiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
        set platform=x86
    )
)

echo "Build projects, configuration: %configuration%, platform: %platform%"

call %vcvars%

setlocal enabledelayedexpansion
set project[0]="%~dp0..\module\libWinHTTP\libWinHttp\libWinHttp.vcxproj"
set project[1]="%~dp0..\module\libmsxml\libmsxml\libmsxml.vcxproj"
for /l %%n in (0,1,1) do (
    MSBuild !project[%%n]! -t:Clean
    MSBuild !project[%%n]! -t:Rebuild -p:Configuration=%configuration% -p:platform=%platform%
)

exit /b
