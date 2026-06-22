<!-- src/views/person/Index.vue -->
<template>
  <MainLayout>
    <div class="person-page container">
      <h1>个人中心</h1>

      <!-- 加载状态 -->
      <div v-if="loading" class="loading">加载中...</div>

      <!-- 个人信息卡片 -->
      <div v-else class="info-card">
        <div class="card-header">
          <h2>基本信息</h2>
          <button class="edit-btn" @click="isEditing = !isEditing">
            {{ isEditing ? '取消' : '编辑' }}
          </button>
        </div>

        <div class="info-content">
          <div class="info-item">
            <span class="info-label">账号：</span>
            <span class="info-value">{{ userInfo.account }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">昵称：</span>
            <div v-if="isEditing" class="edit-field">
              <input
                type="text"
                v-model="editForm.nickname"
                class="form-input"
                placeholder="请输入昵称"
              />
              <button class="save-btn" @click="saveUserInfo">保存</button>
            </div>
            <span v-else class="info-value">{{ userInfo.nickname }}</span>
          </div>
          <div class="info-item">
            <span class="info-label">角色：</span>
            <span class="info-value">{{ getRoleLabel(userInfo.role) }}</span>
          </div>
        </div>
      </div>

      <!-- 编程统计卡片 -->
      <div class="stats-card">
        <h3>编程统计</h3>
        <div class="stats-grid">
          <div class="stat-item">
            <div class="stat-value">{{ recentSubmissionsCount || 0 }}</div>
            <div class="stat-label">近期提交次数</div>
          </div>
          <div class="stat-item">
            <div class="stat-value">{{ recentSolvedCount || 0 }}</div>
            <div class="stat-label">近期已解决题目</div>
          </div>
          <div class="stat-item">
            <div class="stat-value">{{ recentSolveRate }}%</div>
            <div class="stat-label">近期解题率</div>
          </div>
        </div>
        <div class="stats-tips">
          <div class="tips-content">
            <div class="left-tip">坚持练习，每一次提交都是成长的机会</div>
            <div class="right-tip">
              <span class="warning-icon">⚠</span>
              <span>解题率仅供参考，不能完全代表编程能力</span>
            </div>
          </div>
        </div>
      </div>

      <!-- 提交记录卡片 -->
      <div class="submission-card">
        <div class="card-header">
          <h2>提交活动</h2>
        </div>

        <!-- GitHub风格贡献日历 -->
        <div class="contribution-calendar">
          <!-- GitHub风格贡献日历 -->
          <div class="github-calendar">
            <!-- 日历网格 -->
            <div class="github-calendar-grid">
              <div
                v-for="(day, index) in calendarDays"
                :key="index"
                class="github-contribution-day"
                :class="[
                  getContributionLevel(day.date),
                  { 'edge-left': index % 7 === 0 },
                  { 'edge-right': (index + 1) % 7 === 0 },
                ]"
                @click="showSubmissionCard(day.date)"
              >
                <div class="github-tooltip" v-if="day.date">
                  <div class="tooltip-content">
                    <div class="tooltip-date">{{ formatDate(day.date) }}</div>
                    <div class="tooltip-count" v-if="getDaySubmissions(day.date).length > 0">
                      {{ getDaySubmissions(day.date).length }} 次提交
                    </div>
                    <div class="tooltip-count" v-else>0 次提交</div>
                    <div class="tooltip-list" v-if="getDaySubmissions(day.date).length > 0">
                      <div
                        class="tooltip-item"
                        v-for="(submission, subIndex) in getDaySubmissions(day.date).slice(0, 3)"
                        :key="subIndex"
                      >
                        <span class="tooltip-status" :class="submission.status.toLowerCase()">{{
                          submission.status
                        }}</span>
                        <span class="tooltip-problem">题目 {{ submission.question_id }}</span>
                      </div>
                      <div v-if="getDaySubmissions(day.date).length > 3" class="tooltip-more">
                        还有 {{ getDaySubmissions(day.date).length - 3 }} 次提交
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
          <div class="github-legend">
            <span class="legend-label">更少</span>
            <div class="legend-item level-0"></div>
            <div class="legend-item level-1"></div>
            <div class="legend-item level-2"></div>
            <div class="legend-item level-3"></div>
            <div class="legend-item level-4"></div>
            <span class="legend-label">更多</span>
          </div>
        </div>
      </div>

      <!-- 账号设置卡片 -->
      <div class="account-settings-card">
        <div class="card-header">
          <h2>账号设置</h2>
        </div>
        <div class="settings-content">
          <div class="settings-item">
            <span class="settings-label">修改密码</span>
            <button class="settings-btn" @click="showChangePasswordDialog = true">修改</button>
          </div>
          <div class="settings-item">
            <span class="settings-label">邮箱设置</span>
            <button class="settings-btn" @click="showEmailSettingsDialog = true">设置</button>
          </div>
          <div class="settings-item">
            <span class="settings-label">隐私设置</span>
            <button class="settings-btn" @click="openPrivacySettingsDialog">设置</button>
          </div>
        </div>
      </div>

      <!-- 修改密码弹窗 -->
      <CustomDialog
        v-model:visible="showChangePasswordDialog"
        title="修改密码"
        @close="showChangePasswordDialog = false"
      >
        <div class="dialog-content">
          <div class="form-item">
            <label class="form-label">当前密码</label>
            <input
              type="password"
              v-model="passwordForm.oldPassword"
              class="form-input"
              placeholder="请输入当前密码"
            />
          </div>
          <div class="form-item">
            <label class="form-label">新密码</label>
            <input
              type="password"
              v-model="passwordForm.newPassword"
              class="form-input"
              placeholder="请输入新密码"
            />
          </div>
          <div class="form-item">
            <label class="form-label">确认新密码</label>
            <input
              type="password"
              v-model="passwordForm.confirmPassword"
              class="form-input"
              placeholder="请确认新密码"
            />
          </div>
          <div class="form-actions">
            <button class="btn secondary" @click="showChangePasswordDialog = false">取消</button>
            <button class="btn primary" @click="changePassword">确认修改</button>
          </div>
        </div>
      </CustomDialog>

      <!-- 提交记录详情卡片 -->
      <div
        v-if="selectedDate && getDaySubmissions(selectedDate).length > 0"
        class="submission-detail-card"
      >
        <div class="card-header">
          <h2>{{ formatDate(selectedDate) }} 的提交记录</h2>
          <button class="close-btn" @click="selectedDate = null">×</button>
        </div>
        <div class="submission-detail-list">
          <div
            class="submission-detail-item"
            v-for="(submission, index) in getDaySubmissions(selectedDate)"
            :key="submission.id"
            @click="toggleSubmissionDetail(submission.id)"
          >
            <div class="submission-detail-info">
              <div class="submission-detail-title">题目 {{ submission.question_id }}</div>
              <div class="submission-detail-meta">
                <span class="language">{{ submission.language }}</span>
                <span class="time">{{ formatTime(submission.createdAt) }}</span>
              </div>
            </div>
            <div class="submission-detail-status" :class="submission.status.toLowerCase()">
              {{ submission.status }}
            </div>
            <!-- 展开的详情部分 -->
            <div v-if="expandedSubmissionId === submission.id" class="submission-detail-expanded">
              <div class="submission-detail-code">
                <div class="code-header">
                  <span>代码</span>
                  <button
                    class="copy-code-btn"
                    @click.stop="copyCode(submission.code)"
                    title="复制代码"
                  >
                    复制
                  </button>
                </div>
                <pre class="code-content">{{ submission.code || '无代码' }}</pre>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 角色特定功能 -->
      <div v-if="userInfo.role === 'ADMIN'" class="role-specific-card">
        <div class="card-header">
          <h2>管理员功能</h2>
        </div>
        <div class="admin-features">
          <router-link to="/problems/review" class="feature-item">
            <div class="feature-icon">📋</div>
            <div class="feature-text">题目审核</div>
          </router-link>
          <div class="feature-item" @click="getSystemStats">
            <div class="feature-icon">📊</div>
            <div class="feature-text">系统统计</div>
          </div>
        </div>
      </div>

      <div v-if="userInfo.role === 'TEACHER'" class="role-specific-card">
        <div class="card-header">
          <h2>教师功能</h2>
        </div>
        <div class="teacher-features">
          <router-link to="/problems/create" class="feature-item">
            <div class="feature-icon">➕</div>
            <div class="feature-text">创建题目</div>
          </router-link>
          <router-link to="/problems/review" class="feature-item">
            <div class="feature-icon">📋</div>
            <div class="feature-text">题目审核</div>
          </router-link>
        </div>
      </div>

      <div v-if="userInfo.role === 'STUDENT'" class="role-specific-card">
        <div class="card-header">
          <h2>学习中心</h2>
        </div>
        <div class="student-features">
          <router-link to="/training" class="feature-item">
            <div class="feature-icon">🏋️</div>
            <div class="feature-text">训练计划</div>
          </router-link>
          <router-link to="/analysis" class="feature-item">
            <div class="feature-icon">📈</div>
            <div class="feature-text">能力分析</div>
          </router-link>
        </div>
      </div>

      <!-- 邮箱设置弹窗 -->
      <CustomDialog
        v-model:visible="showEmailSettingsDialog"
        title="邮箱设置"
        @close="showEmailSettingsDialog = false"
      >
        <div class="dialog-content">
          <div class="form-item">
            <label class="form-label">邮箱地址</label>
            <input
              type="email"
              v-model="emailForm.email"
              placeholder="请输入邮箱地址"
              class="form-input"
            />
          </div>
          <div class="form-actions">
            <button class="btn secondary" @click="showEmailSettingsDialog = false">取消</button>
            <button class="btn primary" @click="saveEmailSettings">保存设置</button>
          </div>
        </div>
      </CustomDialog>

      <!-- 隐私设置弹窗 -->
      <CustomDialog
        v-model:visible="showPrivacySettingsDialog"
        title="隐私设置"
        @close="showPrivacySettingsDialog = false"
      >
        <div class="dialog-content">
          <div class="form-item">
            <label class="form-label">对外显示设置</label>
            <div class="privacy-options">
              <div class="privacy-option">
                <input type="checkbox" id="showRole" v-model="privacySettings.showRole" />
                <label for="showRole">显示用户角色</label>
              </div>
              <!-- 学生和教师显示学校设置 -->
              <div class="privacy-option" v-if="userInfo.role.toLowerCase() !== 'admin'">
                <input type="checkbox" id="showSchool" v-model="privacySettings.showSchool" />
                <label for="showSchool">显示学校</label>
              </div>
              <!-- 只有学生显示年级设置 -->
              <div class="privacy-option" v-if="userInfo.role.toLowerCase() === 'student'">
                <input type="checkbox" id="showGrade" v-model="privacySettings.showGrade" />
                <label for="showGrade">显示年级</label>
              </div>
              <div class="privacy-option">
                <input
                  type="checkbox"
                  id="showSubmissionsCount"
                  v-model="privacySettings.showSubmissionsCount"
                />
                <label for="showSubmissionsCount">显示提交总数</label>
              </div>
              <div class="privacy-option">
                <input
                  type="checkbox"
                  id="showSolvedCount"
                  v-model="privacySettings.showSolvedCount"
                />
                <label for="showSolvedCount">显示已解决题目数</label>
              </div>
              <div class="privacy-option">
                <input
                  type="checkbox"
                  id="showSubmissionStats"
                  v-model="privacySettings.showSubmissionStats"
                />
                <label for="showSubmissionStats">显示提交语言统计</label>
              </div>
              <!-- 只有学生显示学号设置 -->
              <div class="privacy-option" v-if="userInfo.role.toLowerCase() === 'student'">
                <input
                  type="checkbox"
                  id="showUserRoleNo"
                  v-model="privacySettings.showUserRoleNo"
                />
                <label for="showUserRoleNo">显示学号</label>
              </div>
              <!-- 只有学生和教师显示真实姓名设置 -->
              <div class="privacy-option" v-if="userInfo.role.toLowerCase() !== 'admin'">
                <input type="checkbox" id="showRealName" v-model="privacySettings.showRealName" />
                <label for="showRealName">显示真实姓名</label>
              </div>
            </div>
          </div>
          <div class="form-actions">
            <button class="btn secondary" @click="showPrivacySettingsDialog = false">取消</button>
            <button class="btn primary" @click="savePrivacySettings">保存设置</button>
          </div>
        </div>
      </CustomDialog>

      <!-- 系统统计弹窗 -->
      <CustomDialog v-model:visible="statsDialogVisible" title="系统统计">
        <div class="stats-content">
          <div class="stats-item">
            <span class="stats-label">总题目数：</span>
            <span class="stats-value">{{ systemStats.totalProblems || 0 }}</span>
          </div>
          <div class="stats-item">
            <span class="stats-label">支持语言数：</span>
            <span class="stats-value">{{ systemStats.supportedLanguages || 0 }}</span>
          </div>
          <div class="stats-item">
            <span class="stats-label">总提交数：</span>
            <span class="stats-value">{{ systemStats.totalSubmissions || 0 }}</span>
          </div>
          <div class="stats-item">
            <span class="stats-label">评测准确率：</span>
            <span class="stats-value">{{ (systemStats.judgeAccuracy || 0).toFixed(2) }}%</span>
          </div>
        </div>
      </CustomDialog>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import MainLayout from '@/components/Layout/MainLayout.vue'
import { apiService } from '@/services/api'
import CustomDialog from '@/components/Common/CustomDialog.vue'

// 响应式数据
const loading = ref(true)
const userInfo = ref({
  id: 0,
  account: '',
  nickname: '',
  role: '',
  token: '',
})
const submissions = ref({
  list: [],
  total: 0,
  page: 1,
  pageSize: 50,
})
const isEditing = ref(false)
const editForm = ref({ nickname: '' })
const statsDialogVisible = ref(false)
const systemStats = ref({})

// 贡献日历相关
const months = [
  '1月',
  '2月',
  '3月',
  '4月',
  '5月',
  '6月',
  '7月',
  '8月',
  '9月',
  '10月',
  '11月',
  '12月',
]
const calendarDays = ref([])
const selectedDate = ref(null)
const expandedSubmissionId = ref(null)

// 账号设置相关
const showChangePasswordDialog = ref(false)
const showEmailSettingsDialog = ref(false)
const showPrivacySettingsDialog = ref(false)

// 打开隐私设置弹窗
const openPrivacySettingsDialog = async () => {
  await loadPrivacySettings()
  showPrivacySettingsDialog.value = true
}
const passwordForm = ref({
  oldPassword: '',
  newPassword: '',
  confirmPassword: '',
})

// 隐私设置相关
const privacySettings = ref({
  showRole: true,
  showSchool: true,
  showGrade: true,
  showSubmissionsCount: true,
  showSolvedCount: true,
  showSubmissionStats: true,
  showUserRoleNo: false,
  showRealName: false,
})

// 角色标签映射
const roleLabels = {
  ADMIN: '管理员',
  STUDENT: '学生',
  TEACHER: '教师',
}

// 计算属性
const getRoleLabel = (role) => {
  return roleLabels[role] || role
}

const solvedCount = computed(() => {
  // 从提交记录中计算已解决的题目数
  if (!submissions.value.list) return 0
  const solvedProblems = new Set()
  submissions.value.list.forEach((submission) => {
    if (submission.status === 'AC') {
      solvedProblems.add(submission.question_id)
    }
  })
  return solvedProblems.size
})

const solveRate = computed(() => {
  if (!submissions.value.total || submissions.value.total === 0) return 0
  return Math.round((solvedCount.value / submissions.value.total) * 100)
})

// 获取近期（最近6个月）的统计数据
const getRecentStats = () => {
  const today = new Date()
  today.setHours(0, 0, 0, 0)
  const sixMonthsAgo = new Date(today)
  sixMonthsAgo.setMonth(sixMonthsAgo.getMonth() - 6)

  let count = 0
  const solvedProblems = new Set()

  submissions.value.list.forEach((submission) => {
    if (!submission.createdAt) return

    let dateStr = submission.createdAt
    if (dateStr.includes(' ')) {
      dateStr = dateStr.replace(' ', 'T')
    }
    const subDate = new Date(dateStr)
    subDate.setHours(0, 0, 0, 0)

    if (subDate >= sixMonthsAgo) {
      count++
      if (submission.status === 'AC') {
        solvedProblems.add(submission.question_id)
      }
    }
  })

  return {
    count,
    solvedCount: solvedProblems.size,
  }
}

const recentSubmissionsCount = computed(() => {
  return getRecentStats().count
})

const recentSolvedCount = computed(() => {
  return getRecentStats().solvedCount
})

const recentSolveRate = computed(() => {
  const stats = getRecentStats()
  if (stats.count === 0) return 0
  return Math.round((stats.solvedCount / stats.count) * 100)
})

// 切换提交记录详情展开/收起
const toggleSubmissionDetail = (submissionId) => {
  if (expandedSubmissionId.value === submissionId) {
    expandedSubmissionId.value = null
  } else {
    expandedSubmissionId.value = submissionId
  }
}

// 复制代码到剪贴板
const copyCode = async (code) => {
  if (!code) return

  try {
    await navigator.clipboard.writeText(code)
  } catch (error) {
    // 复制代码失败
  }
}

// 方法
const loadUserInfo = async (forceRefresh = false) => {
  try {
    if (!forceRefresh) {
      // 首先尝试从本地存储加载用户信息
      const cachedUserInfo = localStorage.getItem('userInfo')
      if (cachedUserInfo) {
        const parsedUserInfo = JSON.parse(cachedUserInfo)
        userInfo.value = parsedUserInfo
        editForm.value.nickname = parsedUserInfo.nickname
        loading.value = false
        return
      }
    }

    // 从API请求获取最新的用户信息
    const response = await apiService.getUserInfo()
    if (response.code === 0) {
      userInfo.value = response.data
      editForm.value.nickname = response.data.nickname
      // 更新本地存储
      localStorage.setItem('userInfo', JSON.stringify(response.data))
    }
  } catch (error) {
    // 获取用户信息失败
  } finally {
    loading.value = false
  }
}

const loadSubmissions = async (page = 1) => {
  try {
    // 如果是加载第一页，执行智能获取策略
    if (page === 1) {
      await loadSubmissionsSmart()
    } else {
      // 后续页面使用正常逻辑
      const response = await apiService.getUserSubmissions(page, submissions.value.pageSize)
      if (response.data && response.data.list) {
        processSubmissionsList(response.data.list)
        submissions.value = response.data
      }
    }
  } catch (error) {
    // 获取提交记录失败
  }
}

// 智能获取提交记录策略：确保覆盖最近7天
const loadSubmissionsSmart = async () => {
  const targetDays = 7 // 目标覆盖最近7天
  const minBatchSize = 30 // 每批获取最小数量
  const allSubmissions = []
  let currentPage = 1
  let hasMore = true
  let totalLoaded = 0

  while (hasMore && currentPage <= 10) {
    const response = await apiService.getUserSubmissions(currentPage, minBatchSize)
    if (!response.data || !response.data.list || response.data.list.length === 0) {
      hasMore = false
      break
    }

    const list = response.data.list.map((item) => {
      if (typeof item === 'string') {
        try {
          return JSON.parse(item)
        } catch (e) {
          return item
        }
      }
      return item
    })

    allSubmissions.push(...list)
    totalLoaded += list.length

    // 分析日期分布，检查是否覆盖了最近7天
    const today = new Date()
    today.setHours(0, 0, 0, 0)
    const sevenDaysAgo = new Date(today)
    sevenDaysAgo.setDate(sevenDaysAgo.getDate() - targetDays)

    const hasRecentSubmissions = list.some((sub) => {
      if (!sub.createdAt) return false
      const subDate = new Date(sub.createdAt)
      return subDate >= sevenDaysAgo
    })

    // 如果当前批有最近7天的记录，说明数据足够
    if (hasRecentSubmissions) {
      hasMore = false
      break
    }

    // 如果已加载的数据足够多（超过7天*每天预估30次），也停止
    if (totalLoaded >= targetDays * 30) {
      hasMore = false
      break
    }

    // 如果数据不够，继续加载下一页
    if (list.length < minBatchSize) {
      hasMore = false
      break
    }

    currentPage++
  }

  // 更新submissions
  submissions.value.list = allSubmissions
  submissions.value.total = totalLoaded
  submissions.value.page = 1
}

// 处理提交记录列表
const processSubmissionsList = (list) => {
  if (!Array.isArray(list)) return
  list.forEach((item, index) => {
    if (typeof item === 'string') {
      try {
        list[index] = JSON.parse(item)
      } catch (e) {
        // 保持原样
      }
    }
  })
}

const saveUserInfo = async () => {
  try {
    const response = await apiService.updateNickname(editForm.value.nickname)
    if (response.code === 0) {
      isEditing.value = false
      // 直接更新响应式数据和缓存，不需要重新请求
      userInfo.value.nickname = editForm.value.nickname
      // 更新localStorage
      localStorage.setItem('userInfo', JSON.stringify(userInfo.value))
      // 触发自定义事件，通知Header组件更新
      const event = new CustomEvent('user-info-updated', {
        detail: { userInfo: userInfo.value },
      })
      window.dispatchEvent(event)
    }
  } catch (error) {
    // 更新用户信息失败
  }
}

const changePage = (page) => {
  loadSubmissions(page)
}

const getSystemStats = async () => {
  try {
    const response = await apiService.getStats()
    if (response.code === 0) {
      systemStats.value = response.data
      statsDialogVisible.value = true
    }
  } catch (error) {
    // 获取系统统计失败
  }
}

const formatTime = (timeString) => {
  const date = new Date(timeString)
  return date.toLocaleString('zh-CN')
}

const formatDate = (dateString) => {
  const date = new Date(dateString)
  return date.toLocaleDateString('zh-CN')
}

// 生成GitHub风格的日历数据（根据实际提交动态调整）
const generateCalendarDays = () => {
  const days = []
  const today = new Date()
  today.setHours(0, 0, 0, 0)

  // 第一步：分析提交记录的日期范围
  const submissionsByDate = processSubmissionsByDate()
  const submissionDates = Object.keys(submissionsByDate)

  // 确定最后日期：始终使用今天，确保日历更新到最新日期
  let endDate = new Date(today)

  // 确定最早日期：最远6个月前
  let startDate = new Date(endDate)
  startDate.setMonth(startDate.getMonth() - 6)

  // 如果有更早的提交记录，检查是否需要更早的日期
  if (submissionDates.length > 0) {
    const firstSubmissionDate = new Date(
      Math.min(...submissionDates.map((d) => new Date(d).getTime())),
    )
    firstSubmissionDate.setHours(0, 0, 0, 0)
    // 如果最早提交比6个月前还早，就用最早提交作为起始
    if (firstSubmissionDate < startDate) {
      startDate = new Date(firstSubmissionDate)
    }
  }

  // 计算需要显示的总天数
  const diffTime = endDate - startDate
  const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24)) + 1
  // 确保最少显示30天，最多显示180天（6个月）
  const totalDays = Math.max(30, Math.min(180, diffDays))

  // 重新计算开始日期，确保日历对齐完整
  const adjustedStartDate = new Date(endDate)
  adjustedStartDate.setDate(adjustedStartDate.getDate() - totalDays + 1)

  // 生成日历数据
  let currentDate = new Date(adjustedStartDate)
  for (let i = 0; i < totalDays; i++) {
    const dateStr = currentDate.toISOString().split('T')[0]
    days.push({ date: dateStr, submissions: [] })
    currentDate.setDate(currentDate.getDate() + 1)
  }

  calendarDays.value = days
}

// 动态更新提示框位置
const updateTooltipPosition = (event) => {
  const target = event.currentTarget
  const tooltip = target.querySelector('.github-tooltip')
  if (!tooltip) return

  // 先显示提示框，以便获取其尺寸
  tooltip.style.display = 'block'

  const rect = target.getBoundingClientRect()
  const tooltipRect = tooltip.getBoundingClientRect()
  const containerRect = document.querySelector('.contribution-calendar').getBoundingClientRect()

  // 计算提示框位置
  let left = rect.left + rect.width / 2 - tooltipRect.width / 2

  // 调整左右边界
  if (left < containerRect.left) {
    left = containerRect.left + 10
  } else if (left + tooltipRect.width > containerRect.right) {
    left = containerRect.right - tooltipRect.width - 10
  }

  // 设置提示框位置
  tooltip.style.left = `${left - containerRect.left}px`
  tooltip.style.top = `${rect.top - containerRect.top - tooltipRect.height - 8}px`
}

// 处理提交记录，按日期分组
const processSubmissionsByDate = () => {
  const submissionsByDate = {}

  submissions.value.list.forEach((submission) => {
    // 处理不同格式的日期字符串
    let date
    if (submission.createdAt) {
      if (submission.createdAt.includes('T')) {
        // ISO格式: 2026-04-01T13:57:33
        date = submission.createdAt.split('T')[0]
      } else if (submission.createdAt.includes(' ')) {
        // 空格分隔格式: 2026-04-01 13:57:33
        date = submission.createdAt.split(' ')[0]
      } else {
        date = submission.createdAt
      }
    }

    if (date) {
      if (!submissionsByDate[date]) {
        submissionsByDate[date] = []
      }
      submissionsByDate[date].push(submission)
    }
  })

  return submissionsByDate
}

// 获取某天的提交记录
const getDaySubmissions = (date) => {
  if (!date) return []
  const submissionsByDate = processSubmissionsByDate()
  return submissionsByDate[date] || []
}

// 获取贡献等级
const getContributionLevel = (date) => {
  if (!date) return 'level-0'
  const daySubmissions = getDaySubmissions(date)
  const count = daySubmissions.length

  if (count === 0) return 'level-0'
  if (count < 3) return 'level-1'
  if (count < 5) return 'level-2'
  if (count < 10) return 'level-3'
  return 'level-4'
}

// 显示提交详情卡片
const showSubmissionCard = (date) => {
  selectedDate.value = date
}

// 修改密码
const changePassword = async () => {
  try {
    const response = await apiService.changePassword(
      passwordForm.value.oldPassword,
      passwordForm.value.newPassword,
    )
    if (response.code === 0) {
      showChangePasswordDialog.value = false
      passwordForm.value = {
        oldPassword: '',
        newPassword: '',
        confirmPassword: '',
      }
    }
  } catch (error) {
    // 修改密码失败
  }
}

// 加载隐私设置
const loadPrivacySettings = async () => {
  try {
    // 确保userInfo已加载
    if (!userInfo.value || !userInfo.value.role) {
      console.warn('User info not loaded yet, skipping privacy settings load')
      return
    }

    // 首先尝试从本地存储加载隐私设置
    const cachedPrivacySettings = localStorage.getItem('privacySettings')
    if (cachedPrivacySettings) {
      try {
        const parsed = JSON.parse(cachedPrivacySettings)
        // 根据角色设置默认值
        const defaults = {
          showRole: true,
          showSchool: userInfo.value.role.toLowerCase() !== 'admin',
          showGrade: userInfo.value.role.toLowerCase() === 'student',
          showSubmissionsCount: true,
          showSolvedCount: true,
          showSubmissionStats: true,
          showUserRoleNo: userInfo.value.role.toLowerCase() === 'student',
          showRealName: userInfo.value.role.toLowerCase() !== 'admin',
        }

        // 合并到默认值，确保所有字段都存在
        privacySettings.value = {
          showRole: parsed.showRole !== undefined ? parsed.showRole : defaults.showRole,
          showSchool: parsed.showSchool !== undefined ? parsed.showSchool : defaults.showSchool,
          showGrade: parsed.showGrade !== undefined ? parsed.showGrade : defaults.showGrade,
          showSubmissionsCount:
            parsed.showSubmissionsCount !== undefined
              ? parsed.showSubmissionsCount
              : defaults.showSubmissionsCount,
          showSolvedCount:
            parsed.showSolvedCount !== undefined
              ? parsed.showSolvedCount
              : defaults.showSolvedCount,
          showSubmissionStats:
            parsed.showSubmissionStats !== undefined
              ? parsed.showSubmissionStats
              : defaults.showSubmissionStats,
          showUserRoleNo:
            parsed.showUserRoleNo !== undefined ? parsed.showUserRoleNo : defaults.showUserRoleNo,
          showRealName:
            parsed.showRealName !== undefined ? parsed.showRealName : defaults.showRealName,
        }
      } catch (e) {
        console.error('Failed to parse cached privacy settings:', e)
      }
    }

    // 从API请求获取最新的隐私设置
    const response = await apiService.getPrivacySettings()
    if (response.code === 0 && response.data) {
      // 根据角色设置默认值
      const defaults = {
        showRole: true,
        showSchool: userInfo.value.role.toLowerCase() !== 'admin',
        showGrade: userInfo.value.role.toLowerCase() === 'student',
        showSubmissionsCount: true,
        showSolvedCount: true,
        showSubmissionStats: true,
        showUserRoleNo: userInfo.value.role.toLowerCase() === 'student',
        showRealName: userInfo.value.role.toLowerCase() !== 'admin',
      }

      // 合并API返回的数据，确保所有字段都存在
      privacySettings.value = {
        showRole: response.data.showRole !== undefined ? response.data.showRole : defaults.showRole,
        showSchool:
          response.data.showSchool !== undefined ? response.data.showSchool : defaults.showSchool,
        showGrade:
          response.data.showGrade !== undefined ? response.data.showGrade : defaults.showGrade,
        showSubmissionsCount:
          response.data.showSubmissionsCount !== undefined
            ? response.data.showSubmissionsCount
            : defaults.showSubmissionsCount,
        showSolvedCount:
          response.data.showSolvedCount !== undefined
            ? response.data.showSolvedCount
            : defaults.showSolvedCount,
        showSubmissionStats:
          response.data.showSubmissionStats !== undefined
            ? response.data.showSubmissionStats
            : defaults.showSubmissionStats,
        showUserRoleNo:
          response.data.showUserRoleNo !== undefined
            ? response.data.showUserRoleNo
            : defaults.showUserRoleNo,
        showRealName:
          response.data.showRealName !== undefined
            ? response.data.showRealName
            : defaults.showRealName,
      }
      // 更新本地存储
      localStorage.setItem('privacySettings', JSON.stringify(privacySettings.value))
    }
  } catch (error) {
    console.error('Failed to load privacy settings:', error)
  }
}

// 保存隐私设置
const savePrivacySettings = async () => {
  try {
    const response = await apiService.updatePrivacySettings(privacySettings.value)
    if (response.code === 0) {
      // 更新本地存储
      localStorage.setItem('privacySettings', JSON.stringify(privacySettings.value))
      showPrivacySettingsDialog.value = false
    }
  } catch (error) {
    // 保存隐私设置失败
  }
}

// 邮箱设置
const emailForm = ref({
  email: '',
})

const saveEmailSettings = async () => {
  try {
    const response = await apiService.setEmail(emailForm.value.email)
    if (response.code === 0) {
      showEmailSettingsDialog.value = false
      emailForm.value.email = ''
    }
  } catch (error) {
    // 设置邮箱失败
  }
}

// 生命周期
onMounted(async () => {
  await loadUserInfo()
  await loadSubmissions()
  generateCalendarDays()
  loadPrivacySettings()
})
</script>

<style scoped>
.person-page {
  padding: 40px 0;
}

.person-page h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 30px;
}

.loading {
  text-align: center;
  padding: 40px;
  color: #86909c;
}

/* 卡片样式 */
.info-card,
.submission-card,
.role-specific-card {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
  margin-bottom: 24px;
}

.submission-card {
  overflow: visible;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 24px;
  border-bottom: 1px solid #f0f2f5;
  background-color: #f7f8fa;
}

.card-header h2 {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

/* 编程统计卡片 */
.stats-card {
  background: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
  padding: 24px;
  margin-bottom: 24px;
  transition: all 0.3s ease;
}

.stats-card:hover {
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.15);
}

.stats-card h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 20px 0;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
}

.stat-item {
  text-align: center;
  padding: 20px;
  background: #f7f8fa;
  border-radius: 8px;
  transition: all 0.3s ease;
}

.stat-item:hover {
  background: #ecf5ff;
  transform: translateY(-2px);
}

.stat-value {
  font-size: 28px;
  font-weight: 600;
  color: #409eff;
  margin-bottom: 8px;
}

.stat-label {
  font-size: 14px;
  color: #86909c;
}

/* 统计提示文字 */
.stats-tips {
  margin-top: 20px;
  padding-top: 16px;
  border-top: 1px solid #f0f2f5;
}

.tips-content {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.left-tip {
  display: flex;
  align-items: center;
  font-size: 12px;
  color: #c9cdd4;
}

.warning-icon {
  margin-right: 8px;
  opacity: 0.7;
}

.right-tip {
  font-size: 12px;
  color: #c9cdd4;
  text-align: right;
}

.edit-btn {
  padding: 6px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  background-color: #ffffff;
  color: #165dff;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.edit-btn:hover {
  background-color: #e8f3ff;
}

/* 信息内容 */
.info-content {
  padding: 24px;
}

.info-item {
  display: flex;
  align-items: center;
  margin-bottom: 16px;
}

.info-item:last-child {
  margin-bottom: 0;
}

.info-label {
  width: 80px;
  font-size: 14px;
  color: #4e5969;
  font-weight: 500;
}

.info-value {
  font-size: 14px;
  color: #1d2129;
}

/* 编辑字段 */
.edit-field {
  display: flex;
  align-items: center;
  gap: 12px;
  flex: 1;
}

.form-input {
  flex: 1;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #1d2129;
}

.form-input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.save-btn {
  padding: 6px 16px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.save-btn:hover {
  background-color: #0e42c5;
}

/* 提交记录 */
.submission-list {
  padding: 24px;
}

.empty-state {
  text-align: center;
  padding: 40px;
  color: #86909c;
  font-size: 14px;
}

.submission-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border: 1px solid #f0f2f5;
  border-radius: 8px;
  margin-bottom: 12px;
  transition: all 0.2s ease;
}

.submission-item:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(22, 93, 255, 0.1);
}

.submission-title {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 4px;
}

.submission-meta {
  display: flex;
  gap: 16px;
  font-size: 12px;
  color: #86909c;
}

.submission-status {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.submission-status.accepted {
  background-color: #e8f5e9;
  color: #2e7d32;
}

.submission-status.wrong_answer {
  background-color: #ffebee;
  color: #c62828;
}

.submission-status.time_limit_exceeded {
  background-color: #fff3e0;
  color: #e65100;
}

.submission-status.runtime_error {
  background-color: #f3e5f5;
  color: #7b1fa2;
}

.submission-status.error {
  background-color: #ffebee;
  color: #c62828;
}

/* 分页 */
.pagination {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: 16px;
  padding: 20px;
  border-top: 1px solid #f0f2f5;
}

.page-btn {
  padding: 6px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  background-color: #ffffff;
  color: #1d2129;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.page-btn:hover:not(:disabled) {
  border-color: #165dff;
  color: #165dff;
}

.page-btn:disabled {
  color: #c9cdd4;
  cursor: not-allowed;
}

.page-info {
  font-size: 14px;
  color: #4e5969;
}

/* 角色特定功能 */
.admin-features,
.teacher-features,
.student-features {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
  padding: 24px;
}

.feature-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 24px;
  border: 1px solid #f0f2f5;
  border-radius: 8px;
  background-color: #ffffff;
  cursor: pointer;
  transition: all 0.2s ease;
  text-decoration: none;
  color: inherit;
}

.feature-item:hover {
  border-color: #165dff;
  box-shadow: 0 4px 12px rgba(22, 93, 255, 0.15);
  transform: translateY(-2px);
}

.feature-icon {
  font-size: 32px;
  margin-bottom: 12px;
}

.feature-text {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
}

/* 统计内容 */
.stats-content {
  padding: 20px;
}

.stats-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #f0f2f5;
}

.stats-item:last-child {
  border-bottom: none;
}

.stats-label {
  font-size: 14px;
  color: #4e5969;
}

.stats-value {
  font-size: 16px;
  font-weight: 600;
  color: #165dff;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .person-page {
    padding: 20px 0;
  }

  .card-header {
    padding: 16px 20px;
  }

  .info-content,
  .submission-list {
    padding: 20px;
  }

  .admin-features,
  .teacher-features,
  .student-features {
    grid-template-columns: 1fr;
  }

  .contribution-calendar {
    overflow-x: auto;
  }

  .github-calendar-grid {
    grid-template-columns: repeat(20, minmax(10px, 1fr));
    grid-gap: 2px;
    max-width: 100%;
  }
}

/* GitHub风格贡献日历样式 */
.contribution-calendar {
  padding: 24px 24px 40px 24px;
  overflow: visible;
  position: relative;
}

.github-calendar {
  margin-bottom: 16px;
  position: relative;
  overflow: visible;
}

.github-calendar {
  margin-bottom: 16px;
  position: relative;
  overflow: visible;
  display: flex;
  justify-content: center;
}

.github-calendar-grid {
  display: grid;
  grid-template-columns: repeat(30, minmax(12px, 1fr));
  grid-gap: 3px;
  grid-auto-rows: minmax(12px, 1fr);
  width: 100%;
  max-width: 900px;
  position: relative;
  overflow: visible;
}

.github-contribution-day {
  width: 100%;
  aspect-ratio: 1/1;
  border-radius: 3px;
  position: relative;
  cursor: pointer;
  transition: all 0.2s ease;
  min-width: 12px;
  min-height: 12px;
}

.github-contribution-day:hover {
  transform: scale(1.2);
  z-index: 10;
}

.github-contribution-day.level-0 {
  background-color: #ebedf0;
}

.github-contribution-day.level-1 {
  background-color: #c6e48b;
}

.github-contribution-day.level-2 {
  background-color: #7bc96f;
}

.github-contribution-day.level-3 {
  background-color: #239a3b;
}

.github-contribution-day.level-4 {
  background-color: #196127;
}

.github-legend {
  display: flex;
  align-items: center;
  justify-content: flex-end;
  gap: 8px;
  font-size: 12px;
  color: #86909c;
  min-width: 650px;
  margin-top: 20px;
  position: relative;
  z-index: 1;
}

.legend-item {
  width: 12px;
  height: 12px;
  border-radius: 2px;
}

.legend-item.level-0 {
  background-color: #ebedf0;
}

.legend-item.level-1 {
  background-color: #c6e48b;
}

.legend-item.level-2 {
  background-color: #7bc96f;
}

.legend-item.level-3 {
  background-color: #239a3b;
}

.legend-item.level-4 {
  background-color: #196127;
}

.legend-label {
  margin: 0 4px;
  font-size: 11px;
}

/* GitHub风格提示框样式 */
.github-tooltip {
  position: absolute;
  bottom: 100%;
  left: 50%;
  transform: translateX(-50%);
  background-color: rgba(0, 0, 0, 0.8);
  color: #ffffff;
  border-radius: 6px;
  padding: 8px 12px;
  font-size: 12px;
  z-index: 9999;
  display: none;
  pointer-events: none;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
  max-width: 300px;
  word-wrap: break-word;
  white-space: normal;
  margin-bottom: 8px;
}

.github-contribution-day:hover .github-tooltip {
  display: block;
}

/* 左右边缘砖块的提示框样式 */
.github-contribution-day.edge-left .github-tooltip {
  left: 100%;
  transform: none;
  margin-left: 8px;
  right: auto;
}

.github-contribution-day.edge-right .github-tooltip {
  right: 100%;
  left: auto;
  transform: none;
  margin-right: 8px;
}

.tooltip-content {
  text-align: center;
}

.tooltip-date {
  margin-bottom: 4px;
  font-weight: 500;
}

.tooltip-count {
  margin-bottom: 8px;
}

.tooltip-list {
  text-align: left;
}

.tooltip-item {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 4px;
}

.tooltip-status {
  padding: 2px 6px;
  border-radius: 10px;
  font-size: 10px;
  font-weight: 500;
}

.tooltip-status.accepted {
  background-color: #2e7d32;
  color: #ffffff;
}

.tooltip-status.wrong_answer {
  background-color: #c62828;
  color: #ffffff;
}

.tooltip-status.time_limit_exceeded {
  background-color: #e65100;
  color: #ffffff;
}

.tooltip-status.runtime_error {
  background-color: #7b1fa2;
  color: #ffffff;
}

.tooltip-more {
  font-size: 10px;
  color: #cccccc;
  margin-top: 4px;
}

/* 账号设置样式 */
.account-settings-card {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
  margin-bottom: 24px;
  overflow: hidden;
}

.settings-content {
  padding: 24px;
}

.settings-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 0;
  border-bottom: 1px solid #f0f2f5;
}

.settings-item:last-child {
  border-bottom: none;
}

.settings-item.danger {
  margin-top: 16px;
  padding-top: 16px;
  border-top: 1px solid #ffebee;
}

.settings-label {
  font-size: 14px;
  color: #1d2129;
}

.settings-item.danger .settings-label {
  color: #f53f3f;
}

.settings-btn {
  padding: 6px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  background-color: #ffffff;
  color: #165dff;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.settings-btn:hover {
  background-color: #e8f3ff;
}

.settings-btn.danger {
  color: #f53f3f;
  border-color: #ffd7db;
}

.settings-btn.danger:hover {
  background-color: #ffebee;
}

/* 提交详情卡片 */
.submission-detail-card {
  background-color: #ffffff;
  border-radius: 12px;
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.12);
  margin-top: 24px;
  overflow: hidden;
}

.submission-detail-card .card-header {
  position: relative;
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
  border-radius: 50%;
  transition: all 0.2s ease;
}

.close-btn:hover {
  background-color: #f0f2f5;
  color: #1d2129;
}

.submission-detail-list {
  padding: 24px;
}

.submission-detail-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border: 1px solid #f0f2f5;
  border-radius: 8px;
  margin-bottom: 12px;
  transition: all 0.2s ease;
}

.submission-detail-item:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(22, 93, 255, 0.1);
}

.submission-detail-title {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 4px;
}

.submission-detail-meta {
  display: flex;
  gap: 16px;
  font-size: 12px;
  color: #86909c;
}

/* 展开的提交详情 */
.submission-detail-item {
  cursor: pointer;
  transition: all 0.3s ease;
}

.submission-detail-expanded {
  margin-top: 16px;
  padding-top: 16px;
  border-top: 1px solid #f0f2f5;
  width: 100%;
}

.submission-detail-code {
  background-color: #f7f8fa;
  border-radius: 8px;
  overflow: hidden;
}

.code-header {
  background-color: #e8ecef;
  padding: 8px 16px;
  font-size: 12px;
  font-weight: 500;
  color: #4e5969;
  border-bottom: 1px solid #d1d9e0;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.copy-code-btn {
  padding: 4px 8px;
  border: 1px solid #d1d9e0;
  border-radius: 4px;
  background-color: #ffffff;
  color: #4e5969;
  font-size: 11px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.copy-code-btn:hover {
  background-color: #f0f2f5;
  border-color: #165dff;
  color: #165dff;
}

.code-content {
  padding: 16px;
  font-size: 12px;
  line-height: 1.5;
  color: #1d2129;
  overflow-x: auto;
  margin: 0;
  font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
  white-space: pre-wrap;
  word-wrap: break-word;
}

.submission-detail-status {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.submission-detail-status.accepted {
  background-color: #e8f5e9;
  color: #2e7d32;
}

.submission-detail-status.wrong_answer {
  background-color: #ffebee;
  color: #c62828;
}

.submission-detail-status.time_limit_exceeded {
  background-color: #fff3e0;
  color: #e65100;
}

.submission-detail-status.runtime_error {
  background-color: #f3e5f5;
  color: #7b1fa2;
}

.submission-detail-status.error {
  background-color: #ffebee;
  color: #c62828;
}

/* 弹窗样式 */
.dialog-content {
  padding: 20px;
}

.dialog-content .form-item {
  margin-bottom: 16px;
}

.dialog-content .form-label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.dialog-content .form-input {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #1d2129;
}

.dialog-content .form-input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  margin-top: 24px;
}

.btn {
  padding: 8px 16px;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn.secondary {
  border: 1px solid #e5e6eb;
  background-color: #ffffff;
  color: #1d2129;
}

.btn.secondary:hover {
  border-color: #165dff;
  color: #165dff;
}

.btn.primary {
  border: none;
  background-color: #165dff;
  color: #ffffff;
}

.btn.primary:hover {
  background-color: #0e42c5;
}

/* 隐私设置样式 */
.privacy-options {
  display: flex;
  flex-direction: column;
  gap: 12px;
  margin-top: 12px;
}

.privacy-option {
  display: flex;
  align-items: center;
  gap: 8px;
}

.privacy-option input[type='checkbox'] {
  width: 16px;
  height: 16px;
  cursor: pointer;
}

.privacy-option label {
  font-size: 14px;
  color: #1d2129;
  cursor: pointer;
}

/* 表单样式 */
.form-item {
  margin-bottom: 16px;
}

.form-label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-input {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  font-size: 14px;
  transition: border-color 0.3s;
}

.form-input:focus {
  outline: none;
  border-color: #409eff;
}

.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  margin-top: 20px;
}
</style>
