# Firebase Cloud Functions Setup

This will automatically add timestamps to scans created by ESP32 devices.

## Setup Steps

1. **Install Firebase CLI** (if not already installed):
   ```bash
   npm install -g firebase-tools
   ```

2. **Login to Firebase**:
   ```bash
   firebase login
   ```

3. **Initialize Firebase in the project** (if not already done):
   ```bash
   cd mobile_app
   firebase init functions
   ```
   - Select your existing project: `studious-bit-420613`
   - Choose JavaScript
   - Use existing `functions` folder
   - Install dependencies: Yes

4. **Install dependencies**:
   ```bash
   cd functions
   npm install
   ```

5. **Deploy the functions**:
   ```bash
   cd ..
   firebase deploy --only functions
   ```

## What the Function Does

The `processScan` function automatically processes each RFID scan:

1. **Adds server timestamp** - Ensures accurate time even when ESP32 doesn't have internet time
2. **Looks up RFID card** - Searches the `rfidCards` collection for matching RFID
3. **Adds holder information**:
   - `holderName` - Name of the person who owns the RFID card
   - `userId` - User ID of the card owner
   - `guardianId` - For user access to their own scans
4. **Adds driver/vehicle info** - Looks up device details from `devices` collection:
   - `driverName` - Name of the driver
   - `vehicleType` - Type of vehicle (jeep, bus, etc.)
   - `plateNumber` - Vehicle plate number
5. **Sets scan status**:
   - `verified` - RFID found and matched
   - `unregistered` - RFID not found in database
   - `error` - Processing error occurred

### Example Flow:

**ESP32 sends:**
```json
{
  "rfid": "2101485D",
  "deviceId": "DVC0001",
  "status": "pending"
}
```

**Cloud Function enriches to:**
```json
{
  "rfid": "2101485D",
  "deviceId": "DVC0001",
  "status": "verified",
  "holderName": "Juan Dela Cruz",
  "userId": "abc123",
  "guardianId": "abc123",
  "driverName": "Pedro Driver",
  "vehicleType": "Jeep",
  "plateNumber": "ABC-1234",
  "timestamp": "2026-01-15T10:30:00Z",
  "createdAt": "2026-01-15T10:30:00Z"
}
```

## Alternative: Client-side Timestamp (Vue/Mobile App)

If you prefer not to use Cloud Functions, you can add timestamps in your mobile app when displaying scans:

```javascript
import { serverTimestamp } from 'firebase/firestore'

// When creating a scan manually
await addDoc(collection(db, 'scans'), {
  rfid: '...',
  deviceId: '...',
  status: 'pending',
  timestamp: serverTimestamp() // Firebase server time
})
```

## Cost

Firebase Cloud Functions free tier includes:
- 2 million invocations/month
- 400,000 GB-seconds of compute time
- 200,000 CPU-seconds of compute time

This is more than enough for typical RFID scanning usage.
