@echo off
echo ========================================
echo   SMART GREENHOUSE - QUICK DEPLOY
echo ========================================
echo.

:MENU
echo [1] Upload Code to ESP32
echo [2] Check Public IP
echo [3] Open Web Interface (Local)
echo [4] Open Web Interface (Remote)
echo [5] Test Connection
echo [6] Exit
echo.
set /p choice="Select option: "

if "%choice%"=="1" goto UPLOAD
if "%choice%"=="2" goto CHECK_IP
if "%choice%"=="3" goto LOCAL
if "%choice%"=="4" goto REMOTE
if "%choice%"=="5" goto TEST
if "%choice%"=="6" goto EXIT
goto MENU

:UPLOAD
echo.
echo Uploading code to ESP32...
C:\Users\mimis\.platformio\penv\Scripts\platformio.exe run --target upload
echo.
echo Upload completed!
timeout /t 3
goto MENU

:CHECK_IP
echo.
echo Checking Public IP...
powershell -Command "(Invoke-WebRequest -Uri http://ifconfig.me/ip -UseBasicParsing).Content"
echo.
echo Your ESP32 is accessible at: http://[IP_ABOVE]:8083
echo.
pause
goto MENU

:LOCAL
echo.
echo Opening Local Interface (192.168.2.20)...
start http://192.168.2.20
goto MENU

:REMOTE
echo.
set /p remote_ip="Enter Public IP (or press Enter for saved): "
if "%remote_ip%"=="" (
    echo Using saved IP from file...
    if exist public_ip.txt (
        set /p remote_ip=<public_ip.txt
    ) else (
        echo No saved IP found! Run option 2 first.
        pause
        goto MENU
    )
)
echo Opening Remote Interface...
start http://%remote_ip%:8083
goto MENU

:TEST
echo.
echo Testing ESP32 Connection...
echo.
set /p test_ip="Enter IP to test (local or public): "
echo Testing: http://%test_ip%/api
powershell -Command "try { $r = Invoke-WebRequest -Uri http://%test_ip%/api -UseBasicParsing -TimeoutSec 5; Write-Host 'SUCCESS: ESP32 is responding!' -ForegroundColor Green; Write-Host $r.Content } catch { Write-Host 'ERROR: Cannot connect to ESP32' -ForegroundColor Red }"
echo.
pause
goto MENU

:EXIT
echo.
echo Goodbye!
exit
