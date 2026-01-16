<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Scan History</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding">
      <div class="space-y-4">
        <!-- Filter Buttons -->
        <div class="flex gap-2 overflow-x-auto pb-2">
          <button
            v-for="filter in filters"
            :key="filter.value"
            @click="selectedFilter = filter.value"
            :class="[
              'px-4 py-2 rounded-lg font-semibold text-sm whitespace-nowrap transition',
              selectedFilter === filter.value
                ? 'text-white'
                : 'bg-gray-200 text-gray-700'
            ]"
            :style="selectedFilter === filter.value ? 'background-color: #005145;' : ''"
          >
            {{ filter.label }}
          </button>
        </div>

        <!-- History List -->
        <div v-if="filteredHistory.length > 0" class="space-y-3">
          <div
            v-for="item in filteredHistory"
            :key="item.id"
            class="bg-white rounded-xl p-4 shadow"
          >
            <div class="flex items-start justify-between mb-2">
              <div class="flex items-center gap-3">
                <div class="rounded-full p-2" style="background-color: rgba(0, 81, 69, 0.1);">
                  <ion-icon :icon="checkmarkCircleOutline" class="text-2xl" style="color: #005145;"></ion-icon>
                </div>
                <div>
                  <h3 class="font-bold text-gray-800">{{ item.holderName }}</h3>
                  <p class="text-sm text-gray-600">{{ item.driverName }}</p>
                </div>
              </div>
              <span class="text-xs text-gray-400">
                {{ formatDate(item.timestamp) }}
              </span>
            </div>

            <div class="border-t border-gray-100 pt-3 mt-3 space-y-1 text-sm">
              <div class="flex justify-between">
                <span class="text-gray-600">Device ID:</span>
                <span class="font-medium">{{ item.deviceId }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-gray-600">Driver Name:</span>
                <span class="font-medium">{{ item.driverName || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-gray-600">Plate Number:</span>
                <span class="font-medium">{{ item.plateNumber || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-gray-600">Phone:</span>
                <span class="font-medium">{{ item.driverPhone || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-gray-600">Email:</span>
                <span class="font-medium">{{ item.driverEmail || 'N/A' }}</span>
              </div>
              <div class="flex justify-between">
                <span class="text-gray-600">Time:</span>
                <span class="font-medium">{{ formatTime(item.timestamp) }}</span>
              </div>
            </div>
          </div>
        </div>

        <!-- Empty State -->
        <div v-else class="text-center py-12">
          <ion-icon :icon="timeOutline" class="text-6xl text-gray-300 mb-4"></ion-icon>
          <p class="text-gray-500">No scan history found</p>
        </div>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonIcon } from '@ionic/vue'
import { checkmarkCircleOutline, timeOutline } from 'ionicons/icons'
import { useAuthStore } from '@/stores/auth'
import { collection, query, where, getDocs, orderBy } from 'firebase/firestore'
import { db } from '@/firebase'

const authStore = useAuthStore()

const history = ref([])
const selectedFilter = ref('all')

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

const filters = [
  { label: 'All', value: 'all' },
  { label: 'Today', value: 'today' },
  { label: 'This Week', value: 'week' },
  { label: 'This Month', value: 'month' }
]

const filteredHistory = computed(() => {
  const now = new Date()
  
  switch (selectedFilter.value) {
    case 'today': {
      const today = new Date(now.getFullYear(), now.getMonth(), now.getDate())
      return history.value.filter(item => new Date(item.timestamp) >= today)
    }
    case 'week': {
      const weekAgo = new Date(now.getTime() - 7 * 24 * 60 * 60 * 1000)
      return history.value.filter(item => new Date(item.timestamp) >= weekAgo)
    }
    case 'month': {
      const monthStart = new Date(now.getFullYear(), now.getMonth(), 1)
      return history.value.filter(item => new Date(item.timestamp) >= monthStart)
    }
    default:
      return history.value
  }
})

const formatDate = (timestamp) => {
  const date = new Date(timestamp)
  return date.toLocaleDateString()
}

const formatTime = (timestamp) => {
  const date = new Date(timestamp)
  return date.toLocaleTimeString()
}

const loadHistory = async () => {
  if (!authStore.user) return

  try {
    // Get user's RFID cards
    const cardsSnap = await getDocs(
      query(collection(db, 'rfidCards'), where('userId', '==', authStore.user.uid))
    )
    
    const rfidList = cardsSnap.docs.map(doc => doc.data().rfid)

    if (rfidList.length > 0) {
      // Get scans for these cards
      const scansSnap = await getDocs(
        query(
          collection(db, 'scans'),
          where('rfid', 'in', rfidList)
        )
      )

      // Convert Firestore timestamps and sort in memory
      history.value = scansSnap.docs
        .map(doc => ({
          id: doc.id,
          ...doc.data(),
          timestamp: doc.data().timestamp?.toDate ? doc.data().timestamp.toDate() : new Date(doc.data().timestamp)
        }))
        .sort((a, b) => b.timestamp - a.timestamp)
    }
  } catch (error) {
    console.error('Error loading history:', error)
  }
}

onMounted(() => {
  loadHistory()
})
</script>
