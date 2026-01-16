<template>
  <ion-page>
    <ion-content>
      <router-view />
    </ion-content>
    
    <!-- Custom tab bar -->
    <div class="custom-tab-bar">
      <button 
        @click="navigate('/admin/dashboard')"
        :class="['tab-button', { active: isActive('/admin/dashboard') }]"
      >
        <ion-icon :icon="homeOutline" />
        <span class="tab-label">Dashboard</span>
      </button>
      
      <button 
        @click="navigate('/admin/devices')"
        :class="['tab-button', { active: isActive('/admin/devices') }]"
      >
        <ion-icon :icon="hardwareChipOutline" />
        <span class="tab-label">Devices</span>
      </button>
      
      <button 
        @click="navigate('/admin/users')"
        :class="['tab-button', { active: isActive('/admin/users') }]"
      >
        <ion-icon :icon="peopleOutline" />
        <span class="tab-label">Users</span>
      </button>
      
      <button 
        @click="navigate('/admin/scan-logs')"
        :class="['tab-button', { active: isActive('/admin/scan-logs') }]"
      >
        <ion-icon :icon="documentTextOutline" />
        <span class="tab-label">Scan Logs</span>
      </button>
    </div>
  </ion-page>
</template>

<script setup>
import { IonPage, IonContent, IonIcon } from '@ionic/vue'
import { homeOutline, hardwareChipOutline, peopleOutline, documentTextOutline } from 'ionicons/icons'
import { useRouter, useRoute } from 'vue-router'

const router = useRouter()
const route = useRoute()

const navigate = (path) => {
  if (route.path !== path) {
    router.push(path)
  }
}

const isActive = (path) => route.path === path
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

ion-content {
  --padding-bottom: 70px;
}
</style>
