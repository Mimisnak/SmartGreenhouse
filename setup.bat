@echo off
echo.
echo ================================================
echo   ESP32-S3 Θερμόμετρο - Project Manager
echo ================================================
echo.
echo Διαθέσιμες επιλογές:
echo.
echo [1] Build και Upload στο ESP32-S3
echo [2] Monitor Serial Output
echo [3] Δείτε GitHub Pages σελίδα
echo [4] Καθαρισμός project (.pio build cache)
echo [0] Έξοδος
echo.
set /p choice="Επιλέξτε αριθμό (0-4): "

if "%choice%"=="1" goto build
if "%choice%"=="2" goto monitor
if "%choice%"=="3" goto pages
if "%choice%"=="4" goto clean
if "%choice%"=="0" goto exit
goto invalid

:build
echo.
echo � Building και Upload στο ESP32-S3...
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
    echo 1. Ανοίξτε Serial Monitor (επιλογή 2)
    echo 2. Δείτε το IP address του ESP32-S3
    echo 3. Ρυθμίστε port forwarding στο router
    echo 4. Ανοίξτε GitHub Pages σελίδα (επιλογή 3)
)
echo.
pause
goto start

:monitor
echo.
echo � Serial Monitor - ESP32-S3 Output
echo ===================================
echo Πατήστε Ctrl+C για επιστροφή στο μενού
echo.
pio device monitor
goto start

:pages
echo.
echo 🌐 GitHub Pages - Remote Access
echo ===============================
echo.
echo 📝 Για να λειτουργήσει η απομακρυσμένη πρόσβαση:
echo.
echo 1. 🔧 ΡΥΘΜΙΣΗ ROUTER (Port Forwarding):
echo    • Μπες στο router admin (192.168.1.1 ή 192.168.2.1)
echo    • Βρες "Port Forwarding" ή "Virtual Server"
echo    • Δημιούργησε κανόνα:
echo      - External Port: 8080
echo      - Internal IP: [IP του ESP32-S3]
echo      - Internal Port: 80
echo      - Protocol: TCP
echo.
echo 2. 🌍 ΒΡΕΣ ΤΟ PUBLIC IP ΣΟΥ:
echo    • Πήγαινε στο: https://whatismyipaddress.com/
echo    • Γράψε το IP (π.χ. 123.456.789.123)
echo.
echo 3. 📡 GITHUB PAGES ΣΕΛΙΔΑ:
echo    • URL: https://mimisnak.github.io/SmartGreenhouse/
echo    • Βάλε το Public IP σου και πόρτα 8080
echo    • Κάνε σύνδεση!
echo.
echo Θέλεις να ανοίξεις το GitHub Pages τώρα; (y/n)
set /p open_pages=
if /i "%open_pages%"=="y" (
    start https://mimisnak.github.io/SmartGreenhouse/
)
echo.
pause
goto start

:clean
echo.
echo 🧹 Καθαρισμός Project...
echo ========================
echo Διαγραφή .pio build cache...
rmdir /s /q .pio 2>nul
echo.
echo ✅ Καθαρισμός ολοκληρώθηκε!
echo Στην επόμενη build θα γίνει πλήρες rebuild.
echo.
pause
goto start

:invalid
echo.
echo ❌ Μη έγκυρη επιλογή! Παρακαλώ επιλέξτε 0-4.
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
