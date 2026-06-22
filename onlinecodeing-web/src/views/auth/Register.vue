<template>
  <MainLayout>
    <div class="register-page">
      <div class="form-container">
        <h2 class="title">用户注册</h2>

        <!-- 角色选择组件 -->
        <RoleSelector v-model="selectedRole" />

        <!-- 账号输入 -->
        <div class="form-item">
          <label class="form-item__label">登录账号</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.account"
            placeholder="请设置您的登录账号"
          />
        </div>

        <!-- 手机号输入 -->
        <div class="form-item">
          <label class="form-item__label">手机号码</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.phone"
            placeholder="请输入您的手机号码"
          />
        </div>

        <!-- 邮箱输入 -->
        <div class="form-item">
          <label class="form-item__label">电子邮箱</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.email"
            placeholder="请输入您的电子邮箱"
          />
        </div>

        <!-- 密码输入 -->
        <div class="form-item">
          <label class="form-item__label">登录密码</label>
          <input
            type="password"
            class="form-item__input"
            v-model="form.password"
            placeholder="请设置您的登录密码（至少6位）"
          />
        </div>

        <!-- 确认密码 -->
        <div class="form-item">
          <label class="form-item__label">确认密码</label>
          <input
            type="password"
            class="form-item__input"
            v-model="form.confirmPwd"
            placeholder="请再次输入您的登录密码"
          />
        </div>

        <!-- 昵称输入 -->
        <div class="form-item">
          <label class="form-item__label">用户昵称</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.nickname"
            placeholder="请设置您的昵称（选填）"
          />
        </div>

        <!-- 学生专属字段 -->
        <div v-if="selectedRole === 'STUDENT'" class="form-item">
          <label class="form-item__label">所在学校</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.school"
            placeholder="请输入您的学校名称"
          />
        </div>
        <div v-if="selectedRole === 'STUDENT'" class="form-item">
          <label class="form-item__label">入学年级</label>
          <input
            type="number"
            class="form-item__input"
            v-model="form.grade"
            placeholder="请输入您的入学年级（如2024）"
          />
        </div>

        <!-- 学生/教师专属字段 -->
        <div v-if="selectedRole === 'STUDENT' || selectedRole === 'TEACHER'" class="form-item">
          <label class="form-item__label">真实姓名</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.realName"
            placeholder="请输入您的真实姓名"
          />
        </div>

        <!-- 教师专属字段 -->
        <div v-if="selectedRole === 'TEACHER'" class="form-item">
          <label class="form-item__label">任教学校</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.school"
            placeholder="请输入您的任教学校名称"
          />
        </div>

        <!-- 管理员专属字段 -->
        <div v-if="selectedRole === 'ADMIN'" class="form-item">
          <label class="form-item__label">审核管理员账号</label>
          <input
            type="text"
            class="form-item__input"
            v-model="form.auditAdmin"
            placeholder="请输入审核管理员账号"
          />
        </div>
        <div v-if="selectedRole === 'ADMIN'" class="form-item">
          <label class="form-item__label">审核管理员密码</label>
          <input
            type="password"
            class="form-item__input"
            v-model="form.auditPassword"
            placeholder="请输入审核管理员密码"
          />
        </div>

        <!-- 注册按钮 -->
        <Button type="primary" size="large" class="submit-btn" @click="handleRegister">
          注册
        </Button>

        <!-- 登录链接 -->
        <div class="link-container">
          已有账号？<router-link to="/login" class="link">立即登录</router-link>
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
// 获取全局挂载的axios
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
  account: '', // 账号（必填）
  phone: '', // 手机号（必填）
  email: '', // 邮箱（必填）
  password: '', // 密码（必填）
  confirmPwd: '', // 确认密码（必填）
  nickname: '', // 昵称（选填）
  school: '', // 学校（学生/教师必填）
  grade: '', // 年级（学生必填）
  realName: '', // 真实姓名（学生/教师必填）
  auditAdmin: '', // 审核管理员账号（管理员必填）
  auditPassword: '', // 审核管理员密码（管理员必填）
})

// ========== 表单校验规则 ==========
/**
 * 表单校验函数
 * 包含基础字段校验、格式校验、角色专属字段校验
 * @returns {Boolean} 校验结果：true-通过，false-失败
 */
const validateForm = () => {
  // 1. 基础字段非空校验
  if (!form.value.account.trim()) {
    warning('账号不能为空！')
    return false
  }
  if (!form.value.phone.trim()) {
    warning('手机号不能为空！')
    return false
  }
  if (!form.value.email.trim()) {
    warning('邮箱不能为空！')
    return false
  }
  if (!form.value.password.trim()) {
    warning('密码不能为空！')
    return false
  }

  // 2. 格式校验
  // 手机号格式校验
  const phoneReg = /^1[3-9]\d{9}$/
  if (!phoneReg.test(form.value.phone.trim())) {
    warning('请输入正确的手机号码！')
    return false
  }
  // 邮箱格式校验
  const emailReg = /^[a-zA-Z0-9._%-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,6}$/
  if (!emailReg.test(form.value.email.trim())) {
    warning('请输入正确的电子邮箱！')
    return false
  }
  // 密码长度校验
  if (form.value.password.length < 6) {
    warning('密码长度不能少于6位！')
    return false
  }
  // 密码一致性校验
  if (form.value.password !== form.value.confirmPwd) {
    warning('两次输入的密码不一致！')
    return false
  }

  // 3. 角色专属字段校验
  if (selectedRole.value === 'STUDENT') {
    if (!form.value.school.trim()) {
      warning('学生角色必须填写学校名称！')
      return false
    }
    if (!form.value.grade || form.value.grade < 2022 || form.value.grade > 9999) {
      warning('请输入有效的入学年级（2022-9999）！')
      return false
    }
    if (!form.value.realName.trim()) {
      warning('学生角色必须填写真实姓名！')
      return false
    }
  }
  if (selectedRole.value === 'TEACHER') {
    if (!form.value.school.trim()) {
      warning('教师角色必须填写学校名称！')
      return false
    }
    if (!form.value.realName.trim()) {
      warning('教师角色必须填写真实姓名！')
      return false
    }
  }
  if (selectedRole.value === 'ADMIN') {
    if (!form.value.auditAdmin.trim()) {
      warning('管理员角色必须填写审核管理员账号！')
      return false
    }
    if (!form.value.auditPassword.trim()) {
      warning('管理员角色必须填写审核管理员密码！')
      return false
    }
  }

  return true
}

// ========== 注册核心逻辑 ==========
/**
 * 注册处理函数
 * 仅在用户主动点击注册按钮时触发
 */
const handleRegister = async () => {
  // 1. 前端表单校验
  if (!validateForm()) {
    return
  }

  try {
    // 2. 构造请求参数
    const reqData = {
      role: selectedRole.value.toLowerCase(), // 角色转小写
      account: form.value.account.trim(),
      phone: form.value.phone.trim(),
      email: form.value.email.trim(),
      password: form.value.password.trim(),
      nickname: form.value.nickname.trim() || '', // 昵称选填
    }

    // 3. 补充角色专属字段
    if (selectedRole.value === 'STUDENT') {
      reqData.school = form.value.school.trim()
      reqData.grade = form.value.grade
      reqData.real_name = form.value.realName.trim()
    }
    if (selectedRole.value === 'TEACHER') {
      reqData.school = form.value.school.trim()
      reqData.real_name = form.value.realName.trim()
    }
    if (selectedRole.value === 'ADMIN') {
      reqData.auditAdmin = form.value.auditAdmin.trim()
      reqData.auditPassword = form.value.auditPassword.trim()
    }

    // 4. 发送注册请求
    const res = await axios.post('/register', reqData)

    // 5. 处理后端响应
    if (res.data && res.data.code === 0) {
      // 注册成功
      success('注册成功，正在跳转...')
      // 存储登录状态
      localStorage.setItem('token', res.data.data.token || 'mock-token')
      localStorage.setItem('userInfo', JSON.stringify(res.data.data))
      // 延迟跳转
      setTimeout(() => {
        router.push('/discussion')
      }, 1500)
    } else {
      // 后端返回业务失败
      notifyError(res.data?.msg || '注册失败，请稍后重试')
    }
  } catch (error) {
    // 6. 捕获错误信息
    notifyError('注册失败：请检查后端服务是否启动或网络是否正常！')
  }
}
</script>

<style scoped>
/* 注册页面容器 */
.register-page {
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

/* 提交按钮样式 */
.submit-btn {
  width: 100%;
  margin-top: 20px;
  margin-bottom: 15px;
  height: 48px;
  font-size: 16px;
  font-weight: 600;
  border-radius: 8px;
  transition: all 0.3s ease;
  box-shadow: 0 2px 4px rgba(22, 93, 255, 0.1);

  &:hover {
    box-shadow: 0 4px 8px rgba(22, 93, 255, 0.2);
    transform: translateY(-1px);
  }

  &:active {
    transform: translateY(0);
  }
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
