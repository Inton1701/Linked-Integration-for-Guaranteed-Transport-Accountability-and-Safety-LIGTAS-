import { ref, onMounted, onUnmounted } from 'vue'
import { PushNotifications } from '@capacitor/push-notifications'
import { Capacitor } from '@capacitor/core'
import { doc, updateDoc } from 'firebase/firestore'
import { db, auth } from '@/firebase'

export function useNotifications() {
  const notificationPermission = ref('default')
  const fcmToken = ref(null)
  const currentNotification = ref(null)

  const isNative = Capacitor.isNativePlatform()

  // Request notification permission
  const requestPermission = async () => {
    try {
      if (isNative) {
        // Native (Android/iOS) implementation
        const result = await PushNotifications.requestPermissions()
        notificationPermission.value = result.receive === 'granted' ? 'granted' : 'denied'

        if (result.receive === 'granted') {
          await PushNotifications.register()
        }

        return result.receive
      } else {
        // Web implementation
        const permission = await Notification.requestPermission()
        notificationPermission.value = permission

        if (permission === 'granted') {
          await getAndSaveToken()
        }

        return permission
      }
    } catch (error) {
      console.error('Error requesting notification permission:', error)
      return 'denied'
    }
  }

  // Get FCM token and save to Firestore
  const getAndSaveToken = async () => {
    try {
      const user = auth.currentUser
      if (!user) {
        console.warn('No user authenticated')
        return null
      }

      let token = null

      if (isNative) {
        // Native platform - token comes from registration listener
        // We'll save it in the listener below
        return null
      } else {
        // Web platform - use Firebase messaging
        if ('serviceWorker' in navigator) {
          try {
            const registration = await navigator.serviceWorker.register('/firebase-messaging-sw.js')
            console.log('Service Worker registered:', registration)
          } catch (swError) {
            console.error('Service Worker registration failed:', swError)
          }
        }

        // Import Firebase messaging dynamically for web only
        const { getToken } = await import('firebase/messaging')
        const { messaging } = await import('@/firebase')

        if (!messaging) {
          console.warn('Messaging not supported on this browser')
          return null
        }

        token = await getToken(messaging, {
          // vapidKey: 'YOUR_VAPID_KEY_HERE'  // Uncomment for production
        })
      }

      if (token) {
        fcmToken.value = token
        console.log('FCM Token obtained:', token)

        // Save token to user document
        const userRef = doc(db, 'users', user.uid)
        await updateDoc(userRef, {
          fcmToken: token,
          fcmTokenUpdatedAt: new Date()
        })
        console.log('FCM token saved to Firestore')

        return token
      }
    } catch (error) {
      console.error('Error getting FCM token:', error)
      return null
    }
  }

  // Setup native push notification listeners
  const setupNativeListeners = () => {
    // Registration success
    PushNotifications.addListener('registration', async (token) => {
      console.log('Push registration success, token:', token.value)
      fcmToken.value = token.value

      // Save token to Firestore (only if user is authenticated)
      try {
        const user = auth.currentUser
        if (user) {
          const userRef = doc(db, 'users', user.uid)
          await updateDoc(userRef, {
            fcmToken: token.value,
            fcmTokenUpdatedAt: new Date()
          })
          console.log('FCM token saved to Firestore')
        } else {
          console.log('User not authenticated yet, token will be saved on login')
        }
      } catch (error) {
        console.error('Error saving FCM token:', error)
      }
    })

    // Registration error
    PushNotifications.addListener('registrationError', (error) => {
      console.error('Push registration error:', error)
    })

    // Notification received while app is in foreground
    PushNotifications.addListener('pushNotificationReceived', (notification) => {
      console.log('Push notification received:', notification)
      
      currentNotification.value = {
        title: notification.title || 'New Notification',
        body: notification.body || '',
        data: notification.data || {}
      }
    })

    // Notification tapped by user
    PushNotifications.addListener('pushNotificationActionPerformed', (notification) => {
      console.log('Push notification action performed:', notification)
      
      // Handle notification tap - navigate to history view
      try {
        const router = require('vue-router').useRouter()
        router?.push('/user/history')
      } catch (error) {
        console.error('Error handling notification action:', error)
      }
    })
  }

  // Setup web message listener
  const setupWebMessageListener = async () => {
    try {
      const { onMessage } = await import('firebase/messaging')
      const { messaging } = await import('@/firebase')

      if (!messaging) return

      onMessage(messaging, (payload) => {
        console.log('Foreground message received:', payload)
        
        currentNotification.value = {
          title: payload.notification?.title || 'New Notification',
          body: payload.notification?.body || '',
          data: payload.data || {}
        }

        // Show browser notification
        if (Notification.permission === 'granted') {
          new Notification(payload.notification?.title || 'New Notification', {
            body: payload.notification?.body || '',
            icon: '/icon.png',
            badge: '/badge.png',
            tag: payload.data?.scanId || 'notification'
          })
        }
      })
    } catch (error) {
      console.error('Error setting up message listener:', error)
    }
  }

  // Initialize
  const initialize = async () => {
    if (isNative) {
      setupNativeListeners()
      
      // Check current permission status
      const permStatus = await PushNotifications.checkPermissions()
      notificationPermission.value = permStatus.receive === 'granted' ? 'granted' : 'default'
      
      if (permStatus.receive === 'granted') {
        await PushNotifications.register()
      }
    } else {
      notificationPermission.value = Notification.permission
      
      if (Notification.permission === 'granted') {
        await getAndSaveToken()
      }
      
      await setupWebMessageListener()
    }
  }

  // Initialize on mount
  onMounted(() => {
    initialize()
  })

  // Cleanup on unmount
  onUnmounted(() => {
    if (isNative) {
      PushNotifications.removeAllListeners()
    }
  })

  return {
    notificationPermission,
    fcmToken,
    currentNotification,
    requestPermission,
    getAndSaveToken,
    isNative
  }
}
