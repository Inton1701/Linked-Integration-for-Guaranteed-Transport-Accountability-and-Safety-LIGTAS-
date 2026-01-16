<template>
  <ion-page>
    <ion-header>
      <ion-toolbar style="--background: #005145; --color: #ffffff;">
        <ion-title>User Management</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content :fullscreen="true" class="ion-padding">
      <div class="space-y-4">
        <!-- User List -->
        <ion-card v-for="user in users" :key="user.id" class="mb-4">
          <ion-card-header>
            <ion-card-title class="text-lg">{{ user.name }}</ion-card-title>
            <ion-card-subtitle>{{ user.email }}</ion-card-subtitle>
          </ion-card-header>
          <ion-card-content>
            <div class="space-y-2 text-sm">
              <p><strong>Role:</strong> 
                <ion-badge :color="user.role === 'admin' ? 'primary' : 'secondary'">
                  {{ user.role }}
                </ion-badge>
              </p>
              <p><strong>Registered RFID Cards:</strong> {{ user.rfidCount || 0 }}</p>
              <p><strong>Joined:</strong> {{ formatDate(user.createdAt) }}</p>
            </div>
            <div class="flex gap-2 mt-4">
              <ion-button size="small" fill="outline" @click="viewUserDetails(user)">
                <ion-icon :icon="eyeOutline" slot="start"></ion-icon>
                View Details
              </ion-button>
            </div>
          </ion-card-content>
        </ion-card>

        <!-- Empty State -->
        <div v-if="users.length === 0" class="text-center py-12">
          <ion-icon :icon="peopleOutline" class="text-6xl text-gray-300 mb-4"></ion-icon>
          <p class="text-gray-500">No users found</p>
        </div>
      </div>
    </ion-content>

    <!-- Custom User Details Modal -->
    <div v-if="showModal" class="custom-modal-overlay" @click="closeModal">
      <div class="custom-modal" @click.stop>
        <!-- Modal Header -->
        <div class="modal-header">
          <h2 class="modal-title">User Details</h2>
          <button @click="closeModal" class="close-button">&times;</button>
        </div>
        
        <!-- Modal Content -->
        <div class="modal-content" v-if="selectedUser">
          <div class="space-y-6">
            <!-- User Info -->
            <div class="info-card">
              <h3 class="section-title">User Information</h3>
              <div class="space-y-2 text-sm">
                <p><strong>Name:</strong> {{ selectedUser.name }}</p>
                <p><strong>Email:</strong> {{ selectedUser.email }}</p>
                <p><strong>Role:</strong> {{ selectedUser.role }}</p>
                <p><strong>Joined:</strong> {{ formatDate(selectedUser.createdAt) }}</p>
              </div>
            </div>

            <!-- Registered RFID Cards -->
            <div class="info-card">
              <h3 class="section-title">Registered RFID Cards</h3>
              <div v-if="userRfidCards.length > 0" class="space-y-2">
                <div v-for="card in userRfidCards" :key="card.id" class="card-item">
                  <h4 class="font-semibold">{{ card.holderName }}</h4>
                  <p class="text-xs text-gray-600">{{ card.rfid }}</p>
                </div>
              </div>
              <p v-else class="text-gray-500 text-sm">No RFID cards registered</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  </ion-page>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import {
  IonPage, IonHeader, IonToolbar, IonTitle, IonContent, IonButton,
  IonIcon, IonCard, IonCardHeader, IonCardTitle, IonCardSubtitle, IonCardContent,
  IonBadge
} from '@ionic/vue'
import { peopleOutline, eyeOutline } from 'ionicons/icons'
import { collection, getDocs, query, where } from 'firebase/firestore'
import { db } from '@/firebase'

const users = ref([])
const showModal = ref(false)
const selectedUser = ref(null)
const userRfidCards = ref([])

const loadUsers = async () => {
  try {
    const querySnapshot = await getDocs(query(collection(db, 'users'), where('role', '==', 'user')))
    users.value = querySnapshot.docs.map(doc => ({
      id: doc.id,
      ...doc.data()
    }))

    // Load RFID card counts for each user
    for (const user of users.value) {
      const rfidSnap = await getDocs(query(collection(db, 'rfidCards'), where('userId', '==', user.id)))
      user.rfidCount = rfidSnap.size
    }
  } catch (error) {
    console.error('Error loading users:', error)
  }
}

const viewUserDetails = async (user) => {
  selectedUser.value = user
  
  // Load user's RFID cards
  try {
    const rfidSnap = await getDocs(query(collection(db, 'rfidCards'), where('userId', '==', user.id)))
    userRfidCards.value = rfidSnap.docs.map(doc => ({
      id: doc.id,
      ...doc.data()
    }))
  } catch (error) {
    console.error('Error loading RFID cards:', error)
  }
  
  showModal.value = true
}

const closeModal = () => {
  showModal.value = false
  selectedUser.value = null
  userRfidCards.value = []
}

const formatDate = (dateString) => {
  if (!dateString) return 'N/A'
  const date = new Date(dateString)
  return date.toLocaleDateString()
}

onMounted(() => {
  loadUsers()
})
</script>

<style scoped>
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

.info-card {
  background: #f9fafb;
  border-radius: 8px;
  padding: 16px;
  border: 1px solid #e5e7eb;
}

.section-title {
  font-weight: 700;
  font-size: 16px;
  margin-bottom: 12px;
  color: #005145;
}

.card-item {
  padding: 12px;
  background: white;
  border-radius: 6px;
  border: 1px solid #e5e7eb;
}

.space-y-2 > * + * {
  margin-top: 8px;
}

.space-y-6 > * + * {
  margin-top: 24px;
}
</style>
