// src/router/index.ts
import { createRouter, createWebHistory } from 'vue-router'
import type { RouteRecordRaw } from 'vue-router'
import Login from '@/views/auth/Login.vue'
import Register from '@/views/auth/Register.vue'
import Index from '@/views/home/Index.vue'
import Problems from '@/views/problems/Index.vue'
import CreateProblem from '@/views/problems/CreateProblem.vue'
import ReviewProblems from '@/views/problems/ReviewProblems.vue'
import Analysis from '@/views/analysis/Index.vue'
import Help from '@/views/help/Index.vue'
import Submissions from '@/views/submissions/Index.vue'
import Person from '@/views/person/Index.vue'

import Training from '@/views/training/Index.vue'
import ProblemSetDetail from '@/views/training/ProblemSetDetail.vue'
import Test from '@/views/training/Test.vue'
import Contest from '@/views/contest/Index.vue'
import Editor from '@/views/editor/Index.vue'
import Profile from '@/views/profile/Index.vue'
import Users from '@/views/users/Index.vue'
import Search from '@/views/search/Index.vue'
import AdminProblems from '@/views/admin/Problems.vue'

// 定义路由类型
const routes: Array<RouteRecordRaw> = [
  // 原有路由逻辑不变
  {
    path: '/',
    redirect: '/problems',
  },
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: { title: '用户登录 - CodePractice' },
  },
  {
    path: '/register',
    name: 'Register',
    component: Register,
    meta: { title: '用户注册 - CodePractice' },
  },
  {
    path: '/index',
    name: 'Index',
    component: Index,
    meta: { title: '首页 - CodePractice', requireAuth: true },
  },
  {
    path: '/problems',
    name: 'Problems',
    component: Problems,
    meta: { title: '题目列表 - CodePractice', requireAuth: true },
  },
  {
    path: '/problems/create',
    name: 'CreateProblem',
    component: CreateProblem,
    meta: { title: '创建题目 - CodePractice', requireAuth: true },
  },
  {
    path: '/problems/review',
    name: 'ReviewProblems',
    component: ReviewProblems,
    meta: { title: '题目审核 - CodePractice', requireAuth: true },
  },
  {
    path: '/analysis',
    name: 'Analysis',
    component: Analysis,
    meta: { title: '能力分析 - CodePractice', requireAuth: true },
  },
  {
    path: '/help',
    name: 'Help',
    component: Help,
    meta: { title: '使用帮助 - CodePractice' },
  },
  {
    path: '/submissions',
    name: 'Submissions',
    component: Submissions,
    meta: { title: '提交记录 - CodePractice', requireAuth: true },
  },
  {
    path: '/person',
    name: 'Person',
    component: Person,
    meta: { title: '个人中心 - CodePractice', requireAuth: true },
  },

  {
    path: '/training',
    name: 'Training',
    component: Training,
    meta: { title: '训练 - CodePractice', requireAuth: true },
  },
  {
    path: '/training/set/:id',
    name: 'ProblemSetDetail',
    component: ProblemSetDetail,
    meta: { title: '题目组详情 - CodePractice', requireAuth: true },
  },
  {
    path: '/training/test',
    name: 'Test',
    component: Test,
    meta: { title: '测试页面 - CodePractice', requireAuth: true },
  },
  {
    path: '/contest',
    name: 'Contest',
    component: Contest,
    meta: { title: '竞赛 - CodePractice', requireAuth: true },
  },
  {
    path: '/editor/:id?',
    name: 'Editor',
    component: Editor,
    meta: { title: '在线代码编辑 - CodePractice', requireAuth: true },
  },
  {
    path: '/profile/:id',
    name: 'Profile',
    component: Profile,
    meta: { title: '用户主页 - CodePractice', requireAuth: true },
  },
  {
    path: '/users',
    name: 'Users',
    component: Users,
    meta: { title: '用户管理 - CodePractice', requireAuth: true },
  },
  {
    path: '/admin/problems',
    name: 'AdminProblems',
    component: AdminProblems,
    meta: { title: '题目管理 - CodePractice', requireAuth: true, requireAdmin: true },
  },
  {
    path: '/search',
    name: 'Search',
    component: Search,
    meta: { title: '搜索 - CodePractice', requireAuth: true },
  },
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
  scrollBehavior() {
    return { top: 0 }
  },
})

// 路由守卫逻辑：只对用户相关页面进行登录状态检查
router.beforeEach((to, from) => {
  if (to.meta.title) {
    document.title = to.meta.title as string
  }

  // 需要登录权限的页面
  const protectedPages = [
    '/problems',
    '/problems/create',
    '/problems/review',
    '/submissions',
    '/analysis',
    '/person',
    '/editor',
    '/training',
  ]
  const isLogin = localStorage.getItem('token')

  if (protectedPages.includes(to.path) && !isLogin) {
    return '/login'
  }

  // 需要管理员权限的页面
  if (to.meta.requireAdmin) {
    const userInfo = localStorage.getItem('userInfo')
    if (userInfo) {
      try {
        const user = JSON.parse(userInfo)
        if (user.role !== 'admin') {
          return '/problems'
        }
      } catch (e) {
        return '/login'
      }
    } else {
      return '/login'
    }
  }

  // 不需要返回值，默认允许导航
})

export default router
