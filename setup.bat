@echo off
echo.
echo ================================================
echo   ESP32-S3 Θερμόμετρο - Version Manager
echo ================================================
echo.
echo Διαθέσιμες εκδόσεις:
echo.
echo [1] Απλό Θερμόμετρο (Serial Monitor μόνο)
echo [2] Web Θερμόμετρο (Wi-Fi + Web Server)
echo [3] Δείτε τρέχουσα έκδοση
echo [4] Build και Upload
echo [5] Monitor Serial Output
echo [0] Έξοδος
echo.
set /p choice="Επιλέξτε αριθμό (0-5): "

if "%choice%"=="1" goto simple
if "%choice%"=="2" goto web
if "%choice%"=="3" goto current
if "%choice%"=="4" goto build
if "%choice%"=="5" goto monitor
if "%choice%"=="0" goto exit
goto invalid

:simple
echo.
echo 🔄 Αλλαγή σε Απλό Θερμόμετρο...
copy "src\main.cpp" "src\main_backup.cpp" >nul 2>&1
copy "src\main_simple.cpp" "src\main.cpp" >nul 2>&1
if not exist "src\main_simple.cpp" (
    echo ❌ Αρχείο src\main_simple.cpp δεν βρέθηκε!
    echo Χρησιμοποιώντας το υπάρχον main.cpp
) else (
    echo ✅ Αλλαγή ολοκληρώθηκε!
    echo 📄 Τώρα χρησιμοποιείται: Απλό Θερμόμετρο (Serial μόνο)
)
echo.
pause
goto start

:web
echo.
echo 🔄 Αλλαγή σε Web Θερμόμετρο...
copy "src\main.cpp" "src\main_backup.cpp" >nul 2>&1
copy "src\main_web.cpp" "src\main.cpp" >nul 2>&1
if not exist "src\main_web.cpp" (
    echo ❌ Αρχείο src\main_web.cpp δεν βρέθηκε!
    echo Δημιουργήστε πρώτα το web version!
) else (
    echo ✅ Αλλαγή ολοκληρώθηκε!
    echo 📄 Τώρα χρησιμοποιείται: Web Θερμόμετρο (Wi-Fi + Server)
    echo.
    echo ⚠️  ΣΗΜΑΝΤΙΚΟ:
    echo    Πρέπει να ενημερώσετε το Wi-Fi όνομα και κωδικό στο src\main.cpp
    echo    Αναζητήστε τις γραμμές:
    echo    const char* ssid = "YOUR_WIFI_NAME";
    echo    const char* password = "YOUR_WIFI_PASSWORD";
)
echo.
pause
goto start

:current
echo.
echo 📄 Τρέχουσα έκδοση main.cpp:
echo =============================================
findstr "ESP32-S3.*Θερμόμετρο" src\main.cpp
if errorlevel 1 (
    echo Δεν βρέθηκε αναγνωριστικό στο main.cpp
) 
echo.
findstr "WiFi\|WebServer" src\main.cpp >nul
if errorlevel 1 (
    echo 🔍 Τύπος: Απλό Θερμόμετρο (χωρίς Wi-Fi)
) else (
    echo 🔍 Τύπος: Web Θερμόμετρο (με Wi-Fi)
)
echo.
pause
goto start

:build
echo.
echo 🔨 Building και Upload στο ESP32-S3...
echo ======================================
pio run --target upload
if errorlevel 1 (
    echo.
    echo ❌ Build/Upload αποτυχία!
    echo.
    echo Πιθανές αιτίες:
    echo - ESP32-S3 δεν είναι συνδεδεμένο
    echo - Λάθος COM port
    echo - Κώδικας έχει σφάλματα
    echo.
    echo Δοκιμάστε:
    echo pio device list
) else (
    echo.
    echo ✅ Build/Upload ολοκληρώθηκε επιτυχώς!
    echo.
    echo Επόμενα βήματα:
    echo 1. Ανοίξτε Serial Monitor (επιλογή 5)
    echo 2. Δείτε το IP address (αν χρησιμοποιείτε Web έκδοση)
    echo 3. Ανοίξτε browser στο IP address
)
echo.
pause
goto start

:monitor
echo.
echo 📺 Serial Monitor - ESP32-S3 Output
echo ===================================
echo Πατήστε Ctrl+C για επιστροφή στο μενού
echo.
pio device monitor
goto start

:invalid
echo.
echo ❌ Μη έγκυρη επιλογή! Παρακαλώ επιλέξτε 0-5.
echo.
pause
goto start

:exit
echo.
echo 👋 Αντίο!
echo.
exit /b 0

:start
cls
goto start
