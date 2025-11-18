@echo off
REM Smart Greenhouse Backup Script
REM Usage: backup.bat [description]

set TIMESTAMP=%date:~-4,4%-%date:~-10,2%-%date:~-7,2%_%time:~0,2%-%time:~3,2%-%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%

set BACKUP_DIR=backup\%TIMESTAMP%
if not "%1"=="" set BACKUP_DIR=backup\%TIMESTAMP%_%1

echo Creating backup in %BACKUP_DIR%...
mkdir "%BACKUP_DIR%" 2>nul

echo Copying src files...
copy src\*.* "%BACKUP_DIR%\" >nul

echo Copying important config files...
copy platformio.ini "%BACKUP_DIR%\" >nul 2>nul

echo Backup completed: %BACKUP_DIR%
echo.
echo Contents:
dir "%BACKUP_DIR%" /B

echo.
echo TIP: Always backup before making major changes!
pause
