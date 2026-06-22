<!-- src/views/profile/Index.vue -->
<template>
  <MainLayout>
    <div class="profile-page container">
      <div v-if="loading" class="loading">加载中...</div>
      <div v-else-if="error" class="error">{{ error }}</div>
      <div v-else-if="userProfile" class="profile-content">
        <!-- 个人信息卡片 -->
        <div class="profile-card">
          <div class="profile-header">
            <div class="avatar">
              <span class="avatar-text">{{ getAvatarText(userProfile.nickname) }}</span>
            </div>
            <div class="profile-info">
              <h2 class="nickname">{{ userProfile.nickname }}</h2>
              <div class="role" v-if="isInfoVisible(0)">
                {{ getUserRoleText(userProfile.role) }}
              </div>
              <div class="user-desc" v-if="userProfile.userDesc">{{ userProfile.userDesc }}</div>
            </div>
          </div>

          <!-- 详细信息 -->
          <div class="profile-details" v-if="hasDetails">
            <div
              class="detail-item"
              v-if="isInfoVisible(7) && (isCurrentUserAdmin || userProfile.realName)"
            >
              <span class="detail-label">真实姓名：</span>
              <span class="detail-value">{{ userProfile.realName || '-' }}</span>
            </div>
            <div
              class="detail-item"
              v-if="isInfoVisible(6) && (isCurrentUserAdmin || userProfile.userRoleNo)"
            >
              <span class="detail-label">学号/工号：</span>
              <span class="detail-value">{{ userProfile.userRoleNo || '-' }}</span>
            </div>
            <div
              class="detail-item"
              v-if="isInfoVisible(1) && (isCurrentUserAdmin || userProfile.school)"
            >
              <span class="detail-label">学校：</span>
              <span class="detail-value">{{ userProfile.school || '-' }}</span>
            </div>
            <div
              class="detail-item"
              v-if="isInfoVisible(2) && (isCurrentUserAdmin || userProfile.grade)"
            >
              <span class="detail-label">年级：</span>
              <span class="detail-value">{{
                userProfile.grade ? userProfile.grade + '年级' : '-'
              }}</span>
            </div>
          </div>
        </div>

        <!-- 统计数据 -->
        <div
          class="stats-card"
          v-if="
            (isInfoVisible(3) && (isCurrentUserAdmin || recentSubmissionsCount > 0)) ||
            (isInfoVisible(4) && (isCurrentUserAdmin || recentSolvedCount > 0))
          "
        >
          <h3>编程统计</h3>
          <div class="stats-grid">
            <div
              class="stat-item"
              v-if="isInfoVisible(3) && (isCurrentUserAdmin || recentSubmissionsCount > 0)"
            >
              <div class="stat-value">{{ recentSubmissionsCount }}</div>
              <div class="stat-label">近期提交次数</div>
            </div>
            <div
              class="stat-item"
              v-if="isInfoVisible(4) && (isCurrentUserAdmin || recentSolvedCount > 0)"
            >
              <div class="stat-value">{{ recentSolvedCount }}</div>
              <div class="stat-label">近期已解决题目</div>
            </div>
            <div
              class="stat-item"
              v-if="
                isInfoVisible(3) &&
                isInfoVisible(4) &&
                (isCurrentUserAdmin || recentSubmissionsCount > 0)
              "
            >
              <div class="stat-value">{{ recentSolveRate }}%</div>
              <div class="stat-label">近期解题率</div>
            </div>
          </div>
        </div>

        <!-- 语言分布 -->
        <div
          class="language-card"
          v-if="
            isInfoVisible(5) &&
            (isCurrentUserAdmin || (submissionStatsList && submissionStatsList.length > 0))
          "
        >
          <h3>编程语言分布</h3>
          <div class="language-stats">
            <div v-for="(stat, index) in submissionStatsList" :key="index" class="language-item">
              <div class="language-name">{{ stat.language }}</div>
              <div class="language-bar-container">
                <div
                  class="language-bar"
                  :style="{ width: getLanguagePercentage(stat.count) + '%' }"
                ></div>
              </div>
              <div class="language-count">{{ stat.count }}</div>
            </div>
          </div>
        </div>

        <!-- 贡献日历 -->
        <div
          class="contribution-card"
          v-if="
            isInfoVisible(3) &&
            (isCurrentUserAdmin || (submissions.list && submissions.list.length > 0))
          "
        >
          <h3>提交活动</h3>
          <div class="github-calendar">
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
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRoute } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import { apiService } from '@/services/api'

const route = useRoute()
const userId = computed(() => {
  const id = route.params.id
  return id ? Number(id) : undefined
})

// 获取当前登录用户信息
const getCurrentUser = () => {
  try {
    const cached = localStorage.getItem('userInfo')
    if (cached) {
      return JSON.parse(cached)
    }
  } catch (e) {
    console.error('Failed to parse userInfo from localStorage:', e)
  }
  return null
}

// 判断当前登录用户是否是管理员
const isCurrentUserAdmin = computed(() => {
  const currentUser = getCurrentUser()
  return currentUser?.role === 'ADMIN' || currentUser?.role?.toLowerCase() === 'admin'
})

// 当前登录用户是管理员时显示所有信息，否则按隐私设置
const isInfoVisible = (bitIndex) => {
  if (isCurrentUserAdmin.value) {
    return true
  }

  // 否则按隐私设置
  if (!userProfile.value) {
    return false
  }

  // 如果info_display_setting未定义，默认显示（表示用户未设置隐私）
  if (userProfile.value.info_display_setting === undefined) {
    return true
  }

  return !!(userProfile.value.info_display_setting & (1 << bitIndex))
}

const userProfile = ref(null)
const loading = ref(true)
const error = ref(null)

// 贡献日历相关数据
const submissions = ref({ list: [], total: 0, page: 1, pageSize: 50 })
const calendarDays = ref([])
const submissionsByDate = ref({})

// 加载用户信息
const loadUserProfile = async () => {
  try {
    const response = await apiService.getUserProfile(userId.value)
    if (response.code === 0) {
      userProfile.value = response.data
    } else {
      error.value = response.msg
    }
  } catch (err) {
    error.value = '获取用户信息失败'
  } finally {
    loading.value = false
  }
}

// 智能获取提交记录策略：确保覆盖最近6个月
const loadSubmissionsSmart = async () => {
  const targetDays = 180 // 目标覆盖最近6个月
  const minBatchSize = 30 // 每批获取最小数量
  const allSubmissions = []
  let currentPage = 1
  let hasMore = true
  let totalLoaded = 0

  while (hasMore && currentPage <= 20) {
    const response = await apiService.getUserSubmissions(currentPage, minBatchSize, userId.value)
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

    // 分析日期分布，检查是否覆盖了最近6个月
    const today = new Date()
    today.setHours(0, 0, 0, 0)
    const sixMonthsAgo = new Date(today)
    sixMonthsAgo.setMonth(sixMonthsAgo.getMonth() - 6)

    const hasRecentSubmissions = list.some((sub) => {
      if (!sub.createdAt) return false
      let dateStr = sub.createdAt
      if (dateStr.includes(' ')) {
        dateStr = dateStr.replace(' ', 'T')
      }
      const subDate = new Date(dateStr)
      return subDate >= sixMonthsAgo
    })

    // 如果当前批有最近6个月的记录，说明数据足够
    if (hasRecentSubmissions) {
      hasMore = false
      break
    }

    // 如果已加载的数据足够多（超过6个月*每天预估30次），也停止
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

// 生成GitHub风格的日历数据（根据实际提交动态调整）
const generateCalendarDays = () => {
  const days = []
  const today = new Date()
  today.setHours(0, 0, 0, 0)

  // 确定最后日期：始终使用今天，确保日历更新到最新日期
  let endDate = new Date(today)

  // 确定最早日期：最远6个月前
  let startDate = new Date(endDate)
  startDate.setMonth(startDate.getMonth() - 6)

  // 如果有更早的提交记录，检查是否需要更早的日期
  const submissionDates = Object.keys(submissionsByDate.value)
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

// 按日期整理提交记录
const processSubmissionsByDate = () => {
  const byDate = {}
  submissions.value.list.forEach((submission) => {
    if (submission.createdAt) {
      let dateKey
      if (submission.createdAt.includes('T')) {
        dateKey = submission.createdAt.split('T')[0]
      } else if (submission.createdAt.includes(' ')) {
        dateKey = submission.createdAt.split(' ')[0]
      } else {
        dateKey = submission.createdAt
      }
      if (!byDate[dateKey]) {
        byDate[dateKey] = []
      }
      byDate[dateKey].push(submission)
    }
  })
  submissionsByDate.value = byDate
  return byDate
}

// 按日期获取提交记录
const getDaySubmissions = (date) => {
  return submissionsByDate.value[date] || []
}

// 获取贡献等级
const getContributionLevel = (date) => {
  const submissions = getDaySubmissions(date)
  const count = submissions.length

  if (count === 0) return 'level-0'
  if (count <= 2) return 'level-1'
  if (count <= 5) return 'level-2'
  if (count <= 10) return 'level-3'
  return 'level-4'
}

// 格式化日期
const formatDate = (dateStr) => {
  const date = new Date(dateStr)
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
  const days = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']

  return `${date.getFullYear()}/${date.getMonth() + 1}/${date.getDate()} ${days[date.getDay()]}`
}

// 计算属性
const hasDetails = computed(() => {
  if (!userProfile.value) return false

  // 如果当前登录用户是管理员，显示所有信息
  if (isCurrentUserAdmin.value) {
    return true
  }

  return (
    userProfile.value.realName ||
    userProfile.value.userRoleNo ||
    userProfile.value.school ||
    userProfile.value.grade
  )
})

const solveRate = computed(() => {
  if (!userProfile.value || userProfile.value.submissionsCount === 0) return 0
  return Math.round((userProfile.value.solvedCount / userProfile.value.submissionsCount) * 100)
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

// 将后端返回的对象格式转换为数组格式
const submissionStatsList = computed(() => {
  if (!userProfile.value || !userProfile.value.submissionStats) return []
  const stats = userProfile.value.submissionStats
  return Object.entries(stats)
    .filter(([_, count]) => count > 0)
    .map(([language, count]) => ({ language, count }))
})

// 辅助函数
const getAvatarText = (nickname) => {
  if (!nickname) return '?'
  return nickname.charAt(0).toUpperCase()
}

const getUserRoleText = (role) => {
  const roleMap = {
    student: '学生',
    teacher: '教师',
    admin: '管理员',
  }
  return roleMap[role] || role
}

const getLanguagePercentage = (count) => {
  if (!userProfile.value || userProfile.value.submissionsCount === 0) return 0
  return Math.round((count / userProfile.value.submissionsCount) * 100)
}

// 生命周期
onMounted(async () => {
  await loadUserProfile()
  await loadSubmissionsSmart()
  processSubmissionsByDate()
  generateCalendarDays()
})
</script>

<style scoped>
.profile-page {
  padding: 40px 0;
  min-height: 80vh;
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

.profile-content {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

/* 个人信息卡片 */
.profile-card {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 30px;
  transition: all 0.3s ease;
}

.profile-card:hover {
  box-shadow: 0 4px 16px 0 rgba(0, 0, 0, 0.15);
}

.profile-header {
  display: flex;
  align-items: center;
  gap: 20px;
  margin-bottom: 20px;
}

.avatar {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.avatar-text {
  font-size: 32px;
  font-weight: 600;
  color: white;
}

.profile-info {
  flex: 1;
}

.nickname {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 8px 0;
}

.account {
  font-size: 14px;
  color: #86909c;
  margin-bottom: 4px;
}

.role {
  display: inline-block;
  padding: 2px 8px;
  border-radius: 10px;
  font-size: 12px;
  font-weight: 500;
  margin-bottom: 8px;
}

.role[data-role='student'] {
  background: #ecf5ff;
  color: #409eff;
}

.role[data-role='teacher'] {
  background: #f0f9eb;
  color: #67c23a;
}

.role[data-role='admin'] {
  background: #fef0f0;
  color: #f56c6c;
}

.user-desc {
  font-size: 14px;
  color: #4e5969;
  margin-top: 8px;
  line-height: 1.5;
}

.profile-details {
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #f2f3f5;
  display: flex;
  flex-wrap: wrap;
  gap: 16px;
}

.detail-item {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
}

.detail-label {
  color: #86909c;
  font-weight: 500;
}

.detail-value {
  color: #4e5969;
}

/* 统计数据卡片 */
.stats-card {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 24px;
  transition: all 0.3s ease;
}

.stats-card:hover {
  box-shadow: 0 4px 16px 0 rgba(0, 0, 0, 0.15);
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

/* 语言分布卡片 */
.language-card {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 24px;
  transition: all 0.3s ease;
}

.language-card:hover {
  box-shadow: 0 4px 16px 0 rgba(0, 0, 0, 0.15);
}

.language-card h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 20px 0;
}

.language-stats {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.language-item {
  display: flex;
  align-items: center;
  gap: 12px;
}

.language-name {
  width: 80px;
  font-size: 14px;
  color: #4e5969;
  font-weight: 500;
}

.language-bar-container {
  flex: 1;
  height: 8px;
  background: #f2f3f5;
  border-radius: 4px;
  overflow: hidden;
}

.language-bar {
  height: 100%;
  background: linear-gradient(90deg, #409eff, #667eea);
  border-radius: 4px;
  transition: width 0.5s ease;
}

.language-count {
  width: 40px;
  font-size: 14px;
  color: #86909c;
  text-align: right;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .profile-page {
    padding: 20px 0;
  }

  .profile-card {
    padding: 20px;
  }

  .profile-header {
    flex-direction: column;
    text-align: center;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .language-name {
    width: 60px;
  }
}

/* 贡献日历卡片 */
.contribution-card {
  background: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
  padding: 24px;
  transition: all 0.3s ease;
}

.contribution-card:hover {
  box-shadow: 0 4px 16px 0 rgba(0, 0, 0, 0.15);
}

.contribution-card h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 20px 0;
}

/* GitHub风格贡献日历样式 */
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

.github-contribution-day.edge-left {
  margin-left: 0;
}

.github-contribution-day.edge-right {
  margin-right: 0;
}

/* 工具提示 */
.github-tooltip {
  position: absolute;
  bottom: 100%;
  left: 50%;
  transform: translateX(-50%) translateY(-8px);
  background-color: #24292e;
  color: white;
  padding: 10px 12px;
  border-radius: 6px;
  font-size: 12px;
  white-space: nowrap;
  opacity: 0;
  visibility: hidden;
  transition: all 0.2s ease;
  z-index: 100;
  pointer-events: none;
}

.github-contribution-day:hover .github-tooltip {
  opacity: 1;
  visibility: visible;
}

.github-tooltip::after {
  content: '';
  position: absolute;
  top: 100%;
  left: 50%;
  transform: translateX(-50%);
  border: 6px solid transparent;
  border-top-color: #24292e;
}

.tooltip-content {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.tooltip-date {
  font-weight: 500;
  font-size: 13px;
  color: #c8c9cb;
  margin-bottom: 4px;
}

.tooltip-count {
  font-size: 12px;
  color: #fff;
}

.tooltip-list {
  max-height: 100px;
  overflow-y: auto;
  margin-top: 6px;
  padding-top: 6px;
  border-top: 1px solid #444d56;
}

.tooltip-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 2px 0;
  font-size: 11px;
}

.tooltip-status {
  padding: 1px 6px;
  border-radius: 4px;
  font-size: 10px;
  font-weight: 500;
}

.tooltip-status.ac {
  background-color: #28a745;
  color: white;
}

.tooltip-status.wa {
  background-color: #dc3545;
  color: white;
}

.tooltip-status.tle {
  background-color: #ffc107;
  color: #212529;
}

.tooltip-status.mle {
  background-color: #17a2b8;
  color: white;
}

.tooltip-status.re {
  background-color: #6f42c1;
  color: white;
}

.tooltip-status.ce {
  background-color: #fd7e14;
  color: white;
}

.tooltip-problem {
  color: #c8c9cb;
}

.tooltip-more {
  font-size: 11px;
  color: #959da5;
  margin-top: 4px;
}

/* 图例 */
.github-legend {
  display: flex;
  align-items: center;
  justify-content: flex-end;
  gap: 6px;
  font-size: 12px;
  color: #586069;
}

.legend-label {
  font-size: 12px;
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

/* 响应式设计 */
@media (max-width: 768px) {
  .github-calendar-grid {
    grid-template-columns: repeat(20, minmax(10px, 1fr));
    grid-gap: 2px;
    max-width: 100%;
  }
}
</style>
