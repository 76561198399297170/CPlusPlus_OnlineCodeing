<template>
  <MainLayout>
    <div class="problem-set-detail container">
      <div class="page-header">
        <button class="back-btn" @click="goBack">← 返回</button>
        <h1>{{ problemSet.name || '题目组详情' }}</h1>
        <button v-if="canDelete" class="delete-btn" @click="showDeleteConfirm">删除</button>
      </div>

      <div v-if="loading" class="loading-container">
        <div class="loading-spinner"></div>
        <p>加载中...</p>
      </div>

      <div v-else-if="error" class="error-container">
        <p class="error-message">{{ error }}</p>
        <button class="retry-btn" @click="loadProblemSetDetail">重试</button>
      </div>

      <div v-else class="problem-set-content">
        <!-- 题目组信息 -->
        <div class="info-section">
          <h2>{{ problemSet.name }}</h2>
          <p class="description">{{ problemSet.description }}</p>
          <div class="meta-info">
            <div class="meta-item">
              <span class="meta-label">开始时间：</span>
              <span class="meta-value">{{ formatDate(problemSet.startTime) }}</span>
            </div>
            <div class="meta-item">
              <span class="meta-label">结束时间：</span>
              <span class="meta-value">{{ formatDate(problemSet.endTime) }}</span>
            </div>
            <div class="meta-item">
              <span class="meta-label">创建时间：</span>
              <span class="meta-value">{{ formatDate(problemSet.createTime) }}</span>
            </div>
            <div class="meta-item">
              <span class="meta-label">创建者：</span>
              <span class="meta-value">
                <router-link
                  v-if="creatorInfo.nickname"
                  :to="`/profile/${creatorInfo.id}`"
                  class="creator-link"
                >
                  {{ creatorInfo.nickname }}
                </router-link>
                <span v-else>{{ problemSet.creatorId }}</span>
              </span>
            </div>
            <div class="meta-item">
              <span class="meta-label">题目数量：</span>
              <span class="meta-value">{{ problemSet.problems.length }} 题</span>
            </div>
          </div>

          <!-- 竞赛倒计时 -->
          <div v-if="problemSet?.startTime" class="contest-timer">
            <div class="timer-status" :class="contestStatusClass">
              {{ contestStatus }}
            </div>
            <div v-if="contestStatus === '进行中'" class="timer-display">
              <span class="timer-label">剩余时间：</span>
              <span class="timer-value">{{ remainingTime }}</span>
            </div>
          </div>

          <!-- 竞赛未开始/已结束提示 -->
          <div
            v-if="problemSet?.startTime && contestStatus && contestStatus !== '进行中'"
            class="contest-warning"
          >
            <div v-if="contestStatus === '未开始'" class="warning-message">
              该竞赛尚未开始，开始时间为 {{ formatDate(problemSet.startTime) }}
            </div>
            <div v-else-if="contestStatus === '已结束'" class="warning-message">
              该竞赛已于 {{ formatDate(problemSet.endTime) }} 结束
            </div>
          </div>
        </div>

        <!-- 题目列表 -->
        <div class="problems-section">
          <h3>题目列表</h3>
          <div class="problems-list">
            <div
              v-for="(problem, index) in problemSet.problems"
              :key="problem.id"
              class="problem-item"
            >
              <div class="problem-order">{{ index + 1 }}</div>
              <div class="problem-info">
                <h4 class="problem-title">{{ problem.title }}</h4>
                <div class="problem-meta">
                  <span class="problem-difficulty" :class="'difficulty-' + problem.difficulty">
                    {{ getDifficultyText(problem.difficulty) }}
                  </span>
                  <div class="problem-tags">
                    <span v-for="tag in problem.tags" :key="tag" class="tag">
                      {{ tag }}
                    </span>
                  </div>
                  <span class="problem-status" :class="getStatusCodeClass(problem.completed)">
                    {{ getStatusText(problem.completed) }}
                  </span>
                </div>
              </div>
              <router-link
                v-if="canStartProblem"
                :to="`/editor/${problem.id}?problemSetId=${problemSet.id}`"
                class="solve-btn"
              >
                开始答题
              </router-link>
              <span v-else class="solve-btn disabled">
                {{ contestStatus === '未开始' ? '未开始' : '已结束' }}
              </span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 删除确认弹窗 -->
    <CustomDialog
      :visible="showDeleteDialog"
      title="删除题目组"
      type="warning"
      :buttons="deleteDialogButtons"
      @close="showDeleteDialog = false"
    >
      <div class="delete-confirm-content">
        <p class="warning-text">确定要删除题目组「{{ problemSet.name }}」吗？此操作不可恢复！</p>
        <p class="confirm-hint">请输入以下随机字符串以确认删除：</p>
        <div class="random-string-box">{{ deleteConfirmCode }}</div>
        <input
          type="text"
          v-model="deleteConfirmInput"
          class="confirm-input"
          :placeholder="'请输入上方随机字符串'"
          @keyup.enter="handleDelete"
        />
      </div>
    </CustomDialog>

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
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import CustomDialog from '@/components/Common/CustomDialog.vue'
import NotificationMessage from '@/components/Common/NotificationMessage.vue'
import { useNotification } from '@/composables/useNotification'
import { apiService } from '@/services/api'

const route = useRoute()
const router = useRouter()
const {
  notifications,
  success,
  error: notifyError,
  warning,
  remove: removeNotification,
} = useNotification()

const problemSetId = ref(String(route.params.id))
const isContest = ref(route.query.isContest === '1')
const problemSet = ref({
  id: 0,
  name: '',
  description: '',
  startTime: '',
  endTime: '',
  createTime: '',
  creatorId: 0,
  problems: [],
})
const creatorInfo = ref({
  id: 0,
  nickname: '',
})
const loading = ref(true)
const error = ref('')

const userInfo = ref({
  id: 0,
  role: '',
})

const showDeleteDialog = ref(false)
const deleteConfirmCode = ref('')
const deleteConfirmInput = ref('')

// 竞赛相关变量
const contestStatus = ref('')
const remainingTime = ref('')
const contestTimer = ref(null)

// 计算属性
const contestStatusClass = computed(() => {
  switch (contestStatus.value) {
    case '未开始':
      return 'status-pending'
    case '进行中':
      return 'status-running'
    case '已结束':
      return 'status-ended'
    default:
      return ''
  }
})

// 判断是否可以开始答题（仅竞赛模式需要检查时间，管理员、学生、老师都可以正常访问）
const canStartProblem = computed(() => {
  if (!isContest.value) {
    return true
  }
  // 所有用户都可以正常访问，不受时间限制
  return true
})

const canDelete = computed(() => {
  // 如果当前登录角色是管理员，始终显示删除按钮
  if (userInfo.value.role === 'admin') {
    return true
  }
  // 如果是学生或老师，则检查是否是创建者
  return creatorInfo.value.id === userInfo.value.id
})

const deleteDialogButtons = computed(() => [
  {
    text: '取消',
    type: 'secondary',
    callback: () => {
      showDeleteDialog.value = false
      deleteConfirmInput.value = ''
    },
  },
  {
    text: '确认删除',
    type: 'error',
    callback: handleDelete,
  },
])

const loadProblemSetDetail = async () => {
  try {
    loading.value = true
    error.value = ''

    // 如果是竞赛模式，传递 isContest 参数
    const response = await apiService.getProblemSetDetail(
      parseInt(problemSetId.value),
      isContest.value ? 1 : undefined,
    )

    if (response.code === 0) {
      problemSet.value = response.data
      // 获取创建者信息
      if (response.data.creatorId) {
        const creatorResponse = await apiService.getUserProfile(response.data.creatorId)
        if (creatorResponse.code === 0) {
          creatorInfo.value = {
            id: creatorResponse.data.id,
            nickname: creatorResponse.data.nickname,
          }
        }
      }

      // 开始竞赛计时（只要有startTime就计时）
      if (response.data.startTime) {
        startContestTimer()
      } else {
        // 没有时间限制，设置为空
        contestStatus.value = ''
        remainingTime.value = ''
      }
    } else {
      error.value = response.msg || '获取题目组详情失败'
    }
  } catch (err) {
    error.value = '网络错误，获取题目组详情失败'
    console.error('获取题目组详情失败:', err)
  } finally {
    loading.value = false
  }
}

const goBack = () => {
  router.back()
}

const formatDate = (dateString) => {
  if (!dateString) return '-'

  // 处理字节数组格式的时间字符串
  let formattedDateString = dateString
  if (Array.isArray(dateString)) {
    try {
      formattedDateString = new TextDecoder('utf-8').decode(new Uint8Array(dateString))
      formattedDateString = formattedDateString.replace(/\0/g, '').trim()
    } catch (e) {
      console.error('日期字节数组转换错误:', e)
      return '-'
    }
  }

  const date = new Date(formattedDateString)
  if (isNaN(date.getTime())) {
    return '-'
  }
  return date.toLocaleString('zh-CN')
}

const getDifficultyText = (difficulty) => {
  switch (difficulty) {
    case 1:
      return '入门'
    case 2:
      return '基础'
    case 3:
      return '中等'
    case 4:
      return '进阶'
    case 5:
      return '困难'
    default:
      return '未知'
  }
}

const getStatusText = (status) => {
  switch (status) {
    case 0:
      return '未开始'
    case 1:
      return '未通过'
    case 2:
      return '已通过'
    default:
      return '未知'
  }
}

const getStatusCodeClass = (status) => {
  switch (status) {
    case 0:
      return 'status-pending'
    case 1:
      return 'status-failed'
    case 2:
      return 'status-passed'
    default:
      return ''
  }
}

// 竞赛相关函数
const startContestTimer = () => {
  // 清除之前的定时器
  if (contestTimer.value) {
    clearInterval(contestTimer.value)
  }

  // 更新竞赛状态
  updateContestStatus()

  // 每秒钟更新一次
  contestTimer.value = setInterval(() => {
    updateContestStatus()
  }, 1000)
}

const updateContestStatus = () => {
  const now = new Date().getTime()

  // 检查时间格式
  const startTimeStr = problemSet.value.startTime
  const endTimeStr = problemSet.value.endTime

  console.log('时间信息:', {
    startTimeStr,
    endTimeStr,
    typeofStartTime: typeof startTimeStr,
    typeofEndTime: typeof endTimeStr,
    problemSet: problemSet.value,
  })

  if (!startTimeStr || !endTimeStr) {
    console.log('时间字段为空:', { startTimeStr, endTimeStr })
    contestStatus.value = ''
    remainingTime.value = ''
    return
  }

  // 确保时间字符串是正确的格式
  let formattedStartTimeStr = startTimeStr
  let formattedEndTimeStr = endTimeStr

  // 检查是否是字节数组
  if (Array.isArray(startTimeStr)) {
    console.log('startTime是字节数组:', startTimeStr)
    try {
      formattedStartTimeStr = new TextDecoder('utf-8').decode(new Uint8Array(startTimeStr))
      formattedStartTimeStr = formattedStartTimeStr.replace(/\0/g, '').trim()
      console.log('转换后startTime:', formattedStartTimeStr)
    } catch (e) {
      console.error('字节数组转换错误:', e)
      contestStatus.value = ''
      remainingTime.value = ''
      return
    }
  }

  if (Array.isArray(endTimeStr)) {
    console.log('endTime是字节数组:', endTimeStr)
    try {
      formattedEndTimeStr = new TextDecoder('utf-8').decode(new Uint8Array(endTimeStr))
      formattedEndTimeStr = formattedEndTimeStr.replace(/\0/g, '').trim()
      console.log('转换后endTime:', formattedEndTimeStr)
    } catch (e) {
      console.error('字节数组转换错误:', e)
      contestStatus.value = ''
      remainingTime.value = ''
      return
    }
  }

  // 尝试不同的时间解析方式
  let startTime, endTime

  // 方法1：直接解析
  try {
    startTime = new Date(formattedStartTimeStr).getTime()
    endTime = new Date(formattedEndTimeStr).getTime()
    console.log('直接解析结果:', { startTime, endTime })
  } catch (e) {
    console.error('时间解析错误:', e)
    contestStatus.value = ''
    remainingTime.value = ''
    return
  }

  // 检查时间是否有效
  if (isNaN(startTime) || isNaN(endTime)) {
    // 方法2：手动解析时间字符串
    try {
      // 格式：YYYY-MM-DD HH:mm:ss
      const startParts = formattedStartTimeStr.split(' ')
      if (startParts.length !== 2) {
        throw new Error('时间格式错误：缺少空格分隔符')
      }
      const startDateParts = startParts[0].split('-')
      const startTimeParts = startParts[1].split(':')
      if (startDateParts.length !== 3 || startTimeParts.length !== 3) {
        throw new Error('时间格式错误：日期或时间部分格式不正确')
      }

      startTime = new Date(
        parseInt(startDateParts[0]),
        parseInt(startDateParts[1]) - 1, // 月份从0开始
        parseInt(startDateParts[2]),
        parseInt(startTimeParts[0]),
        parseInt(startTimeParts[1]),
        parseInt(startTimeParts[2]),
      ).getTime()

      const endParts = formattedEndTimeStr.split(' ')
      if (endParts.length !== 2) {
        throw new Error('时间格式错误：缺少空格分隔符')
      }
      const endDateParts = endParts[0].split('-')
      const endTimeParts = endParts[1].split(':')
      if (endDateParts.length !== 3 || endTimeParts.length !== 3) {
        throw new Error('时间格式错误：日期或时间部分格式不正确')
      }

      endTime = new Date(
        parseInt(endDateParts[0]),
        parseInt(endDateParts[1]) - 1, // 月份从0开始
        parseInt(endDateParts[2]),
        parseInt(endTimeParts[0]),
        parseInt(endTimeParts[1]),
        parseInt(endTimeParts[2]),
      ).getTime()
      console.log('手动解析结果:', { startTime, endTime })
    } catch (e) {
      console.error('手动时间解析错误:', e)
      contestStatus.value = ''
      remainingTime.value = ''
      return
    }
  }

  // 再次检查时间是否有效
  if (isNaN(startTime) || isNaN(endTime)) {
    console.error('时间解析后仍无效:', { startTime, endTime })
    contestStatus.value = ''
    remainingTime.value = ''
    return
  }

  // 先判断当前时间与开始/结束时间的关系
  if (now < startTime) {
    // 未开始
    contestStatus.value = '未开始'
    remainingTime.value = ''
  } else if (now >= endTime) {
    // 已结束
    contestStatus.value = '已结束'
    remainingTime.value = ''

    // 清除定时器
    if (contestTimer.value) {
      clearInterval(contestTimer.value)
      contestTimer.value = null
    }
  } else {
    // 在时间范围内，再检查是否是长期有效
    const timeDiffYears = (endTime - startTime) / (1000 * 60 * 60 * 24 * 365)
    if (timeDiffYears > 100) {
      // 长期有效
      contestStatus.value = '进行中'
      remainingTime.value = '长期有效'
    } else {
      // 进行中
      contestStatus.value = '进行中'
      const remaining = endTime - now
      remainingTime.value = formatRemainingTime(remaining)
    }
  }
}

const formatRemainingTime = (ms) => {
  const hours = Math.floor(ms / (1000 * 60 * 60))
  const minutes = Math.floor((ms % (1000 * 60 * 60)) / (1000 * 60))
  const seconds = Math.floor((ms % (1000 * 60)) / 1000)

  return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`
}

const cleanupTimer = () => {
  if (contestTimer.value) {
    clearInterval(contestTimer.value)
    contestTimer.value = null
  }
}

const generateRandomCode = () => {
  const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789'
  let code = ''
  for (let i = 0; i < 6; i++) {
    code += chars.charAt(Math.floor(Math.random() * chars.length))
  }
  return code
}

const showDeleteConfirm = () => {
  deleteConfirmCode.value = generateRandomCode()
  deleteConfirmInput.value = ''
  showDeleteDialog.value = true
}

const handleDelete = async () => {
  if (deleteConfirmInput.value !== deleteConfirmCode.value) {
    warning('输入的确认码不正确，请重新输入')
    return
  }

  try {
    const response = await apiService.deleteProblemSet(parseInt(problemSetId.value))
    if (response.code === 0) {
      showDeleteDialog.value = false
      success('删除成功')
      router.push('/training')
    } else {
      notifyError(response.msg || '删除失败')
    }
  } catch (err) {
    notifyError('网络错误，删除失败')
    console.error('删除题目组失败:', err)
  }
}

onMounted(() => {
  const cachedUserInfo = localStorage.getItem('userInfo')
  if (cachedUserInfo) {
    userInfo.value = JSON.parse(cachedUserInfo)
  }
  loadProblemSetDetail()
})

onUnmounted(() => {
  cleanupTimer()
})
</script>

<style scoped>
.problem-set-detail {
  padding: 40px 0;
  display: flex;
  flex-direction: column;
  gap: 30px;
}

.page-header {
  display: flex;
  align-items: center;
  gap: 16px;
  margin-bottom: 20px;
}

.back-btn {
  padding: 8px 16px;
  background-color: #f7f8fa;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.back-btn:hover {
  background-color: #e8f0ff;
  border-color: #165dff;
  color: #165dff;
}

.page-header h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

.loading-container,
.error-container {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 0;
  gap: 16px;
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid #f3f3f3;
  border-top: 3px solid #165dff;
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}

.error-message {
  color: #f53f3f;
  font-size: 16px;
}

.retry-btn {
  padding: 8px 16px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.retry-btn:hover {
  background-color: #0e42c2;
}

.problem-set-content {
  display: flex;
  flex-direction: column;
  gap: 30px;
}

.info-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.info-section h2 {
  font-size: 20px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 16px 0;
}

.description {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.5;
  margin: 0 0 20px 0;
}

.meta-info {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 12px;
}

.meta-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.meta-label {
  font-size: 12px;
  color: #86909c;
}

.meta-value {
  font-size: 14px;
  color: #1d2129;
  font-weight: 500;
}

.creator-link {
  color: #165dff;
  text-decoration: none;
  transition: all 0.2s ease;
}

.creator-link:hover {
  text-decoration: underline;
  color: #0e42c2;
}

.problems-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.problems-section h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 20px 0;
}

.problems-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.problem-item {
  display: flex;
  align-items: center;
  padding: 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  transition: all 0.2s ease;
  gap: 16px;
}

.problem-item:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.problem-order {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background-color: #165dff;
  color: #ffffff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  font-weight: 600;
  flex-shrink: 0;
}

.problem-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.problem-title {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

.problem-meta {
  display: flex;
  align-items: center;
  gap: 12px;
  flex-wrap: wrap;
}

.problem-difficulty {
  font-size: 12px;
  padding: 4px 12px;
  border-radius: 10px;
  font-weight: 500;
  white-space: nowrap;
}

.difficulty-1,
.difficulty-2 {
  background-color: #e8f5e8;
  color: #00b42a;
}

.difficulty-3 {
  background-color: #fff3e0;
  color: #ff7d00;
}

.difficulty-4,
.difficulty-5 {
  background-color: #fce4ec;
  color: #f53f3f;
}

.problem-tags {
  display: flex;
  gap: 6px;
  flex-wrap: wrap;
}

.tag {
  font-size: 12px;
  padding: 2px 8px;
  background-color: #f0f2f5;
  border-radius: 10px;
  color: #4e5969;
}

.problem-status {
  font-size: 12px;
  padding: 4px 12px;
  border-radius: 10px;
  font-weight: 500;
  white-space: nowrap;
}

.status-pending {
  background-color: #f5f5f5;
  color: #86909c;
}

.status-failed {
  background-color: #fce4ec;
  color: #f53f3f;
}

.status-passed {
  background-color: #e8f5e8;
  color: #00b42a;
}

.solve-btn {
  padding: 8px 16px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  text-decoration: none;
  transition: all 0.2s ease;
  white-space: nowrap;
}

.solve-btn:hover {
  background-color: #0e42c2;
}

.solve-btn.disabled {
  background-color: #c0c4cc;
  cursor: not-allowed;
}

.solve-btn.disabled:hover {
  background-color: #c0c4cc;
}

/* 竞赛计时样式 */
.contest-timer {
  margin-top: 20px;
  padding: 16px;
  background-color: #f7f8fa;
  border-radius: 8px;
  border: 1px solid #e5e6eb;
}

.timer-status {
  font-size: 16px;
  font-weight: 600;
  margin-bottom: 8px;
  padding: 6px 12px;
  border-radius: 4px;
  display: inline-block;
}

.status-running {
  background-color: #e8f5e8;
  color: #00b42a;
  animation: pulse 1s infinite;
}

.status-ended {
  background-color: #fce4ec;
  color: #f53f3f;
}

.timer-display {
  display: flex;
  align-items: center;
  gap: 8px;
}

.timer-label {
  font-size: 14px;
  color: #4e5969;
}

.timer-value {
  font-size: 20px;
  font-weight: 600;
  color: #165dff;
  font-family: monospace;
}

@keyframes pulse {
  0% {
    opacity: 1;
  }
  50% {
    opacity: 0.7;
  }
  100% {
    opacity: 1;
  }
}

/* 竞赛警告提示 */
.contest-warning {
  margin-top: 16px;
  padding: 12px 16px;
  background-color: #fff3e0;
  border: 1px solid #ffb74d;
  border-radius: 8px;
}

.warning-message {
  color: #e65100;
  font-size: 14px;
}

.delete-btn {
  padding: 8px 16px;
  background-color: #f53f3f;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
  margin-left: auto;
}

.delete-btn:hover {
  background-color: #e03636;
}

.delete-confirm-content {
  text-align: center;
}

.warning-text {
  color: #f53f3f;
  font-size: 16px;
  font-weight: 600;
  margin-bottom: 16px;
}

.confirm-hint {
  color: #4e5969;
  font-size: 14px;
  margin-bottom: 12px;
}

.random-string-box {
  background-color: #f7f8fa;
  border: 2px dashed #e5e6eb;
  border-radius: 8px;
  padding: 16px;
  font-size: 24px;
  font-weight: 600;
  letter-spacing: 4px;
  color: #165dff;
  margin-bottom: 16px;
  user-select: all;
}

.confirm-input {
  width: 100%;
  padding: 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  text-align: center;
  letter-spacing: 2px;
}

.confirm-input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

@media (max-width: 768px) {
  .meta-info {
    grid-template-columns: 1fr;
  }

  .problem-item {
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }

  .problem-order {
    align-self: flex-start;
  }

  .solve-btn {
    align-self: flex-start;
  }
}
</style>
