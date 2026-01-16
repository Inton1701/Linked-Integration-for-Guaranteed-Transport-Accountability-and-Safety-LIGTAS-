import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import { 
  signInWithEmailAndPassword, 
  createUserWithEmailAndPassword,
  signOut,
  onAuthStateChanged
} from 'firebase/auth'
import { doc, setDoc, getDoc } from 'firebase/firestore'
import { auth, db } from '@/firebase'
import router from '@/router'

export const useAuthStore = defineStore('auth', () => {
  const user = ref(null)
  const userProfile = ref(null)
  const loading = ref(false)
  const error = ref(null)

  const isAuthenticated = computed(() => !!user.value)
  const isAdmin = computed(() => userProfile.value?.role === 'admin')

  // Check auth state
  const checkAuthState = () => {
    onAuthStateChanged(auth, async (firebaseUser) => {
      if (firebaseUser) {
        user.value = firebaseUser
        await loadUserProfile(firebaseUser.uid)
      } else {
        user.value = null
        userProfile.value = null
      }
    })
  }

  // Load user profile from Firestore
  const loadUserProfile = async (uid) => {
    try {
      const docRef = doc(db, 'users', uid)
      const docSnap = await getDoc(docRef)
      
      if (docSnap.exists()) {
        userProfile.value = docSnap.data()
      } else {
        // Profile doesn't exist - check if it's admin email
        const ADMIN_EMAIL = 'admin@ligtas.com'
        if (user.value?.email?.toLowerCase() === ADMIN_EMAIL.toLowerCase()) {
          // Auto-create admin profile
          const adminProfile = {
            uid,
            email: user.value.email,
            name: 'Admin',
            role: 'admin',
            createdAt: new Date().toISOString(),
            fcmToken: null
          }
          await setDoc(docRef, adminProfile)
          userProfile.value = adminProfile
          console.log('Admin profile auto-created')
        } else {
          console.error('User profile not found in Firestore')
        }
      }
    } catch (err) {
      console.error('Error loading user profile:', err)
    }
  }

  // Login
  const login = async (email, password) => {
    loading.value = true
    error.value = null
    
    try {
      const userCredential = await signInWithEmailAndPassword(auth, email, password)
      user.value = userCredential.user
      await loadUserProfile(userCredential.user.uid)
      
      console.log('Login - User Profile:', userProfile.value) // Debug log
      console.log('Login - Role:', userProfile.value?.role) // Debug log
      
      // Navigate based on role
      if (userProfile.value?.role === 'admin') {
        router.push('/admin/dashboard')
      } else {
        router.push('/user/dashboard')
      }
    } catch (err) {
      error.value = err.message
      throw err
    } finally {
      loading.value = false
    }
  }

  // Register
  const register = async (email, password, name, role = 'user') => {
    loading.value = true
    error.value = null
    
    try {
      const userCredential = await createUserWithEmailAndPassword(auth, email, password)
      const uid = userCredential.user.uid
      
      // Static admin email - automatically gets admin role
      const ADMIN_EMAIL = 'admin@ligtas.com'
      const finalRole = email.toLowerCase() === ADMIN_EMAIL.toLowerCase() ? 'admin' : role
      
      // Create user profile in Firestore
      await setDoc(doc(db, 'users', uid), {
        uid,
        email,
        name,
        role: finalRole,
        createdAt: new Date().toISOString(),
        fcmToken: null
      })
      
      user.value = userCredential.user
      userProfile.value = { uid, email, name, role: finalRole }
      
      // Navigate based on role
      if (finalRole === 'admin') {
        router.push('/admin/dashboard')
      } else {
        router.push('/user/dashboard')
      }
    } catch (err) {
      error.value = err.message
      throw err
    } finally {
      loading.value = false
    }
  }

  // Logout
  const logout = async () => {
    try {
      await signOut(auth)
      user.value = null
      userProfile.value = null
      router.push('/login')
    } catch (err) {
      error.value = err.message
      throw err
    }
  }

  // Update FCM token
  const updateFCMToken = async (token) => {
    if (user.value) {
      try {
        await setDoc(doc(db, 'users', user.value.uid), {
          fcmToken: token
        }, { merge: true })
      } catch (err) {
        console.error('Error updating FCM token:', err)
      }
    }
  }

  return {
    user,
    userProfile,
    loading,
    error,
    isAuthenticated,
    isAdmin,
    checkAuthState,
    login,
    register,
    logout,
    updateFCMToken
  }
})
