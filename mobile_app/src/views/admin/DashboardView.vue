<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Admin Dashboard</ion-title>
        <ion-buttons slot="end">
          <ion-button @click="logout"  style="--color: #ffffff" >
            <ion-icon :icon="logOutOutline"></ion-icon>
          </ion-button>
        </ion-buttons>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding">
      <div class="space-y-6">
        <!-- Welcome Card -->
        <div class="rounded-2xl p-6 text-white" style="background-color: #005145;">
          <h2 class="text-2xl font-bold mb-2">Welcome, {{ authStore.userProfile?.name }}!</h2>
          <p class="opacity-90">Admin Dashboard</p>
        </div>

        <!-- Stats Grid -->
        <div class="grid grid-cols-2 gap-4">
          <div class="bg-white rounded-xl p-4 shadow">
            <div class="flex items-center justify-between mb-2">
              <ion-icon :icon="hardwareChipOutline" class="text-3xl" style="color: #005145;"></ion-icon>
            </div>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.devices }}</h3>
            <p class="text-sm text-gray-600">Devices</p>
          </div>

          <div class="bg-white rounded-xl p-4 shadow">
            <div class="flex items-center justify-between mb-2">
              <ion-icon :icon="peopleOutline" class="text-3xl" style="color: #005145;"></ion-icon>
            </div>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.users }}</h3>
            <p class="text-sm text-gray-600">Users</p>
          </div>

          <div class="bg-white rounded-xl p-4 shadow">
            <div class="flex items-center justify-between mb-2">
              <ion-icon :icon="cardOutline" class="text-3xl" style="color: #005145;"></ion-icon>
            </div>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.rfidCards }}</h3>
            <p class="text-sm text-gray-600">RFID Cards</p>
          </div>

          <div class="bg-white rounded-xl p-4 shadow">
            <div class="flex items-center justify-between mb-2">
              <ion-icon :icon="timeOutline" class="text-3xl" style="color: #005145;"></ion-icon>
            </div>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.todayScans }}</h3>
            <p class="text-sm text-gray-600">Today's Scans</p>
          </div>
        </div>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { 
  IonPage, IonHeader, IonToolbar, IonTitle, IonContent, 
  IonButtons, IonButton, IonIcon
} from '@ionic/vue'
import { 
  logOutOutline, hardwareChipOutline, peopleOutline, 
  cardOutline, timeOutline
} from 'ionicons/icons'
import { useAuthStore } from '@/stores/auth'
import { collection, query, getDocs, where, orderBy, limit } from 'firebase/firestore'
import { db } from '@/firebase'

const authStore = useAuthStore()

const stats = ref({
  devices: 0,
  users: 0,
  rfidCards: 0,
  todayScans: 0
})

const logout = () => {
  authStore.logout()
}

const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  const now = new Date()
  const diff = now - date
  const minutes = Math.floor(diff / 60000)
  
  if (minutes < 1) return 'Just now'
  if (minutes < 60) return `${minutes}m ago`
  if (minutes < 1440) return `${Math.floor(minutes / 60)}h ago`
  return date.toLocaleDateString()
}

const loadStats = async () => {
  try {
    // Load devices count
    const devicesSnap = await getDocs(collection(db, 'devices'))
    stats.value.devices = devicesSnap.size

    // Load users count
    const usersSnap = await getDocs(query(collection(db, 'users'), where('role', '==', 'user')))
    stats.value.users = usersSnap.size

    // Load RFID cards count
    const rfidSnap = await getDocs(collection(db, 'rfidCards'))
    stats.value.rfidCards = rfidSnap.size

    // Load all scans (limited to recent 100 to avoid excessive reads)
    const scansSnap = await getDocs(
      query(
        collection(db, 'scans'),
        limit(100)
      )
    )
    
    const allScans = scansSnap.docs.map(doc => {
      const data = doc.data()
      return {
        id: doc.id,
        ...data
      }
    })

    console.log('Total scans loaded:', allScans.length)

    // Filter today's scans in memory
    const today = new Date()
    today.setHours(0, 0, 0, 0)
    const todayScans = allScans.filter(scan => {
      if (!scan.timestamp) return false
      
      let scanDate
      // Handle Firestore Timestamp object
      if (scan.timestamp.toDate && typeof scan.timestamp.toDate === 'function') {
        scanDate = scan.timestamp.toDate()
      }
      // Handle timestamp in seconds
      else if (typeof scan.timestamp === 'number' && scan.timestamp < 946684800000) {
        scanDate = new Date(scan.timestamp * 1000)
      }
      // Handle ISO string or milliseconds
      else {
        scanDate = new Date(scan.timestamp)
      }
      
      if (isNaN(scanDate.getTime())) return false
      return scanDate >= today
    })
    console.log('Today scans count:', todayScans.length)
    stats.value.todayScans = todayScans.length
  } catch (error) {
    console.error('Error loading stats:', error)
  }
}

onMounted(() => {
  loadStats()
})
</script>
