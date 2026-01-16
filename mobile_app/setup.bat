@echo off
echo ================================================
echo L.I.G.T.A.S Mobile App - Setup Script
echo ================================================

REM Install base dependencies
echo.
echo 📦 Installing base dependencies...
call npm install

REM Install Ionic CLI globally (if not already installed)
echo.
echo 🔧 Installing Ionic CLI...
call npm install -g @ionic/cli

REM Install Capacitor
echo.
echo ⚡ Installing Capacitor...
call npm install @capacitor/core @capacitor/cli

REM Initialize Capacitor (if not already initialized)
echo.
echo 🚀 Initializing Capacitor...
call npx cap init "L.I.G.T.A.S" "com.ligtas.rfid" --web-dir=dist

REM Install Capacitor plugins
echo.
echo 📱 Installing Capacitor plugins...
call npm install @capacitor/android @capacitor/app @capacitor/haptics @capacitor/keyboard @capacitor/push-notifications @capacitor/status-bar

REM Install NFC plugin
echo.
echo 📡 Installing NFC plugin...
call npm install @exxili/capacitor-nfc

REM Add Android platform
echo.
echo 🤖 Adding Android platform...
call npx cap add android

REM Sync Capacitor
echo.
echo 🔄 Syncing Capacitor...
call npx cap sync

echo.
echo ================================================
echo ✅ Setup Complete!
echo ================================================
echo.
echo Next steps:
echo 1. Configure Firebase - Copy .env.example to .env and add your credentials
echo 2. Run development server: npm run dev
echo 3. Build for Android: npm run android:build
echo 4. Open in Android Studio: npm run open:android
echo.
pause
