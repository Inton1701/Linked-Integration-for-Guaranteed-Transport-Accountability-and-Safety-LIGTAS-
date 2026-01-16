#!/bin/bash

echo "================================================"
echo "L.I.G.T.A.S Mobile App - Setup Script"
echo "================================================"

# Install base dependencies
echo ""
echo "📦 Installing base dependencies..."
npm install

# Install Ionic CLI globally (if not already installed)
echo ""
echo "🔧 Installing Ionic CLI..."
npm install -g @ionic/cli

# Install Capacitor
echo ""
echo "⚡ Installing Capacitor..."
npm install @capacitor/core @capacitor/cli

# Initialize Capacitor (if not already initialized)
echo ""
echo "🚀 Initializing Capacitor..."
npx cap init "L.I.G.T.A.S" "com.ligtas.rfid" --web-dir=dist

# Install Capacitor plugins
echo ""
echo "📱 Installing Capacitor plugins..."
npm install @capacitor/android @capacitor/app @capacitor/haptics @capacitor/keyboard @capacitor/push-notifications @capacitor/status-bar

# Install NFC plugin
echo ""
echo "📡 Installing NFC plugin..."
npm install @exxili/capacitor-nfc

# Add Android platform
echo ""
echo "🤖 Adding Android platform..."
npx cap add android

# Sync Capacitor
echo ""
echo "🔄 Syncing Capacitor..."
npx cap sync

echo ""
echo "================================================"
echo "✅ Setup Complete!"
echo "================================================"
echo ""
echo "Next steps:"
echo "1. Configure Firebase - Copy .env.example to .env and add your credentials"
echo "2. Run development server: npm run dev"
echo "3. Build for Android: npm run android:build"
echo "4. Open in Android Studio: npm run open:android"
echo ""
