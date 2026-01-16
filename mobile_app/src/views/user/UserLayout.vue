<template>
  <ion-page>
    <ion-content>
      <router-view />
    </ion-content>
    
    <!-- Custom tab bar -->
    <div class="custom-tab-bar">
      <button 
        @click="navigate('/user/dashboard')"
        :class="['tab-button', { active: isActive('/user/dashboard') }]"
      >
        <ion-icon :icon="homeOutline" />
        <span class="tab-label">Home</span>
      </button>
      
      <button 
        @click="navigate('/user/register-rfid')"
        :class="['tab-button', { active: isActive('/user/register-rfid') }]"
      >
        <ion-icon :icon="addCircleOutline" />
        <span class="tab-label">Register RFID</span>
      </button>
      
      <button 
        @click="navigate('/user/notifications')"
        :class="['tab-button', { active: isActive('/user/notifications') }]"
      >
        <div class="notification-badge-wrapper">
          <ion-icon :icon="notificationsOutline" />
          <span v-if="unreadCount > 0" class="notification-badge">{{ unreadCount }}</span>
        </div>
        <span class="tab-label">Notifications</span>
      </button>
      
      <button 
        @click="navigate('/user/history')"
        :class="['tab-button', { active: isActive('/user/history') }]"
      >
        <ion-icon :icon="timeOutline" />
        <span class="tab-label">History</span>
      </button>
    </div>
  </ion-page>
</template>

<script setup>
import { IonPage, IonContent, IonIcon } from '@ionic/vue'
import { homeOutline, addCircleOutline, timeOutline, notificationsOutline } from 'ionicons/icons'
import { useRouter, useRoute } from 'vue-router'
import { ref, onMounted } from 'vue'
import { collection, query, where, onSnapshot } from 'firebase/firestore'
import { db, auth } from '@/firebase'

const router = useRouter()
const route = useRoute()
const unreadCount = ref(0)

const navigate = (path) => {
  if (route.path !== path) {
    router.push(path)
  }
}

const isActive = (path) => route.path === path

// Listen for unread notifications
onMounted(() => {
  const user = auth.currentUser
  if (user) {
    const notificationsRef = collection(db, 'notifications')
    const q = query(
      notificationsRef,
      where('userId', '==', user.uid),
      where('read', '==', false)
    )
    
    onSnapshot(q, (snapshot) => {
      unreadCount.value = snapshot.size
    })
  }
})
</script>

<style scoped>
.custom-tab-bar {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  height: 70px;
  background-color: white;
  border-top: 1px solid #e0e0e0;
  display: flex;
  justify-content: space-around;
  align-items: center;
  padding-bottom: max(env(safe-area-inset-bottom), 0px);
  z-index: 1000;
}

.tab-button {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 4px;
  padding: 8px;
  background: none;
  border: none;
  cursor: pointer;
  color: #6b7280;
  transition: all 0.2s;
}

.tab-button:active {
  transform: scale(0.95);
}

.tab-button.active {
  color: #005145;
}

.tab-button ion-icon {
  font-size: 24px;
}

.tab-label {
  font-size: 12px;
  font-weight: 500;
}

.notification-badge-wrapper {
  position: relative;
  display: inline-block;
}

.notification-badge {
  position: absolute;
  top: -8px;
  right: -8px;
  background-color: #ef4444;
  color: white;
  border-radius: 10px;
  padding: 2px 6px;
  font-size: 10px;
  font-weight: bold;
  min-width: 18px;
  text-align: center;
}

ion-content {
  --padding-bottom: 70px;
}
</style>
