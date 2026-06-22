<!-- src/views/users/Index.vue -->
<template>
  <MainLayout>
    <div class="users-page container">
      <h1>用户管理</h1>

      <!-- 筛选和搜索表单 -->
      <div class="filter-form">
        <div class="form-row">
          <div class="form-item">
            <label>用户ID:</label>
            <input type="number" v-model="filter.userId" placeholder="输入用户ID" />
          </div>
          <div class="form-item">
            <label>用户类型:</label>
            <select v-model="filter.role">
              <option value="">全部</option>
              <option value="student">学生</option>
              <option value="teacher">教师</option>
              <option value="admin">管理员</option>
            </select>
          </div>
          <div class="form-item">
            <label>搜索:</label>
            <input type="text" v-model="filter.keyword" placeholder="搜索用户名或账号" />
          </div>
          <div class="form-item button-item">
            <button class="search-btn" @click="loadUsers">搜索</button>
            <button class="reset-btn" @click="resetFilter">重置</button>
          </div>
        </div>
      </div>

      <div v-if="loading" class="loading">加载中...</div>
      <div v-else-if="error" class="error">{{ error }}</div>
      <div v-else class="users-container">
        <div class="users-list">
          <div v-for="user in users" :key="user.id" class="user-item">
            <div class="user-avatar">
              <span class="avatar-text">{{ getAvatarText(user.nickname || user.account) }}</span>
            </div>
            <div class="user-info" @click="navigateToProfile(user.id)">
              <div class="user-name">{{ user.nickname || user.account }}</div>
              <div class="user-role">{{ getUserRoleText(user.role) }}</div>
            </div>
            <div class="user-account">{{ user.account }}</div>
            <div class="user-actions">
              <button class="action-btn" @click="openChangePasswordDialog(user)">修改密码</button>
            </div>
          </div>
        </div>
      </div>

      <!-- 修改密码对话框 -->
      <div
        v-if="showChangePasswordDialog"
        class="dialog-overlay"
        @click="closeChangePasswordDialog"
      >
        <div class="dialog-content" @click.stop>
          <div class="dialog-header">
            <h2>修改密码</h2>
            <button class="close-btn" @click="closeChangePasswordDialog">×</button>
          </div>
          <div class="dialog-body">
            <div class="form-group">
              <label>用户: {{ selectedUser?.nickname || selectedUser?.account }}</label>
            </div>
            <div class="form-group">
              <label>管理员密码:</label>
              <input type="password" v-model="adminPassword" placeholder="输入管理员密码" />
            </div>
            <div class="form-group">
              <label>新密码:</label>
              <input type="password" v-model="newPassword" placeholder="输入新密码" />
            </div>
            <div class="form-group">
              <label>确认新密码:</label>
              <input type="password" v-model="confirmPassword" placeholder="确认新密码" />
            </div>
            <div
              class="message-container"
              v-if="passwordError"
              :class="{ 'success-message': passwordError.includes('密码修改成功') }"
              v-html="passwordError"
            ></div>
          </div>
          <div class="dialog-footer">
            <button class="btn cancel-btn" @click="closeChangePasswordDialog">取消</button>
            <button class="btn confirm-btn" @click="changePassword">确认修改</button>
          </div>
        </div>
      </div>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import { apiService } from '@/services/api'

const router = useRouter()
const users = ref([])
const loading = ref(true)
const error = ref(null)

// 筛选和搜索
const filter = ref({
  userId: '',
  role: '',
  keyword: '',
})

// 修改密码相关
const showChangePasswordDialog = ref(false)
const selectedUser = ref(null)
const adminPassword = ref('')
const newPassword = ref('')
const confirmPassword = ref('')
const passwordError = ref('')

// 加载用户列表
const loadUsers = async () => {
  try {
    loading.value = true
    error.value = null

    const response = await apiService.getUsers()
    if (response.code === 0) {
      let filteredUsers = response.data

      // 应用筛选
      if (filter.value.userId) {
        filteredUsers = filteredUsers.filter((user) => user.id === parseInt(filter.value.userId))
      }

      if (filter.value.role) {
        filteredUsers = filteredUsers.filter((user) => user.role === filter.value.role)
      }

      if (filter.value.keyword) {
        const keyword = filter.value.keyword.toLowerCase()
        filteredUsers = filteredUsers.filter(
          (user) =>
            (user.nickname && user.nickname.toLowerCase().includes(keyword)) ||
            (user.account && user.account.toLowerCase().includes(keyword)),
        )
      }

      users.value = filteredUsers
    } else {
      error.value = response.msg
    }
  } catch (err) {
    error.value = '获取用户列表失败'
  } finally {
    loading.value = false
  }
}

// 重置筛选
const resetFilter = () => {
  filter.value = {
    userId: '',
    role: '',
    keyword: '',
  }
  loadUsers()
}

// 导航到用户主页
const navigateToProfile = (userId) => {
  router.push(`/profile/${userId}`)
}

// 打开修改密码对话框
const openChangePasswordDialog = (user) => {
  selectedUser.value = user
  adminPassword.value = ''
  newPassword.value = ''
  confirmPassword.value = ''
  passwordError.value = ''
  showChangePasswordDialog.value = true
}

// 关闭修改密码对话框
const closeChangePasswordDialog = () => {
  showChangePasswordDialog.value = false
  selectedUser.value = null
  adminPassword.value = ''
  newPassword.value = ''
  confirmPassword.value = ''
  passwordError.value = ''
}

// 修改密码
const changePassword = async () => {
  try {
    passwordError.value = ''

    if (!adminPassword.value) {
      passwordError.value = '请输入管理员密码'
      return
    }

    if (!newPassword.value) {
      passwordError.value = '请输入新密码'
      return
    }

    if (newPassword.value !== confirmPassword.value) {
      passwordError.value = '两次输入的密码不一致'
      return
    }

    const response = await apiService.changePassword(
      adminPassword.value,
      newPassword.value,
      selectedUser.value.id,
    )

    if (response.code === 0) {
      passwordError.value = '<span style="color: #52c41a;">密码修改成功</span>'
      setTimeout(() => {
        closeChangePasswordDialog()
      }, 1500)
    } else {
      passwordError.value = response.msg || '修改密码失败'
    }
  } catch (err) {
    passwordError.value = '网络错误，请重试'
  }
}

// 辅助函数
const getAvatarText = (name) => {
  if (!name) return '?'
  return name.charAt(0).toUpperCase()
}

const getUserRoleText = (role) => {
  const roleMap = {
    student: '学生',
    teacher: '教师',
    admin: '管理员',
  }
  return roleMap[role] || role
}

// 生命周期
onMounted(() => {
  loadUsers()
})
</script>

<style scoped>
.users-page {
  padding: 40px 0;
  min-height: 80vh;
}

.users-page h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 30px;
}

.loading,
.error {
  text-align: center;
  padding: 100px 0;
  font-size: 16px;
  color: #666;
}

.error {
  color: #f56c6c;
}

.users-container {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 24px;
}

.users-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.user-item {
  display: flex;
  align-items: center;
  padding: 16px;
  border-radius: 8px;
  transition: all 0.3s ease;
  cursor: pointer;
  border: 1px solid #f2f3f5;
}

.user-item:hover {
  background: #f7f8fa;
  border-color: #165dff;
  transform: translateX(4px);
}

.user-avatar {
  width: 48px;
  height: 48px;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
  margin-right: 20px;
}

.avatar-text {
  font-size: 20px;
  font-weight: 600;
  color: white;
}

.user-info {
  flex: 1;
  min-width: 0;
}

.user-name {
  font-size: 16px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 4px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.user-role {
  font-size: 12px;
  color: #86909c;
}

.user-account {
  font-size: 14px;
  color: #4e5969;
  flex-shrink: 0;
  margin-left: 20px;
  white-space: nowrap;
}

.user-actions {
  margin-left: 20px;
  flex-shrink: 0;
}

.action-btn {
  padding: 6px 12px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 4px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.action-btn:hover {
  background-color: #0e42c2;
}

/* 筛选表单样式 */
.filter-form {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 20px;
  margin-bottom: 20px;
}

.form-row {
  display: flex;
  gap: 16px;
  align-items: flex-end;
}

.form-item {
  flex: 1;
  min-width: 0;
}

.form-item label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-item input,
.form-item select {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.form-item input:focus,
.form-item select:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.button-item {
  display: flex;
  gap: 8px;
}

.search-btn {
  padding: 8px 16px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.search-btn:hover {
  background-color: #0e42c2;
}

.reset-btn {
  padding: 8px 16px;
  background-color: #ffffff;
  color: #4e5969;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.reset-btn:hover {
  border-color: #165dff;
  color: #165dff;
}

/* 对话框样式 */
.dialog-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.dialog-content {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.15);
  width: 90%;
  max-width: 500px;
}

.dialog-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px;
  border-bottom: 1px solid #e5e6eb;
}

.dialog-header h2 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

.close-btn {
  background: none;
  border: none;
  font-size: 24px;
  color: #86909c;
  cursor: pointer;
  padding: 0;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 4px;
  transition: all 0.2s ease;
}

.close-btn:hover {
  background-color: #f7f8fa;
  color: #1d2129;
}

.dialog-body {
  padding: 20px;
}

.form-group {
  margin-bottom: 16px;
}

.form-group label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-group input {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.form-group input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.message-container {
  font-size: 14px;
  margin-bottom: 16px;
  padding: 8px 12px;
  border-radius: 6px;
  border-left: 4px solid;
}

.message-container:not(.success-message) {
  color: #f53f3f;
  background-color: #fce4ec;
  border-left-color: #f53f3f;
}

.success-message {
  color: #52c41a;
  background-color: #f6ffed;
  border-left-color: #52c41a;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  padding: 20px;
  border-top: 1px solid #e5e6eb;
  background-color: #f7f8fa;
  border-radius: 0 0 8px 8px;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.cancel-btn {
  background-color: #ffffff;
  color: #4e5969;
  border: 1px solid #e5e6eb;
}

.cancel-btn:hover {
  border-color: #165dff;
  color: #165dff;
}

.confirm-btn {
  background-color: #165dff;
  color: #ffffff;
}

.confirm-btn:hover {
  background-color: #0e42c2;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .form-row {
    flex-direction: column;
    align-items: stretch;
  }

  .user-item {
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }

  .user-account {
    margin-left: 0;
  }

  .user-actions {
    margin-left: 0;
    align-self: flex-start;
  }
}

/* 响应式设计 */
@media (max-width: 768px) {
  .users-page {
    padding: 20px 0;
  }

  .users-container {
    padding: 16px;
  }

  .user-item {
    padding: 12px;
  }

  .user-avatar {
    width: 40px;
    height: 40px;
    margin-right: 12px;
  }

  .avatar-text {
    font-size: 16px;
  }

  .user-name {
    font-size: 14px;
  }

  .user-account {
    font-size: 12px;
    margin-left: 12px;
  }
}
</style>
