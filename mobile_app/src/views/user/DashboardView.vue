<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Dashboard</ion-title>
        <ion-buttons slot="end">
          <ion-button @click="logout" style="--color:#fff">
            <ion-icon :icon="logOutOutline"></ion-icon>
          </ion-button>
        </ion-buttons>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding">
      <div class="space-y-6">
        <!-- Welcome Card -->
        <div class="rounded-2xl p-6 text-white" style="background-color: #005145;">
          <h2 class="text-2xl font-bold mb-2">Hello, {{ authStore.userProfile?.name }}!</h2>
          <p class="opacity-90">Track your passengers safely</p>
        </div>

        <!-- Quick Stats -->
        <div class="grid grid-cols-2 gap-4">
          <div class="bg-white rounded-xl p-4 shadow">
            <ion-icon :icon="cardOutline" class="text-3xl mb-2" style="color: #005145;"></ion-icon>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.rfidCards }}</h3>
            <p class="text-sm text-gray-600">RFID Cards</p>
          </div>

          <div class="bg-white rounded-xl p-4 shadow">
            <ion-icon :icon="notificationsOutline" class="text-3xl mb-2" style="color: #005145;"></ion-icon>
            <h3 class="text-2xl font-bold text-gray-800">{{ stats.todayScans }}</h3>
            <p class="text-sm text-gray-600">Today's Scans</p>
          </div>
        </div>

        <!-- Registered RFID Cards -->
        <div class="bg-white rounded-xl p-4 shadow">
          <div class="flex justify-between items-center mb-4">
            <h3 class="text-lg font-bold text-gray-800">My RFID Cards</h3>
            <ion-button size="small" @click="$router.push('/user/register-rfid')">
              <ion-icon :icon="addOutline" slot="start"></ion-icon>
              Add
            </ion-button>
          </div>

          <ion-list v-if="rfidCards.length > 0" class="bg-transparent">
            <ion-item v-for="card in rfidCards" :key="card.id" lines="full">
              <ion-icon :icon="personOutline" slot="start" style="color: #005145;"></ion-icon>
              <ion-label>
                <h3 class="font-semibold">{{ card.holderName }}</h3>
                <p class="text-xs text-gray-600">{{ card.rfid }}</p>
              </ion-label>
              <ion-button slot="end" fill="clear" color="danger" @click="deleteCard(card.id)">
                <ion-icon :icon="trashOutline"></ion-icon>
              </ion-button>
            </ion-item>
          </ion-list>

          <div v-else class="text-center py-8">
            <ion-icon :icon="cardOutline" class="text-5xl text-gray-300 mb-3"></ion-icon>
            <p class="text-gray-500 mb-4">No RFID cards registered</p>
            <ion-button @click="$router.push('/user/register-rfid')">
              Register Your First Card
            </ion-button>
          </div>
        </div>

        <!-- Recent Notifications -->
        <div class="bg-white rounded-xl p-4 shadow">
          <h3 class="text-lg font-bold text-gray-800 mb-4">Recent Notifications</h3>
          <ion-list v-if="recentNotifications.length > 0" class="bg-transparent">
            <ion-item v-for="notif in recentNotifications" :key="notif.id" lines="full">
              <ion-icon :icon="checkmarkCircleOutline" slot="start" style="color: #005145;"></ion-icon>
              <ion-label>
                <h3 class="font-semibold">{{ notif.holderName }} Arrived</h3>
                <p class="text-sm text-gray-600">Driver: {{ notif.driverName }}</p>
                <p class="text-xs text-gray-400">{{ formatTime(notif.timestamp) }}</p>
              </ion-label>
            </ion-item>
          </ion-list>

          <div v-else class="text-center py-8">
            <p class="text-gray-500">No notifications yet</p>
          </div>
        </div>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import {
  IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonButtons, IonButton,
  IonIcon, IonList, IonItem, IonLabel, alertController
} from '@ionic/vue'
import {
  logOutOutline, cardOutline, notificationsOutline, addOutline,
  personOutline, trashOutline, checkmarkCircleOutline
} from 'ionicons/icons'
import { useAuthStore } from '@/stores/auth'
import { collection, query, where, getDocs, deleteDoc, doc, orderBy, limit } from 'firebase/firestore'
import { db } from '@/firebase'

const authStore = useAuthStore()

const stats = ref({
  rfidCards: 0,
  todayScans: 0
})

const rfidCards = ref([])
const recentNotifications = ref([])

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

const loadData = async () => {
  if (!authStore.user) return

  try {
    // Load RFID cards
    const cardsSnap = await getDocs(
      query(collection(db, 'rfidCards'), where('userId', '==', authStore.user.uid))
    )
    rfidCards.value = cardsSnap.docs.map(doc => ({
      id: doc.id,
      ...doc.data()
    }))
    stats.value.rfidCards = rfidCards.value.length

    // Load today's scans for user's cards
    const today = new Date()
    today.setHours(0, 0, 0, 0)

    const rfidList = rfidCards.value.map(card => card.rfid)
    if (rfidList.length > 0) {
      // Get all scans for user's cards (limited to recent 100)
      const allScansSnap = await getDocs(
        query(
          collection(db, 'scans'),
          where('rfid', 'in', rfidList),
          limit(100)
        )
      )
      
      const allScans = allScansSnap.docs.map(doc => ({
        id: doc.id,
        ...doc.data(),
        timestamp: doc.data().timestamp?.toDate ? doc.data().timestamp.toDate() : new Date(doc.data().timestamp)
      }))
      
      // Filter today's scans in memory
      const todayScans = allScans.filter(scan => scan.timestamp >= today)
      stats.value.todayScans = todayScans.length

      // Sort by timestamp and get recent 5
      const sortedScans = allScans.sort((a, b) => b.timestamp - a.timestamp)
      recentNotifications.value = sortedScans.slice(0, 5)
    }
  } catch (error) {
    console.error('Error loading data:', error)
  }
}

const deleteCard = async (cardId) => {
  const alert = await alertController.create({
    header: 'Confirm Delete',
    message: 'Are you sure you want to delete this RFID card?',
    buttons: [
      { text: 'Cancel', role: 'cancel' },
      {
        text: 'Delete',
        role: 'destructive',
        handler: async () => {
          try {
            await deleteDoc(doc(db, 'rfidCards', cardId))
            await loadData()
          } catch (error) {
            console.error('Error deleting card:', error)
          }
        }
      }
    ]
  })
  await alert.present()
}

onMounted(() => {
  loadData()
})
</script>
