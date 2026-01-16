<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Scan Logs</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding">
      <div class="space-y-4">
        <!-- Filter Section -->
        <div class="bg-white rounded-xl p-4 shadow">
          <div class="flex gap-2">
            <select v-model="filterPeriod" @change="loadScans" class="flex-1 p-2 border rounded">
              <option value="today">Today</option>
              <option value="week">This Week</option>
              <option value="month">This Month</option>
              <option value="all">All Time</option>
            </select>
          </div>
        </div>

        <!-- Stats Summary -->
        <div class="bg-white rounded-xl p-4 shadow">
          <h3 class="text-lg font-bold text-gray-800 mb-2">Summary</h3>
          <div class="grid grid-cols-2 gap-4">
            <div>
              <p class="text-sm text-gray-600">Total Scans</p>
              <p class="text-2xl font-bold" style="color: #005145;">{{ scans.length }}</p>
            </div>
            <div>
              <p class="text-sm text-gray-600">Unique Cards</p>
              <p class="text-2xl font-bold" style="color: #005145;">{{ uniqueCards }}</p>
            </div>
          </div>
        </div>

        <!-- Scan Logs List -->
        <div class="bg-white rounded-xl p-4 shadow">
          <h3 class="text-lg font-bold text-gray-800 mb-4">Scan History</h3>
          
          <div v-if="scans.length > 0" class="space-y-3">
            <div v-for="scan in scans" :key="scan.id" class="scan-item" @click="viewDetails(scan)" style="cursor: pointer;">
              <div class="flex items-start gap-3">
                <div class="scan-icon">
                  <ion-icon :icon="cardOutline" />
                </div>
                <div class="flex-1">
                  <h4 class="font-semibold text-gray-800">{{ scan.holderName || 'Unknown' }}</h4>
                  <div class="text-sm text-gray-600 space-y-1">
                    <p><strong>RFID:</strong> {{ scan.rfid }}</p>
                    <p><strong>Device:</strong> {{ scan.deviceId || 'Unknown' }}</p>
                    <p v-if="scan.driverName"><strong>Driver:</strong> {{ scan.driverName }}</p>
                    <p v-if="scan.plateNumber"><strong>Plate:</strong> {{ scan.plateNumber }}</p>
                    <p class="text-xs text-gray-400">{{ formatTime(scan.timestamp) }}</p>
                  </div>
                </div>
                <ion-icon :icon="chevronForwardOutline" style="color: #9ca3af; font-size: 20px;" />
              </div>
            </div>
          </div>

          <div v-else class="text-center py-12">
            <ion-icon :icon="documentTextOutline" class="text-6xl text-gray-300 mb-4" />
            <p class="text-gray-500">No scan logs found</p>
          </div>
        </div>
      </div>
    </ion-content>

    <!-- Scan Details Modal -->
    <div v-if="showModal" class="custom-modal-overlay" @click="closeModal">
      <div class="custom-modal" @click.stop>
        <div class="modal-header">
          <h2 class="modal-title">Scan Details</h2>
          <button @click="closeModal" class="close-button">&times;</button>
        </div>
        
        <div class="modal-content" v-if="selectedScan">
          <div class="space-y-4">
            <div class="detail-row">
              <span class="detail-label">Holder Name:</span>
              <span class="detail-value">{{ selectedScan.holderName || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">RFID:</span>
              <span class="detail-value">{{ selectedScan.rfid }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Device ID:</span>
              <span class="detail-value">{{ selectedScan.deviceId || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Driver Name:</span>
              <span class="detail-value">{{ selectedScan.driverName || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Driver License:</span>
              <span class="detail-value">{{ selectedScan.driverLicenseNo || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Plate Number:</span>
              <span class="detail-value">{{ selectedScan.plateNumber || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Phone:</span>
              <span class="detail-value">{{ selectedScan.driverPhone || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Email:</span>
              <span class="detail-value">{{ selectedScan.driverEmail || 'N/A' }}</span>
            </div>
            <div class="detail-row">
              <span class="detail-label">Timestamp:</span>
              <span class="detail-value">{{ formatTime(selectedScan.timestamp) }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  </ion-page>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import {
  IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonIcon
} from '@ionic/vue'
import { cardOutline, documentTextOutline, chevronForwardOutline } from 'ionicons/icons'
import { collection, query, getDocs, limit } from 'firebase/firestore'
import { db } from '@/firebase'

const scans = ref([])
const filterPeriod = ref('all')
const showModal = ref(false)
const selectedScan = ref(null)

const uniqueCards = computed(() => {
  const rfids = new Set(scans.value.map(scan => scan.rfid))
  return rfids.size
})

const formatTime = (timestamp) => {
  if (!timestamp) return 'N/A'
  
  let date
  // Handle Firestore Timestamp object
  if (timestamp.toDate && typeof timestamp.toDate === 'function') {
    date = timestamp.toDate()
  } 
  // Handle timestamp in seconds (Unix epoch)
  else if (typeof timestamp === 'number' && timestamp < 946684800000) {
    date = new Date(timestamp * 1000)
  }
  // Handle ISO string or milliseconds
  else {
    date = new Date(timestamp)
  }
  
  if (isNaN(date.getTime())) {
    console.error('Invalid timestamp:', timestamp)
    return 'Invalid Date'
  }
  
  return date.toLocaleString('en-US', {
    month: 'short',
    day: 'numeric',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  })
}

const viewDetails = (scan) => {
  selectedScan.value = scan
  showModal.value = true
}

const closeModal = () => {
  showModal.value = false
  selectedScan.value = null
}

const loadScans = async () => {
  try {
    console.log('Loading scans...')
    // Load recent scans (limited to 100)
    const scansSnap = await getDocs(
      query(collection(db, 'scans'), limit(100))
    )
    
    console.log('Scans fetched:', scansSnap.size)
    
    let allScans = scansSnap.docs.map(doc => {
      const data = doc.data()
      console.log('Raw scan data:', data)
      console.log('Timestamp type:', typeof data.timestamp, 'Value:', data.timestamp)
      
      // Store the raw timestamp for display
      return {
        id: doc.id,
        ...data
      }
    })

    console.log('Total scans before filtering:', allScans.length)

    // Sort by timestamp
    allScans.sort((a, b) => {
      const getTime = (ts) => {
        if (!ts) return 0
        if (ts.toDate && typeof ts.toDate === 'function') return ts.toDate().getTime()
        if (typeof ts === 'number' && ts < 946684800000) return ts * 1000
        return new Date(ts).getTime()
      }
      return getTime(b.timestamp) - getTime(a.timestamp)
    })

    // Filter based on selected period
    const now = new Date()
    if (filterPeriod.value === 'today') {
      const today = new Date()
      today.setHours(0, 0, 0, 0)
      allScans = allScans.filter(scan => {
        if (!scan.timestamp) return false
        let scanDate
        if (scan.timestamp.toDate && typeof scan.timestamp.toDate === 'function') {
          scanDate = scan.timestamp.toDate()
        } else if (typeof scan.timestamp === 'number' && scan.timestamp < 946684800000) {
          scanDate = new Date(scan.timestamp * 1000)
        } else {
          scanDate = new Date(scan.timestamp)
        }
        return scanDate >= today
      })
    } else if (filterPeriod.value === 'week') {
      const weekAgo = new Date(now.getTime() - 7 * 24 * 60 * 60 * 1000)
      allScans = allScans.filter(scan => {
        if (!scan.timestamp) return false
        let scanDate
        if (scan.timestamp.toDate && typeof scan.timestamp.toDate === 'function') {
          scanDate = scan.timestamp.toDate()
        } else if (typeof scan.timestamp === 'number' && scan.timestamp < 946684800000) {
          scanDate = new Date(scan.timestamp * 1000)
        } else {
          scanDate = new Date(scan.timestamp)
        }
        return scanDate >= weekAgo
      })
    } else if (filterPeriod.value === 'month') {
      const monthAgo = new Date(now.getTime() - 30 * 24 * 60 * 60 * 1000)
      allScans = allScans.filter(scan => {
        if (!scan.timestamp) return false
        let scanDate
        if (scan.timestamp.toDate && typeof scan.timestamp.toDate === 'function') {
          scanDate = scan.timestamp.toDate()
        } else if (typeof scan.timestamp === 'number' && scan.timestamp < 946684800000) {
          scanDate = new Date(scan.timestamp * 1000)
        } else {
          scanDate = new Date(scan.timestamp)
        }
        return scanDate >= monthAgo
      })
    }

    console.log('Filtered scans:', allScans.length)
    scans.value = allScans
  } catch (error) {
    console.error('Error loading scans:', error)
  }
}

onMounted(() => {
  loadScans()
})
</script>

<style scoped>
.scan-item {
  padding: 16px;
  background: #f9fafb;
  border-radius: 8px;
  border: 1px solid #e5e7eb;
  transition: all 0.2s;
}

.scan-item:hover {
  background: #f3f4f6;
  border-color: #005145;
}

.scan-icon {
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background-color: #005145;
  color: white;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
}

.space-y-1 > * + * {
  margin-top: 4px;
}

.space-y-3 > * + * {
  margin-top: 12px;
}

.space-y-4 > * + * {
  margin-top: 16px;
}

.custom-modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 9999;
  padding: 20px;
}

.custom-modal {
  background: white;
  border-radius: 12px;
  width: 100%;
  max-width: 500px;
  max-height: 80vh;
  display: flex;
  flex-direction: column;
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
}

.modal-header {
  background-color: #005145;
  color: white;
  padding: 16px 20px;
  border-radius: 12px 12px 0 0;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.modal-title {
  font-size: 18px;
  font-weight: 600;
  margin: 0;
}

.close-button {
  background: none;
  border: none;
  color: white;
  font-size: 32px;
  line-height: 1;
  cursor: pointer;
  padding: 0;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.modal-content {
  padding: 20px;
  overflow-y: auto;
}

.detail-row {
  display: flex;
  justify-content: space-between;
  padding: 12px;
  background: #f9fafb;
  border-radius: 6px;
  border: 1px solid #e5e7eb;
}

.detail-label {
  font-weight: 600;
  color: #005145;
}

.detail-value {
  color: #374151;
}
</style>
