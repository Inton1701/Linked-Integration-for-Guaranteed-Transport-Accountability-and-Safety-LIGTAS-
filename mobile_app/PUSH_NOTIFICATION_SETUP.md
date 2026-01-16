# Push Notification Setup Guide

## Overview
Push notifications are now configured to alert users when their RFID card is scanned by the ESP32 device.

## Setup Instructions

### 1. Firebase Cloud Functions Setup

The Cloud Functions are already configured in `functions/index.js`. The `processScan` function:
- Triggers when a new scan document is created
- Looks up the RFID card owner
- Retrieves the user's FCM token
- Sends a push notification
- Stores the notification in Firestore

**Deploy the Cloud Functions:**
```bash
cd mobile_app
firebase deploy --only functions
```

### 2. Get VAPID Key from Firebase Console

1. Go to [Firebase Console](https://console.firebase.google.com)
2. Select your project: `studious-bit-420613`
3. Go to **Project Settings** > **Cloud Messaging**
4. Scroll to **Web Push certificates**
5. Generate a new key pair (if not already generated)
6. Copy the **Key pair** (VAPID key)

### 3. Update VAPID Key in Mobile App

Edit `mobile_app/src/composables/useNotifications.js`:

```javascript
const token = await getToken(messaging, {
  vapidKey: 'YOUR_VAPID_KEY_HERE'  // Replace with actual key from Firebase Console
})
```

### 4. Update Firestore Security Rules

Ensure these rules are in `firestore.rules`:

```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    // Users collection
    match /users/{userId} {
      allow read, write: if request.auth != null && request.auth.uid == userId;
    }
    
    // Notifications collection
    match /notifications/{notificationId} {
      allow read, update: if request.auth != null && 
        resource.data.userId == request.auth.uid;
      allow create: if true; // Cloud Functions create notifications
    }
    
    // RFID Cards collection
    match /rfidCards/{cardId} {
      allow read: if request.auth != null;
      allow write: if request.auth != null;
    }
    
    // Devices collection (ESP32 needs read access)
    match /devices/{deviceId} {
      allow read: if true; // Allow ESP32 to read device info
      allow write: if request.auth != null;
    }
    
    // Scans collection (ESP32 needs write access)
    match /scans/{scanId} {
      allow read: if request.auth != null;
      allow create: if true; // Allow ESP32 to create scans
      allow update, delete: if request.auth != null;
    }
  }
}
```

Deploy rules:
```bash
firebase deploy --only firestore:rules
```

### 5. Ensure RFID Cards Have User IDs

When users register their RFID cards, make sure the `userId` field is populated:

```javascript
// In RegisterRFIDView.vue or wherever cards are registered
await addDoc(collection(db, 'rfidCards'), {
  rfid: cardData.rfid,
  userId: auth.currentUser.uid,  // IMPORTANT: Link card to user
  holderName: cardData.holderName,
  holderEmail: cardData.holderEmail,
  holderPhone: cardData.holderPhone,
  registeredAt: serverTimestamp()
})
```

### 6. Test the Notification Flow

1. **Register User**: Create an account in the mobile app
2. **Register RFID Card**: Link an RFID card to the user account
3. **Allow Notifications**: When prompted, allow notification permissions
4. **Scan RFID Card**: Tap the card on the ESP32 NFC reader
5. **Receive Notification**: User should receive a push notification

### 7. Testing on Different Platforms

#### Web (PWA):
- Notifications work on Chrome, Edge, Firefox
- HTTPS required (localhost works for testing)

#### Android (Capacitor):
- Build and install the APK
- Notifications work automatically
- No additional configuration needed

#### iOS (Capacitor):
- Requires Apple Developer account
- Configure push notifications in Xcode
- Add `Push Notifications` capability

## Notification Data Structure

### Notification Document (Firestore)
```javascript
{
  userId: "user123",
  title: "🔔 RFID Card Scanned",
  body: "Your card was scanned at Device A - Main Entrance",
  scanId: "scan123",
  read: false,
  createdAt: Timestamp
}
```

### FCM Message Payload
```javascript
{
  notification: {
    title: "🔔 RFID Card Scanned",
    body: "Your card was scanned at Device A - Main Entrance"
  },
  data: {
    scanId: "scan123",
    rfid: "04:AB:CD:EF",
    deviceId: "ESP32_001",
    timestamp: "2024-01-01T12:00:00Z",
    type: "rfid_scan"
  },
  token: "fcm_token_here"
}
```

## Troubleshooting

### Notifications not received:
1. Check FCM token is saved in user document
2. Verify notification permissions are granted
3. Check Cloud Functions logs: `firebase functions:log`
4. Ensure RFID card has valid `userId` field

### FCM Token not generated:
1. Verify VAPID key is correct
2. Check browser console for errors
3. Ensure HTTPS is used (or localhost)

### Cloud Functions not triggering:
1. Check function deployment: `firebase functions:list`
2. View logs: `firebase functions:log`
3. Verify Firestore trigger path: `/scans/{scanId}`

## Files Modified/Created

1. **functions/index.js** - Added FCM notification sending
2. **src/composables/useNotifications.js** - FCM token management
3. **src/views/user/NotificationsView.vue** - Notifications UI
4. **src/views/user/UserLayout.vue** - Added notification tab with badge
5. **src/router/index.js** - Added notifications route
6. **src/App.vue** - Initialize notifications on app load
7. **public/firebase-messaging-sw.js** - Service worker for background notifications

## Next Steps

1. Get VAPID key from Firebase Console
2. Update `useNotifications.js` with VAPID key
3. Deploy Cloud Functions: `firebase deploy --only functions`
4. Deploy Firestore rules: `firebase deploy --only firestore:rules`
5. Test end-to-end notification flow
6. Configure iOS push notifications (if targeting iOS)
