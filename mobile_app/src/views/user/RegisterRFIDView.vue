<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>Register RFID Card</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding" style="--padding-bottom: 80px;">
      <div class="space-y-6">
        <!-- Instructions Card -->
        <div class="border-2 rounded-xl p-4" style="background-color: rgba(0, 81, 69, 0.05); border-color: rgba(0, 81, 69, 0.2);">
          <h3 class="font-bold mb-2" style="color: #005145;">📱 How to Register</h3>
          <ol class="text-sm space-y-1 list-decimal list-inside" style="color: #005145;">
            <li>Enter the holder's name below</li>
            <li>Tap "Scan NFC/RFID Card" button</li>
            <li>Hold your RFID card near the phone</li>
            <li>Wait for the scan to complete</li>
          </ol>
        </div>

        <!-- Registration Form -->
        <div class="bg-white rounded-xl p-6 shadow">
          <form @submit.prevent="startNFCScan" class="space-y-4">
            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">
                Cardholder Name
              </label>
              <input
                v-model="holderName"
                type="text"
                required
                placeholder="e.g., John Doe"
                class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none transition"
                style="focus:border-color: #005145;"
              />
              <p class="text-xs text-gray-500 mt-1">
                Enter the name of the person who will use this card
              </p>
            </div>

            <!-- Manual RFID Input -->
            <div>
              <label class="block text-sm font-semibold text-gray-700 mb-2">
                RFID Card Number (Manual Entry)
              </label>
              <input
                v-model="manualRFID"
                type="text"
                placeholder="Enter RFID number or scan below"
                class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none transition"
                style="focus:border-color: #005145;"
                @input="handleManualInput"
              />
              <p class="text-xs text-gray-500 mt-1">
                You can type the RFID number manually or use the scan button below
              </p>
            </div>

            <!-- Scanned RFID Display -->
            <div v-if="scannedRFID" class="border-2 rounded-xl p-4" style="background-color: rgba(0, 81, 69, 0.05); border-color: rgba(0, 81, 69, 0.2);">
              <p class="text-sm font-semibold mb-1" style="color: #005145;">✓ Card Scanned Successfully</p>
              <p class="text-xs font-mono" style="color: #005145;">{{ scannedRFID }}</p>
            </div>

            <!-- Error Message -->
            <div v-if="error" class="bg-red-50 border-2 border-red-200 rounded-xl p-4">
              <p class="text-sm font-semibold text-red-800">{{ error }}</p>
            </div>

            <!-- Scan Button -->
            <button
              v-if="!scannedRFID && nfcAvailable"
              type="submit"
              :disabled="scanning || !holderName"
              class="w-full font-bold py-4 rounded-xl hover:shadow-lg transition disabled:opacity-50 flex items-center justify-center gap-2"
              style="background-color: #005145; color: #ffffff;"
            >
              <ion-icon v-if="!scanning" :icon="scanOutline" class="text-2xl"></ion-icon>
              <ion-spinner v-if="scanning" name="crescent"></ion-spinner>
              {{ scanning ? 'Scanning...' : 'Scan NFC/RFID Card' }}
            </button>

            <!-- Register Button -->
            <button
              v-if="scannedRFID || manualRFID"
              type="button"
              @click="registerCard"
              :disabled="registering || !holderName"
              class="w-full font-bold py-4 rounded-xl hover:shadow-lg transition disabled:opacity-50"
              style="background-color: #005145; color: #ffffff;"
            >
              {{ registering ? 'Registering...' : 'Complete Registration' }}
            </button>

            <!-- Cancel Button -->
            <button
              v-if="scannedRFID"
              type="button"
              @click="resetForm"
              class="w-full bg-gray-300 text-gray-700 font-bold py-3 rounded-xl hover:bg-gray-400 transition"
            >
              Cancel & Scan Again
            </button>
          </form>
        </div>

        <!-- NFC Not Available -->
        <div v-if="!nfcAvailable" class="bg-gray-50 border-2 border-gray-300 rounded-xl p-4">
          <p class="text-sm font-semibold text-gray-800 mb-2">⚠️ NFC Not Available</p>
          <p class="text-xs text-gray-700">
            This device doesn't support NFC or NFC is disabled. Please enable NFC in your device settings.
          </p>
        </div>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref, onMounted, onBeforeUnmount } from 'vue'
import { IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonIcon, IonSpinner, toastController } from '@ionic/vue'
import { scanOutline } from 'ionicons/icons'
import { useAuthStore } from '@/stores/auth'
import { Capacitor } from '@capacitor/core'
import { addDoc, collection } from 'firebase/firestore'
import { db } from '@/firebase'
import { useRouter } from 'vue-router'

const authStore = useAuthStore()
const router = useRouter()

const holderName = ref('')
const scannedRFID = ref('')
const manualRFID = ref('')
const scanning = ref(false)
const registering = ref(false)
const error = ref(null)
const nfcAvailable = ref(false)

let NFC = null
let unsubscribeRead = null
let unsubscribeError = null

// Handle manual RFID input
const handleManualInput = () => {
  if (manualRFID.value) {
    // Convert to uppercase and remove any non-hex characters
    manualRFID.value = manualRFID.value.toUpperCase().replace(/[^0-9A-F]/g, '')
    scannedRFID.value = '' // Clear scanned value if manually entering
    error.value = null
  }
}

// Check NFC availability
onMounted(async () => {
  if (Capacitor.isNativePlatform()) {
    try {
      // Import NFC plugin
      const NFCModule = await import('@exxili/capacitor-nfc')
      NFC = NFCModule.NFC

      // Check if NFC is supported
      const { supported } = await NFC.isSupported()
      nfcAvailable.value = supported
    } catch (err) {
      console.error('NFC not supported:', err)
      nfcAvailable.value = false
    }
  } else {
    // NFC not available on web
    nfcAvailable.value = false
  }
})

// Start NFC scan
const startNFCScan = async () => {
  if (!holderName.value) return

  error.value = null
  scanning.value = true

  try {
    if (Capacitor.isNativePlatform() && NFC) {
      // Listen for NFC tag detection
      unsubscribeRead = NFC.onRead((data) => {
        const messages = data.string()
        
        // Get tag UID from tagInfo
        if (messages.tagInfo && messages.tagInfo.uid) {
          scannedRFID.value = messages.tagInfo.uid.toUpperCase()
          scanning.value = false
          
          // Clean up listeners
          if (unsubscribeRead) unsubscribeRead()
          if (unsubscribeError) unsubscribeError()
        }
      })

      // Handle errors
      unsubscribeError = NFC.onError((nfcError) => {
        error.value = 'Scan failed: ' + nfcError.error
        scanning.value = false
        
        // Clean up listeners
        if (unsubscribeRead) unsubscribeRead()
        if (unsubscribeError) unsubscribeError()
      })
      
      // Only call startScan on iOS (not needed on Android)
      if (Capacitor.getPlatform() === 'ios') {
        await NFC.startScan()
      }
    } else {
      // NFC only works on native platforms
      error.value = 'NFC scanning is only available on mobile devices. Please use the Android app.'
      scanning.value = false
    }
  } catch (err) {
    error.value = 'Error starting NFC scan: ' + err.message
    scanning.value = false
  }
}

// Register the card
const registerCard = async () => {
  registering.value = true
  error.value = null

  // Use manual input if available, otherwise use scanned RFID
  const rfidValue = manualRFID.value || scannedRFID.value

  if (!rfidValue) {
    error.value = 'Please enter or scan an RFID card number'
    registering.value = false
    return
  }

  // Validate RFID format (should be hex characters, typically 8 or 14 characters)
  const hexPattern = /^[0-9A-F]+$/
  if (!hexPattern.test(rfidValue)) {
    error.value = 'Invalid RFID format. Should only contain numbers and letters A-F (e.g., AA3A6806)'
    registering.value = false
    return
  }

  try {
    await addDoc(collection(db, 'rfidCards'), {
      userId: authStore.user.uid,
      holderName: holderName.value,
      rfid: rfidValue.toUpperCase(),
      createdAt: new Date().toISOString()
    })

    const toast = await toastController.create({
      message: 'RFID card registered successfully!',
      duration: 2000,
      color: 'success',
      position: 'top'
    })
    await toast.present()

    // Navigate back to dashboard
    router.push('/user/dashboard')
  } catch (err) {
    error.value = 'Error registering card: ' + err.message
  } finally {
    registering.value = false
  }
}

// Reset form
const resetForm = () => {
  scannedRFID.value = ''
  manualRFID.value = ''
  error.value = null
}

// Cleanup on unmount
onBeforeUnmount(() => {
  if (unsubscribeRead) unsubscribeRead()
  if (unsubscribeError) unsubscribeError()
})
</script>
