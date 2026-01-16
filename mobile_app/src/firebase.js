import { initializeApp } from 'firebase/app'
import { getAuth } from 'firebase/auth'
import { getFirestore } from 'firebase/firestore'

// Firebase Web App Configuration
const firebaseConfig = {
  apiKey: "AIzaSyAge9lFaqLvPGVo_t6hymfis4FP_ESZXFc",
  authDomain: "studious-bit-420613.firebaseapp.com",
  projectId: "studious-bit-420613",
  storageBucket: "studious-bit-420613.firebasestorage.app",
  messagingSenderId: "733217464214",
  appId: "1:733217464214:web:53dbc48864c2f7852507b7",
  measurementId: "G-27N621TM94"
}

// Initialize Firebase
const app = initializeApp(firebaseConfig)

// Initialize services
export const auth = getAuth(app)
export const db = getFirestore(app)

// Messaging is initialized dynamically in useNotifications composable
// Only for web platform (not on native mobile)
export const messaging = null

export default app
