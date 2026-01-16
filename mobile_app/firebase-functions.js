const admin = require('firebase-admin');

// Initialize Firebase Admin SDK
admin.initializeApp({
  credential: admin.credential.applicationDefault()
});

/**
 * Send push notification to a user
 * @param {string} fcmToken - FCM token of the user's device
 * @param {object} notificationData - Notification data
 */
async function sendPushNotification(fcmToken, notificationData) {
  const message = {
    notification: {
      title: notificationData.title,
      body: notificationData.body
    },
    data: {
      deviceId: notificationData.deviceId || '',
      rfid: notificationData.rfid || '',
      holderName: notificationData.holderName || '',
      driverName: notificationData.driverName || '',
      plateNumber: notificationData.plateNumber || '',
      timestamp: notificationData.timestamp || new Date().toISOString()
    },
    token: fcmToken
  };

  try {
    const response = await admin.messaging().send(message);
    console.log('Successfully sent message:', response);
    return { success: true, messageId: response };
  } catch (error) {
    console.error('Error sending message:', error);
    return { success: false, error: error.message };
  }
}

/**
 * Send notification to guardian when passenger scans RFID
 * Called by the ESP32 device via API
 */
async function notifyGuardian(scanData) {
  const { rfid, deviceId, timestamp } = scanData;

  try {
    const db = admin.firestore();

    // 1. Find the RFID card and get holder info
    const rfidCardSnapshot = await db.collection('rfidCards')
      .where('rfid', '==', rfid)
      .limit(1)
      .get();

    if (rfidCardSnapshot.empty) {
      console.log('RFID card not found:', rfid);
      return { success: false, message: 'RFID card not registered' };
    }

    const rfidCard = rfidCardSnapshot.docs[0].data();
    const userId = rfidCard.userId;
    const holderName = rfidCard.holderName;

    // 2. Get device info
    const deviceSnapshot = await db.collection('devices')
      .where('deviceId', '==', deviceId)
      .limit(1)
      .get();

    let driverInfo = {
      driverName: 'Unknown Driver',
      driverBirthdate: null,
      plateNumber: 'N/A'
    };

    if (!deviceSnapshot.empty) {
      driverInfo = deviceSnapshot.docs[0].data();
    }

    // Calculate driver age from birthdate
    const calculateAge = (birthdate) => {
      if (!birthdate) return 'N/A'
      const today = new Date()
      const birth = new Date(birthdate)
      let age = today.getFullYear() - birth.getFullYear()
      const monthDiff = today.getMonth() - birth.getMonth()
      if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birth.getDate())) {
        age--
      }
      return age
    }
    const driverAge = calculateAge(driverInfo.driverBirthdate)

    // 3. Get user's FCM token
    const userDoc = await db.collection('users').doc(userId).get();
    
    if (!userDoc.exists) {
      console.log('User not found:', userId);
      return { success: false, message: 'User not found' };
    }

    const userData = userDoc.data();
    const fcmToken = userData.fcmToken;

    if (!fcmToken) {
      console.log('User has no FCM token:', userId);
      return { success: false, message: 'User has no FCM token' };
    }

    // 4. Save scan record to database
    await db.collection('scans').add({
      rfid,
      deviceId,
      userId,
      holderName,
      driverName: driverInfo.driverName,
      driverBirthdate: driverInfo.driverBirthdate,
      driverAge,
      plateNumber: driverInfo.plateNumber,
      timestamp: timestamp || new Date().toISOString()
    });

    // 5. Send push notification
    const notificationResult = await sendPushNotification(fcmToken, {
      title: `${holderName} Arrived!`,
      body: `Driver: ${driverInfo.driverName} (${driverInfo.plateNumber})`,
      deviceId,
      rfid,
      holderName,
      driverName: driverInfo.driverName,
      plateNumber: driverInfo.plateNumber,
      timestamp: timestamp || new Date().toISOString()
    });

    return {
      success: true,
      message: 'Notification sent successfully',
      notificationResult
    };
  } catch (error) {
    console.error('Error in notifyGuardian:', error);
    return { success: false, error: error.message };
  }
}

module.exports = {
  sendPushNotification,
  notifyGuardian
};
