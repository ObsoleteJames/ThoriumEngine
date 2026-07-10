@echo off

for /f "tokens=2*" %%A in ('reg query "HKEY_CURRENT_USER\SOFTWARE\ThoriumEngine\1.0" /v "path" 2^>nul') do (
    set "REG_VALUE=%%B"
)

if defined REG_VALUE (
    "%REG_VALUE%\bin\win64\buildtool.exe" "%cd%\Build.cfg"
    cmake -A x64 -Wno-dev -DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF -B "Intermediate/build" "Intermediate"
) else (
    echo Couldn't find engine path.
)