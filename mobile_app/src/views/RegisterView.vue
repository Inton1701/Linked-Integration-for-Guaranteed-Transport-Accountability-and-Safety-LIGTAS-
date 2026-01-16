<template>
  <ion-page>
    <ion-content :fullscreen="true" style="--background: #ffffff;">
      <div class="flex items-center justify-center min-h-screen p-6">
        <div class="w-full max-w-md">
          <!-- Logo/Title -->
          <div class="text-center mb-8">
            <img :src="logo" alt="L.I.G.T.A.S Logo" class="w-32 h-32 mx-auto mb-4" />
            <h1 class="text-3xl font-bold mb-1" style="color: #005145;">L.I.G.T.A.S</h1>
            <p class="text-gray-600 text-sm">Create Your Account</p>
          </div>

          <!-- Register Form -->
          <form @submit.prevent="handleRegister">
            <div class="space-y-4">
              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Full Name</label>
                <input
                  v-model="name"
                  type="text"
                  required
                  placeholder="John Doe"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none transition"
                  style="focus:border-color: #005145;"
                />
              </div>

              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Email</label>
                <input
                  v-model="email"
                  type="email"
                  required
                  placeholder="your@email.com"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none focus:border-primary transition"
                />
              </div>

              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Password</label>
                <input
                  v-model="password"
                  type="password"
                  required
                  minlength="6"
                  placeholder="••••••••"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none focus:border-primary transition"
                />
              </div>

              <div>
                <label class="block text-sm font-semibold text-gray-700 mb-2">Confirm Password</label>
                <input
                  v-model="confirmPassword"
                  type="password"
                  required
                  placeholder="••••••••"
                  class="w-full px-4 py-3 border-2 border-gray-200 rounded-xl focus:outline-none focus:border-primary transition"
                />
              </div>

              <!-- Error Message -->
              <div v-if="error" class="bg-red-100 border border-red-400 text-red-700 px-4 py-3 rounded-xl">
                {{ error }}
              </div>

              <!-- Register Button -->
              <button
                type="submit"
                :disabled="loading"
                class="w-full font-bold py-3 rounded-xl hover:shadow-lg transition disabled:opacity-50"
                style="background-color: #005145; color: #FFFF;"
              >
                {{ loading ? 'Creating Account...' : 'Register' }}
              </button>
            </div>
          </form>

          <!-- Login Link -->
          <div class="text-center mt-6">
            <p class="text-gray-600">
              Already have an account?
              <router-link to="/login" class="font-semibold hover:underline" style="color: #005145;">
                Login
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

const name = ref('')
const email = ref('')
const password = ref('')
const confirmPassword = ref('')
const loading = ref(false)
const error = ref(null)

const handleRegister = async () => {
  loading.value = true
  error.value = null

  if (password.value !== confirmPassword.value) {
    error.value = 'Passwords do not match'
    loading.value = false
    return
  }

  try {
    await authStore.register(email.value, password.value, name.value, 'user')
  } catch (err) {
    error.value = err.message || 'Registration failed'
  } finally {
    loading.value = false
  }
}
</script>
