<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Device Management</ion-title>
        <ion-buttons slot="end">
          <ion-button @click="openAddDevice" style="--color: #ffffff">
            <ion-icon :icon="addOutline"></ion-icon>
          </ion-button>
        </ion-buttons>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding" style="--background: #f5f5f5;">
      <div class="space-y-4" style="padding-bottom: 80px;">
        <!-- Device List -->
        <div v-for="device in devices" :key="device.id" class="bg-white rounded-2xl shadow-md overflow-hidden mb-4">
          <div class="p-5">
            <div class="flex items-center justify-between mb-3">
              <div>
                <h3 class="text-lg font-bold" style="color: #005145;">{{ device.deviceId }}</h3>
                <p class="text-sm text-gray-600">{{ device.driverName }}</p>
              </div>
              <ion-badge :color="device.active ? 'success' : 'danger'" style="font-size: 12px; padding: 6px 12px;">
                {{ device.active ? 'Active' : 'Inactive' }}
              </ion-badge>
            </div>
            
            <div class="space-y-2 text-sm mb-4" style="color: #666;">
              <div class="flex items-center gap-2">
                <ion-icon :icon="personOutline" style="color: #005145;"></ion-icon>
                <span><strong>Age:</strong> {{ calculateAge(device.driverBirthdate) }} years</span>
              </div>
              <div class="flex items-center gap-2">
                <ion-icon :icon="cardOutline" style="color: #005145;"></ion-icon>
                <span><strong>License:</strong> {{ device.driverLicenseNo || 'N/A' }}</span>
              </div>
              <div class="flex items-center gap-2">
                <ion-icon :icon="carOutline" style="color: #005145;"></ion-icon>
                <span><strong>Plate:</strong> {{ device.plateNumber }}</span>
              </div>
              <div class="flex items-center gap-2">
                <ion-icon :icon="callOutline" style="color: #005145;"></ion-icon>
                <span><strong>Phone:</strong> {{ device.driverPhone || 'N/A' }}</span>
              </div>
              <div class="flex items-center gap-2">
                <ion-icon :icon="mailOutline" style="color: #005145;"></ion-icon>
                <span><strong>Email:</strong> {{ device.driverEmail || 'N/A' }}</span>
              </div>
              <div class="flex items-center gap-2">
                <ion-icon :icon="calendarOutline" style="color: #005145;"></ion-icon>
                <span><strong>Birthdate:</strong> {{ formatDate(device.driverBirthdate) }}</span>
              </div>
            </div>
            
            <div class="flex gap-2">
              <button
                @click="editDevice(device)"
                class="flex-1 py-2 px-4 rounded-xl font-semibold transition"
                style="background-color: #ffffff; color: #005145; border: 2px solid #005145;"
              >
                <ion-icon :icon="createOutline" class="align-middle"></ion-icon>
                Edit
              </button>
              <button
                @click="deleteDevice(device.id)"
                class="py-2 px-4 rounded-xl font-semibold transition"
                style="background-color: #ff4444; color: white;"
              >
                <ion-icon :icon="trashOutline" class="align-middle"></ion-icon>
              </button>
            </div>
          </div>
        </div>

        <!-- Empty State -->
        <div v-if="devices.length === 0" class="text-center py-16">
          <ion-icon :icon="hardwareChipOutline" class="text-7xl mb-4" style="color: #d0d0d0;"></ion-icon>
          <p class="text-lg mb-2" style="color: #005145; font-weight: 600;">No devices registered yet</p>
          <p class="text-sm" style="color: #999;">Tap the + button to add your first device</p>
        </div>
      </div>

      <!-- Floating Action Button -->
      <ion-fab vertical="bottom" horizontal="end" slot="fixed">
        <ion-fab-button @click="openAddDevice" style="--background: #ffffff; --color: #005145;">
          <ion-icon :icon="addOutline"></ion-icon>
        </ion-fab-button>
      </ion-fab>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import {
  IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonButtons, IonButton,
  IonIcon, IonBadge, IonToggle, IonFab, IonFabButton, alertController, modalController
} from '@ionic/vue'
import { 
  addOutline, createOutline, trashOutline, hardwareChipOutline, 
  closeOutline, personOutline, carOutline, calendarOutline,
  cardOutline, callOutline, mailOutline
} from 'ionicons/icons'
import { collection, getDocs, addDoc, updateDoc, deleteDoc, doc } from 'firebase/firestore'
import { db } from '@/firebase'
import DeviceFormModal from '@/components/DeviceFormModal.vue'

const devices = ref([])
const editingDevice = ref(null)
const saving = ref(false)

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

const formatDate = (dateString) => {
  if (!dateString) return 'N/A'
  const date = new Date(dateString)
  return date.toLocaleDateString()
}

const formData = ref({
  deviceId: '',
  driverName: '',
  driverBirthdate: '',
  plateNumber: '',
  active: true
})

const loadDevices = async () => {
  try {
    const querySnapshot = await getDocs(collection(db, 'devices'))
    devices.value = querySnapshot.docs.map(doc => ({
      id: doc.id,
      ...doc.data()
    }))
  } catch (error) {
    console.error('Error loading devices:', error)
  }
}

const openAddDevice = async () => {
  const modal = await modalController.create({
    component: DeviceFormModal,
    componentProps: {
      device: null
    }
  })
  
  await modal.present()
  
  const { data, role } = await modal.onWillDismiss()
  if (role === 'confirm' || data) {
    await saveDevice(data.data, data.deviceId)
  }
}

const editDevice = async (device) => {
  const modal = await modalController.create({
    component: DeviceFormModal,
    componentProps: {
      device: device
    }
  })
  
  await modal.present()
  
  const { data, role } = await modal.onWillDismiss()
  if (role === 'confirm' || data) {
    await saveDevice(data.data, data.deviceId)
  }
}

const saveDevice = async (deviceData, deviceId) => {
  try {
    if (deviceId) {
      // Update existing device
      await updateDoc(doc(db, 'devices', deviceId), deviceData)
    } else {
      // Add new device
      await addDoc(collection(db, 'devices'), {
        ...deviceData,
        createdAt: new Date().toISOString()
      })
    }
    await loadDevices()
  } catch (error) {
    console.error('Error saving device:', error)
  }
}

const deleteDevice = async (deviceId) => {
  const alert = await alertController.create({
    header: 'Confirm Delete',
    message: 'Are you sure you want to delete this device?',
    buttons: [
      { text: 'Cancel', role: 'cancel' },
      {
        text: 'Delete',
        role: 'destructive',
        handler: async () => {
          try {
            await deleteDoc(doc(db, 'devices', deviceId))
            await loadDevices()
          } catch (error) {
            console.error('Error deleting device:', error)
          }
        }
      }
    ]
  })
  await alert.present()
}

onMounted(() => {
  loadDevices()
})
</script>
