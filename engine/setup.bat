@echo off
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    cls
    echo error: Adminstrator privileges required
    pause
    exit /B

:gotAdmin
    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"

REG ADD HKEY_CURRENT_USER\SOFTWARE\ThoriumEngine\1.0 /v path /d "%cd%" /f
assoc .thproj=thoriumproject
ftype throiumproject="%cd%\bin\win64\ThoriumEditor.exe"