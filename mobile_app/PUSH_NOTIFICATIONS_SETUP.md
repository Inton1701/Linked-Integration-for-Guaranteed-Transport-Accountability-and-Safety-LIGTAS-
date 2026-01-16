# Push Notifications Setup for Capacitor

## ✅ Completed Steps

1. ✅ Installed `@capacitor/push-notifications` package
2. ✅ Updated `useNotifications.js` composable to support both web and native platforms
3. ✅ Synced Capacitor plugins with native Android project

## 📱 Android Configuration Required

### Step 1: Download google-services.json

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Select your project: **studious-bit-420613**
3. Click the gear icon ⚙️ > **Project settings**
4. Scroll down to **Your apps** section
5. Find your Android app (or add one if it doesn't exist)
   - Package name should be: `com.ligtas.app` (or check `capacitor.config.json` for `appId`)
6. Click **Download google-services.json**

### Step 2: Add google-services.json to Android Project

Copy the downloaded `google-services.json` file to:
```
mobile_app/android/app/google-services.json
```

### Step 3: Verify Android Configuration

The following files should already be configured (Capacitor does this automatically):

**android/build.gradle:**
```gradle
dependencies {
    classpath 'com.google.gms:google-services:4.4.0'
}
```

**android/app/build.gradle:**
```gradle
apply plugin: 'com.google.gms.google-services'
```

## 🧪 Testing Push Notifications

### On Android Device/Emulator:

1. Build and run the app:
   ```bash
   npx cap run android
   ```

2. Login to the app
3. Go to Notifications tab
4. Click "Enable Notifications"
5. Grant permission when prompted
6. The FCM token will be automatically saved to Firestore

### Test Receiving Notifications:

1. Use the ESP32 to scan an RFID card
2. The Cloud Function will send a notification to the device
3. You should see:
   - **Foreground**: Notification appears in the app
   - **Background**: Notification appears in system tray
   - **Tap notification**: Opens the app and navigates to History

## 🌐 Web Push Notifications

Web push notifications continue to work as before using Firebase Cloud Messaging and service workers. No additional setup needed.

## 📋 Key Features

- **✅ Native Android push notifications** using FCM
- **✅ Web push notifications** using service workers
- **✅ Automatic platform detection** (native vs web)
- **✅ Token management** (saves to Firestore)
- **✅ Foreground notifications** (shows in-app)
- **✅ Background notifications** (system tray)
- **✅ Notification actions** (tap to navigate)

## 🔧 Troubleshooting

### No notifications received:
1. Check that FCM token is saved in Firestore users collection
2. Verify `google-services.json` is in the correct location
3. Check Android Studio logcat for errors
4. Ensure notification permissions are granted

### Token not saving:
1. Check console logs in the app
2. Verify Firestore rules allow updating user documents
3. Ensure user is authenticated before registering

### Build errors:
1. Clean and rebuild: `cd android && ./gradlew clean && cd ..`
2. Sync Capacitor: `npx cap sync`
3. Check that `google-services.json` is valid
