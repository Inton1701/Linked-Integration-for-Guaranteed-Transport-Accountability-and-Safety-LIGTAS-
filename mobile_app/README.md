# L.I.G.T.A.S Mobile App

Native Android mobile app for the L.I.G.T.A.S RFID Tracking System built with Vue 3, Ionic, Capacitor, Tailwind CSS, and Firebase.

## Features

### 🔐 User Roles

#### Admin
- Register and manage RFID scanning devices
- Link devices to driver information (name, age, plate number)
- Manage user accounts
- View system statistics and activity logs

#### Guardian/User
- Register multiple RFID/NFC cards using phone's NFC reader
- Link each card to a passenger/student name
- Receive push notifications when registered cards are scanned
- View scan history with driver and vehicle information
- Track arrivals in real-time

### 📱 Core Functionality

- **NFC Card Registration**: Use your Android phone's NFC reader to register RFID cards
- **Push Notifications**: Instant alerts when passengers scan their cards
- **Real-time Tracking**: Monitor passenger arrivals with timestamps
- **Driver Information**: See which driver picked up/dropped off passengers
- **History Tracking**: View complete scan history with filters

## Tech Stack

- **Frontend**: Vue 3, Ionic Framework, Tailwind CSS
- **Mobile Runtime**: Capacitor
- **Backend**: Firebase (Authentication, Firestore, Cloud Messaging)
- **NFC Integration**: @exxili/capacitor-nfc

## Setup Instructions

### 1. Install Dependencies

```bash
cd mobile_app
npm install
```

### 2. Configure Firebase

1. Create a Firebase project at [https://console.firebase.google.com](https://console.firebase.google.com)
2. Enable Authentication (Email/Password)
3. Create a Firestore database
4. Enable Firebase Cloud Messaging
5. Download your Firebase config
6. Copy `.env.example` to `.env` and fill in your Firebase credentials:

```env
VITE_FIREBASE_API_KEY=your-api-key
VITE_FIREBASE_AUTH_DOMAIN=your-project.firebaseapp.com
VITE_FIREBASE_PROJECT_ID=your-project-id
VITE_FIREBASE_STORAGE_BUCKET=your-project.appspot.com
VITE_FIREBASE_MESSAGING_SENDER_ID=your-sender-id
VITE_FIREBASE_APP_ID=your-app-id
VITE_FIREBASE_VAPID_KEY=your-vapid-key
```

### 3. Firebase Collections Structure

The app uses the following Firestore collections:

#### `users`
```javascript
{
  uid: string,
  email: string,
  name: string,
  role: 'admin' | 'user',
  fcmToken: string,
  createdAt: string
}
```

#### `devices`
```javascript
{
  deviceId: string,
  driverName: string,
  driverBirthdate: string,  // ISO date string, age calculated dynamically
  plateNumber: string,
  active: boolean,
  createdAt: string
}
```

#### `rfidCards`
```javascript
{
  userId: string,
  holderName: string,
  rfid: string,
  createdAt: string
}
```

#### `scans`
```javascript
{
  rfid: string,
  deviceId: string,
  userId: string,
  holderName: string,
  driverName: string,
  driverBirthdate: string,
  driverAge: number,  // calculated from birthdate at scan time
  plateNumber: string,
  timestamp: string
}
```

### 4. Build for Android

```bash
# Sync Capacitor
npm run sync

# Open in Android Studio
npm run open:android

# Build APK in Android Studio or via command line
npm run android:build
```

### 5. Development

```bash
# Run dev server (web preview)
npm run dev

# Run on Android device
npm run android
```

## Backend Integration

The mobile app works with the ESP32 RFID scanner devices. When a card is scanned:

1. ESP32 sends RFID data to your backend server
2. Backend processes the scan and identifies the card holder
3. Backend retrieves device/driver information
4. Backend sends push notification to the guardian's phone via Firebase Cloud Messaging
5. Guardian receives instant notification with arrival details

### Backend API Endpoint

The ESP32 devices should POST to your backend API with this format:

```json
{
  "deviceID": "DVC0001",
  "rfid": "A1B2C3D4"
}
```

Your backend should then call the Firebase Cloud Messaging API to send notifications.

## Firebase Cloud Functions (Optional)

You can use Firebase Cloud Functions to handle the notification logic. See `firebase-functions.js` for reference implementation.

## Security Rules

Add these Firestore security rules:

```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    // Users can read their own data
    match /users/{userId} {
      allow read: if request.auth != null && request.auth.uid == userId;
      allow write: if request.auth != null && request.auth.uid == userId;
    }
    
    // Admins can manage devices
    match /devices/{deviceId} {
      allow read: if request.auth != null;
      allow write: if request.auth != null && get(/databases/$(database)/documents/users/$(request.auth.uid)).data.role == 'admin';
    }
    
    // Users can manage their own RFID cards
    match /rfidCards/{cardId} {
      allow read: if request.auth != null;
      allow create: if request.auth != null;
      allow update, delete: if request.auth != null && resource.data.userId == request.auth.uid;
    }
    
    // Users can read their own scans
    match /scans/{scanId} {
      allow read: if request.auth != null && resource.data.userId == request.auth.uid;
      allow create: if request.auth != null;
    }
  }
}
```

## Permissions

The app requires the following Android permissions:
- NFC (for reading RFID cards)
- Internet (for Firebase connection)
- Notifications (for push alerts)

These are automatically configured in the Capacitor config.

## Troubleshooting

### NFC Not Working
- Ensure NFC is enabled in device settings
- Check that your device supports NFC
- Make sure the app has NFC permissions

### Push Notifications Not Received
- Verify Firebase Cloud Messaging is properly configured
- Check that the user's FCM token is saved in Firestore
- Ensure notifications are enabled in app settings

### Build Errors
- Clear node_modules and reinstall: `rm -rf node_modules && npm install`
- Sync Capacitor: `npm run sync`
- Clean Android build: Delete `android/app/build` folder

## License

MIT

## Support

For issues and questions, please open an issue on GitHub.
