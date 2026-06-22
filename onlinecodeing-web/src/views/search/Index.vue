<!-- src/views/search/Index.vue -->
<template>
  <MainLayout>
    <div class="search-page container">
      <div class="page-header">
        <h1>搜索</h1>
      </div>

      <!-- 搜索框 -->
      <div class="search-container card">
        <div class="search-input-wrapper">
          <input
            type="text"
            class="search-input"
            v-model="keyword"
            placeholder="搜索题目、标签..."
            @keyup.enter="handleSearch"
          />
          <button class="search-btn" @click="handleSearch">搜索</button>
        </div>
      </div>

      <!-- 筛选条件 -->
      <div class="filter-container card">
        <div class="filter-section">
          <h3>标签筛选</h3>
          <div class="custom-tag-input">
            <input
              type="text"
              v-model="customTag"
              placeholder="输入自定义标签"
              @keyup.enter="addCustomTag"
            />
            <button @click="addCustomTag" class="add-tag-btn">添加</button>
          </div>
          <div class="tags-container">
            <span
              v-for="tag in allTags"
              :key="tag"
              :class="['tag-item', { active: selectedTags.includes(tag) }]"
              @click="toggleTag(tag)"
            >
              {{ tag }}
            </span>
          </div>
        </div>

        <div class="filter-section">
          <h3>难度筛选</h3>
          <div class="difficulty-container">
            <span
              v-for="level in difficultyLevels"
              :key="level.value"
              :class="['difficulty-item', { active: selectedDifficulty === level.value }]"
              @click="selectedDifficulty = level.value"
            >
              {{ level.label }}
            </span>
          </div>
        </div>

        <div class="filter-actions">
          <button class="reset-btn" @click="resetFilters">重置筛选</button>
          <button class="apply-btn" @click="handleFilterSearch">应用筛选</button>
        </div>
      </div>

      <!-- 搜索结果 -->
      <div class="results-container">
        <div class="results-header">
          <h3>搜索结果</h3>
          <div class="results-info">共 {{ total }} 个结果</div>
        </div>

        <div v-if="loading" class="loading">加载中...</div>
        <div v-else-if="error" class="error">{{ error }}</div>
        <div v-else-if="results.length === 0" class="no-results">没有找到符合条件的题目</div>
        <div v-else class="results-list">
          <div
            v-for="problem in results"
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
            <div class="item tags">
              <span v-for="(tag, index) in problem.tags" :key="index" class="tag">
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
        <div v-if="!loading && !error && results.length > 0" class="pagination">
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
import { ref, computed, onMounted, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import { apiService } from '@/services/api'

// 路由实例
const router = useRouter()
const route = useRoute()

// 响应式数据
const keyword = ref('')
const selectedTags = ref([])
const selectedDifficulty = ref(0) // 0表示不限
const results = ref([])
const loading = ref(false)
const error = ref('')
const page = ref(1)
const pageSize = ref(10)
const total = ref(0)
const customTag = ref('')

// 标签和难度选项
const allTags = ref([
  '数组',
  '字符串',
  '链表',
  '树',
  '图',
  '排序',
  '搜索',
  '动态规划',
  '贪心',
  '分治',
  '回溯',
  '位运算',
  '数学',
  '模拟',
  '哈希表',
  '栈',
  '队列',
])

const difficultyLevels = [
  { value: 0, label: '不限' },
  { value: 1, label: '入门' },
  { value: 2, label: '基础' },
  { value: 3, label: '中等' },
  { value: 4, label: '进阶' },
  { value: 5, label: '困难' },
]

// 计算属性
const totalPages = computed(() => {
  return Math.ceil(total.value / pageSize.value)
})

// 导航到题目详情
const navigateToProblem = (id) => {
  router.push(`/editor/${id}`)
}

// 切换页码
const changePage = (newPage) => {
  if (newPage >= 1 && newPage <= totalPages.value) {
    page.value = newPage
    if (keyword.value) {
      handleSearch()
    } else {
      handleFilterSearch()
    }
  }
}

// 切换标签
const toggleTag = (tag) => {
  const index = selectedTags.value.indexOf(tag)
  if (index > -1) {
    selectedTags.value.splice(index, 1)
  } else {
    selectedTags.value.push(tag)
  }
}

// 添加自定义标签
const addCustomTag = () => {
  const tag = customTag.value.trim()
  if (tag) {
    // 检查标签是否已存在
    if (!allTags.value.includes(tag) && !selectedTags.value.includes(tag)) {
      allTags.value.push(tag)
      selectedTags.value.push(tag)
    } else if (!selectedTags.value.includes(tag)) {
      selectedTags.value.push(tag)
    }
    customTag.value = ''
  }
}

// 重置筛选
const resetFilters = () => {
  selectedTags.value = []
  selectedDifficulty.value = 0
  customTag.value = ''
}

// 处理搜索
const handleSearch = async () => {
  try {
    loading.value = true
    error.value = ''
    page.value = 1

    const response = await apiService.searchProblems({
      keyword: keyword.value.trim() || undefined,
      tags: selectedTags.value.length > 0 ? selectedTags.value : undefined,
      difficulty: selectedDifficulty.value > 0 ? selectedDifficulty.value : undefined,
      page: page.value,
      pageSize: pageSize.value,
    })

    if (response.code === 0) {
      // 处理tags字段，确保它是一个数组
      results.value = response.data.list.map((problem) => {
        if (typeof problem.tags === 'string') {
          try {
            problem.tags = JSON.parse(problem.tags)
          } catch (e) {
            problem.tags = []
          }
        } else if (
          Array.isArray(problem.tags) &&
          problem.tags.length === 1 &&
          typeof problem.tags[0] === 'string'
        ) {
          // 处理嵌套数组的情况，如 ["[\"算法\", \"字符串\", \"贪心\"]"]
          try {
            problem.tags = JSON.parse(problem.tags[0])
          } catch (e) {
            problem.tags = []
          }
        }
        return problem
      })
      total.value = response.data.total
    } else {
      error.value = response.msg || '搜索失败'
    }
  } catch (err) {
    error.value = '网络错误，无法搜索'
  } finally {
    loading.value = false
  }
}

// 处理筛选搜索
const handleFilterSearch = async () => {
  try {
    loading.value = true
    error.value = ''
    page.value = 1

    const response = await apiService.searchProblems({
      tags: selectedTags.value.length > 0 ? selectedTags.value : undefined,
      difficulty: selectedDifficulty.value > 0 ? selectedDifficulty.value : undefined,
      page: page.value,
      pageSize: pageSize.value,
    })

    if (response.code === 0) {
      // 处理tags字段，确保它是一个数组
      results.value = response.data.list.map((problem) => {
        if (typeof problem.tags === 'string') {
          try {
            problem.tags = JSON.parse(problem.tags)
          } catch (e) {
            problem.tags = []
          }
        } else if (
          Array.isArray(problem.tags) &&
          problem.tags.length === 1 &&
          typeof problem.tags[0] === 'string'
        ) {
          // 处理嵌套数组的情况，如 ["[\"算法\", \"字符串\", \"贪心\"]"]
          try {
            problem.tags = JSON.parse(problem.tags[0])
          } catch (e) {
            problem.tags = []
          }
        }
        return problem
      })
      total.value = response.data.total
    } else {
      error.value = response.msg || '搜索失败'
    }
  } catch (err) {
    error.value = '网络错误，无法搜索'
  } finally {
    loading.value = false
  }
}

// 切换页码获取状态类名
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

// 监听路由参数变化
watch(
  () => route.query.keyword,
  (newKeyword) => {
    if (newKeyword) {
      keyword.value = newKeyword
      handleSearch()
    }
  },
  { immediate: true },
)

// 组件挂载时获取数据
onMounted(() => {
  // 如果路由中没有搜索参数，默认加载题目列表
  if (!route.query.keyword) {
    handleFilterSearch()
  }
})
</script>

<style scoped>
.search-page {
  padding: 40px 0;
  display: flex;
  flex-direction: column;
  gap: 20px;
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

/* 搜索框 */
.search-container {
  padding: 30px;
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.search-input-wrapper {
  display: flex;
  align-items: center;
  max-width: 600px;
  margin: 0 auto;
}

.search-input {
  flex: 1;
  height: 48px;
  padding: 0 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px 0 0 8px;
  font-size: 16px;
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
  height: 48px;
  padding: 0 24px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 0 8px 8px 0;
  font-size: 16px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    background-color: #0e42c5;
  }
}

/* 筛选条件 */
.filter-container {
  padding: 24px;
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.filter-section {
  margin-bottom: 20px;

  h3 {
    font-size: 16px;
    font-weight: 600;
    color: #1d2129;
    margin-bottom: 12px;
  }
}

.tags-container {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.custom-tag-input {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 12px;
  width: 100%;
  max-width: 300px;

  input {
    flex: 1;
    padding: 6px 12px;
    border: 1px solid #e5e6eb;
    border-radius: 16px;
    font-size: 14px;
    outline: none;
    transition: all 0.2s ease;

    &:focus {
      border-color: #165dff;
    }
  }

  .add-tag-btn {
    padding: 6px 12px;
    border: 1px solid #165dff;
    border-radius: 16px;
    background-color: #165dff;
    color: #ffffff;
    font-size: 14px;
    cursor: pointer;
    transition: all 0.2s ease;

    &:hover {
      background-color: #0e42c5;
    }
  }
}

.tag-item {
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 16px;
  font-size: 14px;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    border-color: #165dff;
    color: #165dff;
  }

  &.active {
    background-color: #165dff;
    border-color: #165dff;
    color: #ffffff;
  }
}

.difficulty-container {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.difficulty-item {
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 16px;
  font-size: 14px;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    border-color: #165dff;
    color: #165dff;
  }

  &.active {
    background-color: #165dff;
    border-color: #165dff;
    color: #ffffff;
  }
}

.filter-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  margin-top: 20px;
  padding-top: 20px;
  border-top: 1px solid #e5e6eb;
}

.reset-btn {
  padding: 8px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  background-color: #ffffff;
  color: #4e5969;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    border-color: #165dff;
    color: #165dff;
  }
}

.apply-btn {
  padding: 8px 16px;
  border: 1px solid #165dff;
  border-radius: 6px;
  background-color: #165dff;
  color: #ffffff;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    background-color: #0e42c5;
  }
}

/* 搜索结果 */
.results-container {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  overflow: hidden;
}

.results-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 24px;
  border-bottom: 1px solid #e5e6eb;
  background-color: #f7f8fa;

  h3 {
    font-size: 16px;
    font-weight: 600;
    color: #1d2129;
    margin-bottom: 0;
  }

  .results-info {
    font-size: 14px;
    color: #86909c;
  }
}

.results-list {
  padding: 0 24px;
}

/* 题目项 */
.problem-item {
  display: grid;
  grid-template-columns: 80px 2fr 100px 1.5fr 120px;
  padding: 16px 0;
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

.item.tags {
  flex-wrap: wrap;
  gap: 8px;
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

/* 标签 */
.tag {
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
.error,
.no-results {
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
