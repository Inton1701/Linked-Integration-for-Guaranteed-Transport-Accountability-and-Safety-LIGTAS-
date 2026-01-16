import { createRouter, createWebHistory } from '@ionic/vue-router'
import { useAuthStore } from '@/stores/auth'

const routes = [
  {
    path: '/',
    redirect: '/login'
  },
  {
    path: '/login',
    name: 'Login',
    component: () => import('@/views/LoginView.vue')
  },
  {
    path: '/register',
    name: 'Register',
    component: () => import('@/views/RegisterView.vue')
  },
  {
    path: '/admin',
    component: () => import('@/views/admin/AdminLayout.vue'),
    meta: { requiresAuth: true, requiresAdmin: true },
    children: [
      {
        path: '',
        redirect: '/admin/dashboard'
      },
      {
        path: 'dashboard',
        name: 'AdminDashboard',
        component: () => import('@/views/admin/DashboardView.vue')
      },
      {
        path: 'devices',
        name: 'AdminDevices',
        component: () => import('@/views/admin/DevicesView.vue')
      },
      {
        path: 'users',
        name: 'AdminUsers',
        component: () => import('@/views/admin/UsersView.vue')
      },
      {
        path: 'scan-logs',
        name: 'AdminScanLogs',
        component: () => import('@/views/admin/ScanLogsView.vue')
      }
    ]
  },
  {
    path: '/user',
    component: () => import('@/views/user/UserLayout.vue'),
    meta: { requiresAuth: true },
    children: [
      {
        path: '',
        redirect: '/user/dashboard'
      },
      {
        path: 'dashboard',
        name: 'UserDashboard',
        component: () => import('@/views/user/DashboardView.vue')
      },
      {
        path: 'register-rfid',
        name: 'RegisterRFID',
        component: () => import('@/views/user/RegisterRFIDView.vue')
      },
      {
        path: 'notifications',
        name: 'Notifications',
        component: () => import('@/views/user/NotificationsView.vue')
      },
      {
        path: 'history',
        name: 'History',
        component: () => import('@/views/user/HistoryView.vue')
      }
    ]
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes
})

// Navigation guard
router.beforeEach(async (to, from, next) => {
  const authStore = useAuthStore()
  
  if (to.meta.requiresAuth && !authStore.isAuthenticated) {
    next('/login')
  } else if (to.meta.requiresAdmin && !authStore.isAdmin) {
    next('/user/dashboard')
  } else {
    next()
  }
})

export default router
