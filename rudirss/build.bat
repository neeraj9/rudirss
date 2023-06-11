@echo off

set vcvars="%programfiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set configuration=Release
set platform=x64

if -%1-==-- (
    echo "Please specify a project to build."
    exit /b
) else (
    set project=%1
)

if -%2-==-- (
    echo "Please specify configuration(Release or Debug)."
    exit /b
) else (
    if /i %2==Debug (
        set configuration=Debug
    )
)

if -%3-==-- (
    echo "Please specify platform(x86 or x64)."
    exit /b
) else ( 
    if /i %3==x86 (
        set vcvars="%programfiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
        set platform=x86
    )
)

echo "Build project: %project%, configuration: %configuration%, platform: %platform%"

call %vcvars%
MSBuild %project% -t:Clean
MSBuild %project% -t:Rebuild -p:Configuration=%configuration% -p:platform=%platform%

exit
