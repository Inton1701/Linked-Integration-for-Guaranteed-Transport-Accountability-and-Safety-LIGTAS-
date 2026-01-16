<template>
  <ion-page>
    <ion-header>
      <ion-toolbar color="primary">
        <ion-title>Notifications</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <div class="notifications-container">
        <!-- Enable notifications prompt -->
        <div v-if="!fcmTokenSaved" class="notification-prompt">
          <ion-icon :icon="notificationsOutline" class="prompt-icon" />
          <h3>Enable Push Notifications</h3>
          <p>Get instant alerts when your RFID card is scanned</p>
          <ion-button @click="enableNotifications" color="primary">
            Enable Notifications
          </ion-button>
        </div>
        
        <!-- Empty state -->
        <div v-if="notifications.length === 0" class="empty-state">
          <ion-icon :icon="notificationsOffOutline" class="empty-icon" />
          <h2>No Notifications</h2>
          <p>You'll see notifications here when your RFID card is scanned</p>
        </div>

        <!-- Notifications list -->
        <div v-else class="notifications-list">
          <div
            v-for="notification in notifications"
            :key="notification.id"
            :class="['notification-item', { unread: !notification.read }]"
            @click="goToHistory(notification.id)"
          >
            <div class="notification-icon">
              <ion-icon :icon="cardOutline" />
            </div>
            
            <div class="notification-content">
              <h3>{{ notification.title }}</h3>
              <p>{{ notification.holderName || 'Unknown' }} - {{ notification.deviceId || 'Unknown Device' }}</p>
              <span class="notification-time">{{ formatTime(notification.createdAt) }}</span>
            </div>

            <div v-if="!notification.read" class="unread-indicator" />
            <div class="chevron-icon">
              <ion-icon :icon="chevronForwardOutline" />
            </div>
          </div>
        </div>

        <!-- Mark all as read button -->
        <ion-button
          v-if="unreadCount > 0"
          expand="block"
          fill="clear"
          @click="markAllAsRead"
          class="mark-all-button"
        >
          Mark all as read
        </ion-button>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import {
  IonPage,
  IonHeader,
  IonToolbar,
  IonTitle,
  IonContent,
  IonIcon,
  IonButton
} from '@ionic/vue'
import { notificationsOffOutline, cardOutline, notificationsOutline, chevronForwardOutline } from 'ionicons/icons'
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import {
  collection,
  query,
  where,
  orderBy,
  onSnapshot,
  doc,
  updateDoc,
  writeBatch,
  getDoc
} from 'firebase/firestore'
import { db, auth } from '@/firebase'
import { useNotifications } from '@/composables/useNotifications'

const router = useRouter()
const notifications = ref([])
const fcmTokenSaved = ref(false)
const { requestPermission } = useNotifications()

const unreadCount = computed(() => 
  notifications.value.filter(n => !n.read).length
)

// Check if FCM token is saved
const checkFCMToken = async () => {
  const user = auth.currentUser
  if (!user) return
  
  const userDoc = await getDoc(doc(db, 'users', user.uid))
  if (userDoc.exists()) {
    fcmTokenSaved.value = !!userDoc.data().fcmToken
    console.log('FCM Token saved:', fcmTokenSaved.value)
  }
}

// Enable push notifications
const enableNotifications = async () => {
  const permission = await requestPermission()
  if (permission === 'granted') {
    await checkFCMToken()
  }
}

// Load notifications
onMounted(async () => {
  await checkFCMToken()
  
  const user = auth.currentUser
  if (!user) {
    console.log('No user logged in')
    return
  }

  console.log('Loading notifications for user:', user.uid)

  const notificationsRef = collection(db, 'notifications')
  const q = query(
    notificationsRef,
    where('userId', '==', user.uid)
  )

  onSnapshot(q, (snapshot) => {
    console.log('Notifications snapshot:', snapshot.size, 'notifications found')
    
    // Get notifications and sort by createdAt in memory
    notifications.value = snapshot.docs
      .map(doc => {
        console.log('Notification:', doc.id, doc.data())
        return {
          id: doc.id,
          ...doc.data(),
          createdAt: doc.data().createdAt?.toDate ? doc.data().createdAt.toDate() : new Date(doc.data().createdAt || Date.now())
        }
      })
      .sort((a, b) => b.createdAt - a.createdAt)
    
    console.log('Total notifications loaded:', notifications.value.length)
  }, (error) => {
    console.error('Error loading notifications:', error)
  })
})

const goToHistory = async (notificationId) => {
  await markAsRead(notificationId)
  router.push('/user/history')
}

// Mark notification as read
// Mark notification as read
const markAsRead = async (notificationId) => {
  const notifRef = doc(db, 'notifications', notificationId)
  await updateDoc(notifRef, { read: true })
}

// Mark all as read
const markAllAsRead = async () => {
  const batch = writeBatch(db)
  const unreadNotifications = notifications.value.filter(n => !n.read)

  unreadNotifications.forEach(notification => {
    const notifRef = doc(db, 'notifications', notification.id)
    batch.update(notifRef, { read: true })
  })

  await batch.commit()
}

// Format timestamp
const formatTime = (timestamp) => {
  if (!timestamp) return ''
  
  const date = timestamp.toDate ? timestamp.toDate() : new Date(timestamp)
  const now = new Date()
  const diff = now - date

  // Less than 1 minute
  if (diff < 60000) return 'Just now'
  
  // Less than 1 hour
  if (diff < 3600000) {
    const minutes = Math.floor(diff / 60000)
    return `${minutes}m ago`
  }
  
  // Less than 24 hours
  if (diff < 86400000) {
    const hours = Math.floor(diff / 3600000)
    return `${hours}h ago`
  }
  
  // Less than 7 days
  if (diff < 604800000) {
    const days = Math.floor(diff / 86400000)
    return `${days}d ago`
  }
  
  // Format as date
  return date.toLocaleDateString('en-US', {
    month: 'short',
    day: 'numeric',
    year: date.getFullYear() !== now.getFullYear() ? 'numeric' : undefined
  })
}
</script>

<style scoped>
.notifications-container {
  padding: 16px;
  max-width: 800px;
  margin: 0 auto;
}

.notification-prompt {
  background: var(--ion-color-primary);
  color: white;
  padding: 24px;
  border-radius: 12px;
  text-align: center;
  margin-bottom: 20px;
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
}

.prompt-icon {
  font-size: 48px;
  margin-bottom: 12px;
}

.notification-prompt h3 {
  font-size: 18px;
  font-weight: 600;
  margin-bottom: 8px;
}

.notification-prompt p {
  font-size: 14px;
  opacity: 0.9;
  margin-bottom: 16px;
}

.notification-prompt ion-button {
  --background: white;
  --color: #667eea;
  font-weight: 600;
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 20px;
  text-align: center;
  color: #6b7280;
}

.empty-icon {
  font-size: 80px;
  color: #d1d5db;
  margin-bottom: 20px;
}

.empty-state h2 {
  font-size: 20px;
  font-weight: 600;
  margin-bottom: 8px;
  color: #374151;
}

.empty-state p {
  font-size: 14px;
  color: #9ca3af;
}

.notifications-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.notification-item {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 16px;
  background-color: white;
  border-radius: 12px;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
  cursor: pointer;
  transition: all 0.2s;
  position: relative;
}

.notification-item:hover {
  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.15);
  transform: translateY(-2px);
}

.notification-item.unread {
  background-color: #f0fdf4;
  border-left: 4px solid #10b981;
}

.notification-icon {
  flex-shrink: 0;
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: #dcfce7;
  border-radius: 50%;
  color: #10b981;
}

.notification-icon ion-icon {
  font-size: 24px;
}

.notification-content {
  flex: 1;
}

.notification-content h3 {
  font-size: 16px;
  font-weight: 600;
  color: #111827;
  margin-bottom: 4px;
}

.notification-content p {
  font-size: 14px;
  color: #6b7280;
  margin-bottom: 8px;
  line-height: 1.5;
}

.notification-time {
  font-size: 12px;
  color: #6b7280;
}

.chevron-icon {
  flex-shrink: 0;
  color: #9ca3af;
  font-size: 20px;
  margin-left: 8px;
  right: 16px;
  transform: translateY(-50%);
  width: 8px;
  height: 8px;
  background-color: #10b981;
  border-radius: 50%;
}

.mark-all-button {
  margin-top: 16px;
}
</style>
