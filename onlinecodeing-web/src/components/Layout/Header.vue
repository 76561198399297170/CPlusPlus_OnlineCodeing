<!-- src/components/Layout/Header.vue -->
<template>
  <header class="header-container">
    <div class="container header-content">
      <!-- 左侧：Logo和导航 -->
      <div class="header-left">
        <!-- 网站Logo -->
        <div class="logo">
          <router-link to="/problems">
            <span class="logo-text">CodePractice</span>
          </router-link>
        </div>

        <!-- 导航菜单 -->
        <nav class="nav-menu">
          <router-link to="/problems" class="nav-item">题库</router-link>
          <router-link to="/training" class="nav-item">训练</router-link>
          <router-link to="/contest" class="nav-item">竞赛</router-link>
          <router-link v-if="isAdmin" to="/admin/problems" class="nav-item">题目</router-link>
          <router-link v-if="isAdmin" to="/users" class="nav-item">用户</router-link>
        </nav>
      </div>

      <!-- 中间：搜索框 -->
      <div class="search-container">
        <input
          type="text"
          class="search-input"
          v-model="searchKeyword"
          placeholder="搜索题目..."
          @keyup.enter="handleSearch"
        />
        <button class="search-btn" @click="handleSearch">搜索</button>
      </div>

      <!-- 右侧：用户操作区 -->
      <div class="user-actions">
        <template v-if="isLogin">
          <span class="user-name" @click="navigateToPerson">
            {{ userInfo.nickname || userInfo.account }}
          </span>
          <div class="user-avatar" @click="navigateToPerson">
            <!-- 这里可以放用户头像图片，暂时用首字母代替 -->
            <span class="avatar-text">{{ userInitial }}</span>
          </div>
          <button class="logout-btn" @click="handleLogout">退出登录</button>
        </template>
        <template v-else>
          <router-link to="/register" class="register-btn">注册</router-link>
          <span class="or-text">或</span>
          <router-link to="/login" class="login-btn">登录</router-link>
          <div class="user-avatar">
            <!-- 空白圆形头像 -->
            <span class="avatar-text">?</span>
          </div>
        </template>
      </div>
    </div>
  </header>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'

// 路由实例
const router = useRouter()

// 响应式数据
const isLogin = ref(false)
const userInfo = ref({})
const searchKeyword = ref('')

// 计算属性：用户头像首字母
const userInitial = computed(() => {
  if (userInfo.value.nickname) {
    return userInfo.value.nickname.charAt(0).toUpperCase()
  } else if (userInfo.value.account) {
    return userInfo.value.account.charAt(0).toUpperCase()
  }
  return '?'
})

// 计算属性：是否为管理员
const isAdmin = computed(() => {
  return userInfo.value.role === 'admin'
})

// 初始化用户登录状态
onMounted(() => {
  const token = localStorage.getItem('token')
  const info = localStorage.getItem('userInfo')

  if (token && info) {
    isLogin.value = true
    userInfo.value = JSON.parse(info)
  }

  // 监听用户信息更新事件
  window.addEventListener('user-info-updated', (e) => {
    if (e.detail && e.detail.userInfo) {
      userInfo.value = e.detail.userInfo
      // 同时更新localStorage，确保一致性
      localStorage.setItem('userInfo', JSON.stringify(e.detail.userInfo))
    }
  })
})

// 跳转到个人中心
const navigateToPerson = () => {
  router.push('/person')
}

// 退出登录处理
const handleLogout = () => {
  localStorage.removeItem('token')
  localStorage.removeItem('userInfo')
  isLogin.value = false
  router.push('/login')
}

// 搜索处理
const handleSearch = () => {
  const keyword = searchKeyword.value.trim()
  // 构建搜索页面的URL
  const searchUrl = new URL(window.location.origin + '/search')
  if (keyword) {
    searchUrl.searchParams.set('keyword', keyword)
  }
  // 使用刷新的方式进入搜索页面
  window.location.href = searchUrl.toString()
}
</script>

<style scoped>
/* 头部容器 */
.header-container {
  height: 60px;
  background-color: #ffffff;
  border-bottom: 1px solid #e5e6eb;
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  z-index: 1000;
}

/* 头部内容 */
.header-content {
  height: 100%;
  display: flex;
  align-items: center;
  justify-content: space-between;
}

/* 左侧：Logo和导航 */
.header-left {
  display: flex;
  align-items: center;
  gap: 30px;
}

/* Logo样式 */
.logo {
  flex-shrink: 0;
  .logo-text {
    font-size: 18px;
    font-weight: 600;
    color: #165dff;
    text-decoration: none;
  }
}

/* 导航菜单 */
.nav-menu {
  display: flex;
  gap: 30px;
  flex-shrink: 0;

  .nav-item {
    color: #1d2129;
    text-decoration: none;
    font-size: 14px;
    font-weight: 500;
    transition: all 0.2s ease;

    &:hover {
      color: #165dff;
    }
  }
}

/* 搜索框 */
.search-container {
  display: flex;
  align-items: center;
  flex-shrink: 0;
  width: 30%;
  max-width: 400px;
  min-width: 250px;

  .search-input {
    flex: 1;
    height: 36px;
    padding: 0 12px;
    border: 1px solid #e5e6eb;
    border-radius: 18px 0 0 18px;
    font-size: 14px;
    outline: none;
    transition: all 0.2s ease;

    &:focus {
      border-color: #165dff;
    }

    &::placeholder {
      color: #86909c;
    }
  }

  .search-btn {
    height: 36px;
    padding: 0 16px;
    background-color: #165dff;
    color: #ffffff;
    border: none;
    border-radius: 0 18px 18px 0;
    font-size: 14px;
    cursor: pointer;
    transition: all 0.2s ease;

    &:hover {
      background-color: #0e42c5;
    }
  }
}

/* 用户操作区 */
.user-actions {
  display: flex;
  align-items: center;
  gap: 10px;
  flex-shrink: 0;

  .user-name {
    font-size: 14px;
    color: #4e5969;
    white-space: nowrap;
    cursor: pointer;
    transition: all 0.2s ease;

    &:hover {
      color: #165dff;
    }
  }

  .user-avatar {
    width: 36px;
    height: 36px;
    border-radius: 50%;
    background-color: #f7f8fa;
    display: flex;
    align-items: center;
    justify-content: center;
    border: 1px solid #e5e6eb;
    flex-shrink: 0;
    cursor: pointer;
    transition: all 0.2s ease;

    &:hover {
      border-color: #165dff;
      background-color: rgba(22, 93, 255, 0.05);
    }

    .avatar-text {
      font-size: 16px;
      font-weight: 600;
      color: #165dff;
    }
  }

  .login-btn {
    color: #165dff;
    text-decoration: none;
    font-size: 14px;
    padding: 6px 12px;
    border-radius: 6px;
    transition: all 0.2s ease;

    &:hover {
      background-color: rgba(22, 93, 255, 0.05);
    }
  }

  .register-btn {
    color: #ffffff;
    background-color: #165dff;
    text-decoration: none;
    font-size: 14px;
    padding: 6px 16px;
    border-radius: 6px;
    transition: all 0.2s ease;

    &:hover {
      background-color: #0e42c5;
    }
  }

  .logout-btn {
    color: #f53f3f;
    background: transparent;
    border: none;
    font-size: 14px;
    cursor: pointer;
    padding: 6px 12px;
    border-radius: 6px;
    transition: all 0.2s ease;

    &:hover {
      background-color: rgba(245, 63, 63, 0.05);
    }
  }

  .or-text {
    font-size: 14px;
    color: #86909c;
  }
}
</style>
