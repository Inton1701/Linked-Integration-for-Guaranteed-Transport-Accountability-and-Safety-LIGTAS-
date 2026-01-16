<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>{{ device ? 'Edit Device' : 'Add Device' }}</ion-title>
        <ion-buttons slot="end">
          <ion-button @click="dismiss" style="--color: #ffffff">
            <ion-icon :icon="closeOutline"></ion-icon>
          </ion-button>
        </ion-buttons>
      </ion-toolbar>
    </ion-header>
    <ion-content class="ion-padding" style="--background: #f5f5f5;">
      <form @submit.prevent="handleSubmit" class="space-y-4 max-w-lg mx-auto">
        <div class="bg-white rounded-2xl p-5 shadow-md">
          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Device ID</label>
            <input
              v-model="formData.deviceId"
              required
              placeholder="DVC0001"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Driver Name</label>
            <input
              v-model="formData.driverName"
              required
              placeholder="John Doe"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Driver Birthdate</label>
            <input
              v-model="formData.driverBirthdate"
              type="date"
              required
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Plate Number</label>
            <input
              v-model="formData.plateNumber"
              required
              placeholder="ABC 1234"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Driver License Number</label>
            <input
              v-model="formData.driverLicenseNo"
              required
              placeholder="DL123456"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Phone Number</label>
            <input
              v-model="formData.driverPhone"
              required
              type="tel"
              placeholder="+1234567890"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="mb-4">
            <label class="block text-sm font-semibold mb-2" style="color: #005145;">Email Address</label>
            <input
              v-model="formData.driverEmail"
              required
              type="email"
              placeholder="driver@example.com"
              class="w-full px-4 py-3 border-2 rounded-xl focus:outline-none transition"
              style="border-color: #e0e0e0;"
            />
          </div>

          <div class="flex items-center justify-between p-4 rounded-xl mb-4" style="background-color: #f8f8f8;">
            <label class="font-semibold" style="color: #005145;">Active Status</label>
            <ion-toggle v-model="formData.active" style="--background: #d0d0d0; --background-checked: #005145; --handle-background-checked: #ffffff;"></ion-toggle>
          </div>

          <button
            type="submit"
            :disabled="saving"
            class="w-full font-bold py-4 rounded-xl transition disabled:opacity-50"
            style="background-color: #005145; color: #ffffff;"
          >
            {{ saving ? 'Saving...' : 'Save Device' }}
          </button>
        </div>
      </form>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, defineProps } from 'vue'
import { IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonButtons, IonButton, IonIcon, IonToggle, modalController } from '@ionic/vue'
import { closeOutline } from 'ionicons/icons'

const props = defineProps({
  device: {
    type: Object,
    default: null
  }
})

const saving = ref(false)
const formData = ref({
  deviceId: props.device?.deviceId || '',
  driverName: props.device?.driverName || '',
  driverBirthdate: props.device?.driverBirthdate || '',
  plateNumber: props.device?.plateNumber || '',
  driverLicenseNo: props.device?.driverLicenseNo || '',
  driverPhone: props.device?.driverPhone || '',
  driverEmail: props.device?.driverEmail || '',
  active: props.device?.active ?? true
})

const dismiss = () => {
  modalController.dismiss()
}

const handleSubmit = async () => {
  saving.value = true
  try {
    await modalController.dismiss({
      data: formData.value,
      deviceId: props.device?.id
    })
  } catch (error) {
    console.error('Error submitting form:', error)
    saving.value = false
  }
}
</script>
