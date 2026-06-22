<!-- src/views/problems/Index.vue -->
<template>
  <MainLayout>
    <div class="problems-page container">
      <div class="page-header">
        <h1>题目列表</h1>
        <div class="page-actions">
          <Button type="primary" @click="navigateToCreate"> + 创建题目 </Button>
          <Button v-if="canReview" type="secondary" @click="navigateToReview"> 题目审核 </Button>
        </div>
      </div>

      <!-- 平台数据统计 -->
      <div class="stats-container">
        <div class="stats-card card">
          <div class="stats-title">平台数据统计</div>
          <div class="stats-grid">
            <div class="stats-item">
              <div class="stats-value">{{ stats.totalProblems }}+</div>
              <div class="stats-label">编程题目</div>
            </div>
            <div class="stats-item">
              <div class="stats-value">{{ stats.supportedLanguages }}+</div>
              <div class="stats-label">支持语言</div>
            </div>
            <div class="stats-item">
              <div class="stats-value">{{ stats.totalSubmissions }}+</div>
              <div class="stats-label">代码提交</div>
            </div>
            <div class="stats-item">
              <div class="stats-value">{{ stats.judgeAccuracy }}%</div>
              <div class="stats-label">判题准确率</div>
            </div>
          </div>
        </div>
      </div>

      <!-- 题目列表 -->
      <div class="problems-list">
        <div class="problems-header">
          <div class="header-item id">ID</div>
          <div class="header-item title">题目名称</div>
          <div class="header-item difficulty">难度</div>
          <div class="header-item type">标签</div>
          <div class="header-item status">是否完成</div>
        </div>

        <div v-if="loading" class="loading">加载中...</div>
        <div v-else-if="error" class="error">{{ error }}</div>
        <div v-else class="problems-body">
          <div
            v-for="problem in problems"
            :key="problem.id"
            class="problem-item"
            @click="navigateToProblem(problem.id)"
          >
            <div class="item id">{{ problem.id }}</div>
            <div class="item title">{{ problem.title }}</div>
            <div class="item difficulty">
              <span :class="['difficulty-tag', getDifficultyClass(problem.difficulty)]">
                {{ getDifficultyText(problem.difficulty) }}
              </span>
            </div>
            <div class="item type">
              <span v-for="(tag, index) in problem.tags" :key="index" class="type-tag">
                {{ tag }}
              </span>
            </div>
            <div class="item status">
              <span :class="['status-tag', getStatusClass(problem.completed)]">
                {{ getStatusText(problem.completed) }}
              </span>
            </div>
          </div>
        </div>

        <!-- 分页 -->
        <div v-if="!loading && !error" class="pagination">
          <button class="pagination-btn" :disabled="page === 1" @click="changePage(page - 1)">
            上一页
          </button>
          <span class="pagination-info"> 第 {{ page }} 页，共 {{ totalPages }} 页 </span>
          <button
            class="pagination-btn"
            :disabled="page === totalPages"
            @click="changePage(page + 1)"
          >
            下一页
          </button>
        </div>
      </div>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import { apiService } from '@/services/api'

// 获取当前用户角色
const getUserRole = () => {
  const userInfo = localStorage.getItem('userInfo')
  if (userInfo) {
    try {
      const parsed = JSON.parse(userInfo)
      return parsed.role?.toUpperCase() || 'STUDENT'
    } catch (e) {
      return 'STUDENT'
    }
  }
  return 'STUDENT'
}

const currentRole = getUserRole()
const canReview = computed(() => {
  return currentRole === 'ADMIN' || currentRole === 'TEACHER'
})

// 响应式数据
const problems = ref([])
const loading = ref(false)
const error = ref('')
const page = ref(1)
const pageSize = ref(10)
const total = ref(0)
const stats = ref({
  totalProblems: 0,
  supportedLanguages: 0,
  totalSubmissions: 0,
  judgeAccuracy: 0,
})

// 计算属性
const totalPages = computed(() => {
  return Math.ceil(total.value / pageSize.value)
})

// 路由实例
const router = useRouter()

// 导航到题目详情
const navigateToProblem = (id) => {
  router.push(`/editor/${id}`)
}

// 导航到创建题目页面
const navigateToCreate = () => {
  router.push('/problems/create')
}

// 导航到审核题目页面
const navigateToReview = () => {
  router.push('/problems/review')
}

// 切换页码
const changePage = (newPage) => {
  if (newPage >= 1 && newPage <= totalPages.value) {
    page.value = newPage
    fetchProblems()
  }
}

// 获取状态类名
const getStatusClass = (completed) => {
  switch (completed) {
    case 0:
      return 'pending'
    case 1:
      return 'failed'
    case 2:
      return 'completed'
    default:
      return 'pending'
  }
}

// 获取状态文本
const getStatusText = (completed) => {
  switch (completed) {
    case 0:
      return '未开始'
    case 1:
      return '未通过'
    case 2:
      return '已通过'
    default:
      return '未开始'
  }
}

// 获取难度类名
const getDifficultyClass = (difficulty) => {
  switch (difficulty) {
    case 1:
      return 'easy'
    case 2:
      return 'easy'
    case 3:
      return 'medium'
    case 4:
      return 'hard'
    case 5:
      return 'hard'
    default:
      return 'medium'
  }
}

// 获取难度文本
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

// 格式化时间
const formatTime = (time) => {
  if (!time) return '-'
  const date = new Date(time)
  return date.toLocaleString('zh-CN')
}

// 获取题目列表
const fetchProblems = async () => {
  try {
    loading.value = true
    error.value = ''

    const response = await apiService.getProblems(page.value, pageSize.value)

    if (response.code === 0) {
      // 使用后端返回的完成状态
      problems.value = response.data.list
      total.value = response.data.total
    } else {
      error.value = response.msg || '获取题目列表失败'
    }
  } catch (err) {
    error.value = err.msg || '网络错误，无法获取题目列表'
  } finally {
    loading.value = false
  }
}

// 获取平台统计数据
const fetchStats = async () => {
  try {
    const response = await apiService.getStats()
    if (response.code === 0) {
      stats.value = response.data
    }
  } catch (err) {
    // 获取平台统计数据失败
  }
}

// 组件挂载时获取数据
onMounted(() => {
  fetchProblems()
  fetchStats()
})
</script>

<style scoped>
.problems-page {
  padding: 40px 0;
  display: flex;
  flex-direction: column;
  gap: 30px;
}

.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.page-header h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 0;
}

.page-actions {
  display: flex;
  gap: 12px;
}

/* 统计信息容器 */
.stats-container {
  width: 100%;

  .stats-card {
    padding: 30px;

    .stats-title {
      font-size: 18px;
      font-weight: 600;
      color: #1d2129;
      margin-bottom: 20px;
    }

    .stats-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
      gap: 20px;

      .stats-item {
        text-align: center;
        padding: 20px;
        background-color: #f7f8fa;
        border-radius: 8px;

        .stats-value {
          font-size: 28px;
          font-weight: 600;
          color: #165dff;
          margin-bottom: 8px;
        }

        .stats-label {
          font-size: 14px;
          color: #4e5969;
        }
      }
    }
  }
}

/* 题目列表 */
.problems-list {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  overflow: hidden;
  display: flex;
  flex-direction: column;
  height: 900px;
}

/* 表头 */
.problems-header {
  display: grid;
  grid-template-columns: 80px 2fr 100px 1.5fr 120px;
  padding: 16px;
  background-color: #f7f8fa;
  border-bottom: 1px solid #e5e6eb;
  font-weight: 500;
  color: #4e5969;
  font-size: 14px;
}

.header-item {
  display: flex;
  align-items: center;
}

/* 表体 */
.problems-body {
  flex: 1;
  overflow-y: auto;
  min-height: 0;
}

/* 题目项 */
.problem-item {
  display: grid;
  grid-template-columns: 80px 2fr 100px 1.5fr 120px;
  padding: 16px;
  border-bottom: 1px solid #e5e6eb;
  cursor: pointer;
  transition: background-color 0.2s ease;

  &:hover {
    background-color: #f7f8fa;
  }
}

.item {
  display: flex;
  align-items: center;
  font-size: 14px;
  color: #1d2129;
}

.item.type {
  flex-wrap: wrap;
  gap: 8px;
}

.item.time {
  font-size: 13px;
  color: #86909c;
}

/* ID */
.item.id {
  font-weight: 500;
  color: #165dff;
}

/* 标题 */
.item.title {
  font-weight: 500;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

/* 难度标签 */
.difficulty-tag {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;

  &.easy {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.medium {
    background-color: #fff3e0;
    color: #ff7d00;
  }

  &.hard {
    background-color: #fce4ec;
    color: #f53f3f;
  }
}

/* 类型标签 */
.type-tag {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  color: #165dff;
  background-color: #e8f0ff;
  margin-right: 8px;
}

/* 状态标签 */
.status-tag {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
  display: flex;
  align-items: center;
  gap: 4px;

  &.completed {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.pending {
    background-color: #f5f5f5;
    color: #86909c;
  }

  &.failed {
    background-color: #fce4ec;
    color: #f53f3f;
  }

  &::before {
    content: '';
    display: inline-block;
    width: 8px;
    height: 8px;
    border-radius: 50%;
  }

  &.pending::before {
    background-color: #86909c;
  }

  &.failed::before {
    background-color: #f53f3f;
  }

  &.completed::before {
    background-color: #00b42a;
  }
}

/* 加载和错误状态 */
.loading,
.error {
  padding: 40px;
  text-align: center;
  color: #86909c;
}

.error {
  color: #f53f3f;
}

/* 分页 */
.pagination {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 16px;
  border-top: 1px solid #e5e6eb;
  gap: 16px;
  height: 60px;
  box-sizing: border-box;
}

.pagination-btn {
  padding: 6px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  background-color: #ffffff;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover:not(:disabled) {
    border-color: #165dff;
    color: #165dff;
  }

  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
}

.pagination-info {
  font-size: 14px;
  color: #4e5969;
}
</style>
