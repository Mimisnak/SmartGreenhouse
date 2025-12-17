@echo off
echo ================================================
echo     ESP32-S3 UPLOAD HELPER
echo ================================================
echo.
echo Βηματα:
echo 1. Κρατα πατημενο το BOOT κουμπι
echo 2. Πατα το RESET κουμπι
echo 3. Αφησε το RESET
echo 4. Αφησε το BOOT
echo 5. Πατα οποιοδηποτε πληκτρο ΕΔΩ για upload
echo.
pause
echo.
echo ⏳ Uploading firmware...
C:\Users\mimis\.platformio\penv\Scripts\platformio.exe run --target upload
echo.
if %errorlevel% EQU 0 (
    echo ✅ Upload SUCCESS!
    echo.
    echo Ανοιγμα Serial Monitor...
    C:\Users\mimis\.platformio\penv\Scripts\platformio.exe device monitor
) else (
    echo ❌ Upload FAILED!
    echo.
    echo Δοκιμασε ξανα με τα κουμπια BOOT + RESET
)
pause
