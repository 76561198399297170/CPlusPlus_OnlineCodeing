<template>
  <MainLayout>
    <div class="login-page">
      <div class="form-container">
        <h2 class="title">用户登录</h2>

        <!-- 角色选择组件 -->
        <RoleSelector v-model="selectedRole" />

        <!-- 账号输入 -->
        <div class="form-item">
          <label class="form-item__label">登录账号</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.account"
            placeholder="请输入您的登录账号"
          />
        </div>

        <!-- 密码输入 -->
        <div class="form-item">
          <label class="form-item__label">登录密码</label>
          <input
            type="password"
            class="form-item__input"
            v-model="form.password"
            placeholder="请输入您的登录密码"
            @keyup.enter="handleLogin"
          />
        </div>

        <!-- 登录按钮 -->
        <Button type="primary" size="large" class="submit-btn" @click="handleLogin"> 登录 </Button>

        <!-- 注册链接 -->
        <div class="link-container">
          还没有账号？<router-link to="/register" class="link">立即注册</router-link>
        </div>
      </div>
    </div>

    <!-- 通知消息 -->
    <div v-for="notification in notifications" :key="notification.id">
      <NotificationMessage
        :visible="notification.visible"
        :title="notification.title"
        :message="notification.message"
        :type="notification.type"
        :duration="notification.duration"
        @close="removeNotification(notification.id)"
      />
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, getCurrentInstance } from 'vue'
import { useRouter } from 'vue-router'
// 引入组件
import MainLayout from '@/components/Layout/MainLayout.vue'
import RoleSelector from '@/components/Common/RoleSelector.vue'
import Button from '@/components/Common/Button.vue'
import NotificationMessage from '@/components/Common/NotificationMessage.vue'
import { useNotification } from '@/composables/useNotification'

// ========== 核心依赖与实例 ==========
const router = useRouter()
const { proxy } = getCurrentInstance()
const axios = proxy.$axios
const {
  notifications,
  success,
  error: notifyError,
  warning,
  remove: removeNotification,
} = useNotification()

// ========== 响应式数据 ==========
// 选中的角色
const selectedRole = ref('ADMIN')
// 表单数据
const form = ref({
  account: '',
  password: '',
})

// ========== 表单校验规则 ==========
/**
 * 表单校验函数
 * @returns {Boolean} 校验结果：true-通过，false-失败
 */
const validateForm = () => {
  // 账号校验：非空
  if (!form.value.account.trim()) {
    warning('账号不能为空！')
    return false
  }

  // 密码校验：非空 + 长度≥6
  if (!form.value.password.trim()) {
    warning('密码不能为空！')
    return false
  }
  if (form.value.password.length < 6) {
    warning('密码长度不能少于6位！')
    return false
  }

  return true
}

// ========== 登录核心逻辑 ==========
/**
 * 登录处理函数
 * 仅在用户主动点击登录按钮时触发
 */
const handleLogin = async () => {
  // 1. 前端表单校验
  if (!validateForm()) {
    return
  }

  try {
    // 2. 构造请求参数（角色转小写，兼容后端）
    const reqData = {
      account: form.value.account.trim(),
      password: form.value.password.trim(),
      role: selectedRole.value.toLowerCase(), // ADMIN → admin
    }

    // 3. 发送登录请求（使用全局axios）
    const res = await axios.post('/login', reqData)

    // 4. 处理后端响应
    if (res.data && res.data.code === 0) {
      // 登录成功
      success('登录成功，正在跳转...')
      // 存储登录状态
      localStorage.setItem('token', res.data.data.token || 'mock-token')
      localStorage.setItem('userInfo', JSON.stringify(res.data.data))
      // 延迟跳转，提升用户体验
      setTimeout(() => {
        router.push('/problems')
      }, 1000)
    } else {
      // 后端返回业务失败
      notifyError(res.data?.msg || '登录失败，请检查账号密码或角色！')
    }
  } catch (error) {
    // 5. 捕获错误信息
    notifyError('登录失败：请检查后端服务是否启动或网络是否正常！')
  }
}
</script>

<style scoped>
/* 登录页面容器 */
.login-page {
  padding: 40px 0;
  min-height: calc(100vh - 60px - 80px);
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: #f7f8fa;
}

/* 表单容器自定义样式 */
.form-container {
  width: 100%;
  max-width: 450px;
  padding: 40px;
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.1);
}

/* 标题样式 */
.title {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  text-align: center;
  margin-bottom: 30px;
}

/* 表单项目样式 */
.form-item {
  margin-bottom: 20px;
}

.form-item__label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-item__input {
  width: 100%;
  height: 48px;
  padding: 0 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  font-size: 14px;
  color: #1d2129;
  transition: all 0.2s ease;
}

.form-item__input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.form-item__input::placeholder {
  color: #86909c;
}

/* 链接容器 */
.link-container {
  text-align: center;
  margin-top: 20px;
  font-size: 14px;
  color: #4e5969;

  .link {
    color: #165dff;
    text-decoration: none;
    font-weight: 500;
    transition: color 0.2s ease;

    &:hover {
      color: #0e42c5;
      text-decoration: underline;
    }
  }
}
</style>
