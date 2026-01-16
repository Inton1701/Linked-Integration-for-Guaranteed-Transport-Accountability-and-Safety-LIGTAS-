# Test Notification Setup

## Current Status
- ✅ Notification view is working
- ✅ Service worker registration added
- ⚠️ No notifications found (because no RFID scans have been created yet)

## To Test Notifications:

### Option 1: Register RFID Card and Scan (Recommended)
1. Go to "Register RFID" tab
2. Add a new RFID card with your details
3. Scan that card with your ESP32 device
4. The Cloud Function will automatically create a notification
5. You'll see it appear in the Notifications tab

### Option 2: Manually Create Test Notification in Firebase Console
1. Go to Firebase Console: https://console.firebase.google.com
2. Navigate to Firestore Database
3. Create a new document in the `notifications` collection:
   ```json
   {
     "userId": "WelYEH6UFkYipIAgMyVqWGQzQsJ2",
     "title": "🔔 RFID Card Scanned",
     "body": "Your card was scanned at Test Device - Main Entrance",
     "scanId": "test123",
     "read": false,
     "createdAt": (use server timestamp)
   }
   ```
4. Refresh the Notifications page - you should see it!

### Option 3: Test via Cloud Function
The Cloud Function `processScan` will automatically create notifications when:
- A scan document is created in the `scans` collection
- The RFID card exists in `rfidCards` collection
- The RFID card has a `userId` field matching a user

## FCM Token Issue
The FCM token error is expected if:
- You haven't added a VAPID key (not required for testing)
- Browser doesn't support push notifications

For production, get your VAPID key:
1. Go to Firebase Console > Project Settings > Cloud Messaging
2. Scroll to "Web Push certificates"
3. Generate a key pair (if not already generated)
4. Copy the key and update `useNotifications.js` line 41

## Next Steps
1. Register an RFID card
2. Scan it with ESP32
3. Watch notification appear automatically!
