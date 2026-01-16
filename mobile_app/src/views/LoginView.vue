<template>
  <ion-page>
    <ion-content :fullscreen="true" style="--background: #ffffff;">
      <div class="flex items-center justify-center min-h-screen p-6">
        <div class="w-full max-w-md">
          <!-- Logo/Title -->
          <div class="text-center mb-8">
            <img :src="logo" alt="L.I.G.T.A.S Logo" class="w-32 h-32 mx-auto mb-4" />
            <h1 class="text-3xl font-bold mb-1" style="color: #005145;">L.I.G.T.A.S</h1>
            <p class="text-gray-600 text-sm">Linked Integration for Guaranteed Transport</p>
            <p class="text-gray-600 text-sm">Accountability and Safety</p>
          </div>

          <!-- Login Form -->
          <form @submit.prevent="handleLogin">
            <div class="space-y-4">
              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Email</label>
                <input
                  v-model="email"
                  type="email"
                  required
                  placeholder="your@email.com"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none transition"
                  style="focus:border-color: #005145;"
                />
              </div>

              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Password</label>
                <input
                  v-model="password"
                  type="password"
                  required
                  placeholder="••••••••"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none transition"
                  style="focus:border-color: #005145;"
                />
              </div>

              <!-- Error Message -->
              <div v-if="error" class="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded-xl">
                {{ error }}
              </div>

              <!-- Login Button -->
              <button
                type="submit"
                :disabled="loading"
                class="w-full font-bold py-3 rounded-xl hover:shadow-lg transition disabled:opacity-50"
                style="background-color: #005145; color: #FFFF;"
              >
                {{ loading ? 'Logging in...' : 'Login' }}
              </button>
            </div>
          </form>

          <!-- Register Link -->
          <div class="text-center mt-6">
            <p class="text-gray-600">
              Don't have an account?
              <router-link to="/register" class="font-semibold hover:underline" style="color: #005145;">
                Register
              </router-link>
            </p>
          </div>
        </div>
      </div>
    </ion-content>
  </ion-page>
</template>

<script setup>
import { ref } from 'vue'
import { IonPage, IonContent } from '@ionic/vue'
import { useAuthStore } from '@/stores/auth'
import logo from '@/assets/logo.png'

const authStore = useAuthStore()

const email = ref('')
const password = ref('')
const loading = ref(false)
const error = ref(null)

// Quick login credentials
const ADMIN_CREDENTIALS = {
  email: 'admin@ligtas.com',
  password: 'admin123'
}

const USER_CREDENTIALS = {
  email: 'user@demo.com',
  password: 'user123'
}

const quickLogin = async (type) => {
  const credentials = type === 'admin' ? ADMIN_CREDENTIALS : USER_CREDENTIALS
  email.value = credentials.email
  password.value = credentials.password
  await handleLogin()
}

const handleLogin = async () => {
  loading.value = true
  error.value = null

  try {
    await authStore.login(email.value, password.value)
  } catch (err) {
    error.value = 'Invalid email or password'
  } finally {
    loading.value = false
  }
}
</script>
