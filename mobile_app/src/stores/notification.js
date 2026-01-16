import { defineStore } from 'pinia'
import { ref } from 'vue'
import { PushNotifications } from '@capacitor/push-notifications'
import { useAuthStore } from './auth'
import { Capacitor } from '@capacitor/core'

export const useNotificationStore = defineStore('notification', () => {
  const notifications = ref([])
  const isInitialized = ref(false)

  // Initialize push notifications
  const initPushNotifications = async () => {
    if (!Capacitor.isNativePlatform()) {
      console.log('Push notifications only available on native platforms')
      return
    }

    try {
      // Request permission
      let permStatus = await PushNotifications.checkPermissions()

      if (permStatus.receive === 'prompt') {
        permStatus = await PushNotifications.requestPermissions()
      }

      if (permStatus.receive !== 'granted') {
        console.error('User denied permissions!')
        return
      }

      // Register with FCM
      await PushNotifications.register()

      // Listen for registration
      await PushNotifications.addListener('registration', async (token) => {
        console.log('Push registration success, token: ' + token.value)
        
        // Update token in Firestore
        const authStore = useAuthStore()
        await authStore.updateFCMToken(token.value)
      })

      // Listen for registration errors
      await PushNotifications.addListener('registrationError', (error) => {
        console.error('Error on registration: ' + JSON.stringify(error))
      })

      // Listen for push notifications
      await PushNotifications.addListener('pushNotificationReceived', (notification) => {
        console.log('Push notification received: ', notification)
        
        notifications.value.unshift({
          id: Date.now(),
          title: notification.title,
          body: notification.body,
          data: notification.data,
          timestamp: new Date().toISOString()
        })
      })

      // Listen for notification taps
      await PushNotifications.addListener('pushNotificationActionPerformed', (notification) => {
        console.log('Push notification action performed', notification)
      })

      isInitialized.value = true
    } catch (error) {
      console.error('Error initializing push notifications:', error)
    }
  }

  // Clear all notifications
  const clearNotifications = () => {
    notifications.value = []
  }

  return {
    notifications,
    isInitialized,
    initPushNotifications,
    clearNotifications
  }
})
