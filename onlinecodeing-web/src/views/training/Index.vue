<!-- src/views/training/Index.vue -->
<template>
  <MainLayout>
    <div class="training-page container">
      <div class="page-header">
        <h1>训练</h1>
        <div class="header-controls">
          <Button type="primary" @click="showCreateDialog = true" :disabled="loading">
            + 创建题目组
          </Button>
        </div>
      </div>

      <!-- 创建题目组对话框 -->
      <div v-if="showCreateDialog" class="dialog-overlay" @click="closeCreateDialog">
        <div class="dialog-content" @click.stop>
          <div class="dialog-header">
            <h2>创建题目组</h2>
            <button class="close-btn" @click="closeCreateDialog">×</button>
          </div>
          <div class="dialog-body">
            <!-- 题目组信息 -->
            <div class="form-group">
              <label>题目组名称</label>
              <input type="text" v-model="newProblemSet.name" placeholder="请输入题目组名称" />
            </div>
            <div class="form-group">
              <label>题目组描述</label>
              <textarea
                v-model="newProblemSet.description"
                placeholder="请输入题目组描述"
                rows="3"
              ></textarea>
            </div>

            <!-- 题目选择 -->
            <div class="form-group">
              <label>选择题目</label>
              <div class="problems-selector">
                <div class="available-problems">
                  <div class="problem-search">
                    <input
                      type="text"
                      v-model="problemSearchKeyword"
                      placeholder="搜索题目..."
                      @keyup.enter="searchProblems"
                      :disabled="loadingProblems"
                    />
                    <button class="search-btn" @click="searchProblems" :disabled="loadingProblems">
                      {{ loadingProblems ? '搜索中...' : '搜索' }}
                    </button>
                  </div>
                  <div class="filter-options">
                    <div class="filter-item">
                      <label>难度：</label>
                      <select
                        v-model="selectedDifficulty"
                        @change="searchProblems"
                        :disabled="loadingProblems"
                      >
                        <option value="0">全部</option>
                        <option value="1">入门</option>
                        <option value="2">基础</option>
                        <option value="3">中等</option>
                        <option value="4">进阶</option>
                        <option value="5">困难</option>
                      </select>
                    </div>
                    <div class="filter-item">
                      <label>标签：</label>
                      <div class="tag-input-container">
                        <input
                          type="text"
                          v-model="newTag"
                          placeholder="输入新标签..."
                          @keyup.enter="addNewTag"
                          :disabled="loadingProblems"
                        />
                        <button class="add-tag-btn" @click="addNewTag" :disabled="loadingProblems">
                          添加
                        </button>
                      </div>
                      <div class="tags-container">
                        <span
                          v-for="tag in availableTags"
                          :key="tag"
                          class="tag-item"
                          :class="{ selected: selectedTags.includes(tag) }"
                          @click="toggleTag(tag)"
                          :style="{ cursor: loadingProblems ? 'not-allowed' : 'pointer' }"
                          :disabled="loadingProblems"
                        >
                          {{ tag }}
                        </span>
                      </div>
                    </div>
                    <div v-if="isAdminOrTeacher" class="filter-item">
                      <label for="include-hidden">包含隐藏题目：</label>
                      <input
                        id="include-hidden"
                        type="checkbox"
                        v-model="includeHiddenProblems"
                        @change="searchProblems"
                        :disabled="loadingProblems"
                      />
                    </div>
                  </div>
                  <h3>可选题目</h3>
                  <div class="problems-list">
                    <div v-if="loadingProblems" class="loading-text">加载中...</div>
                    <div v-else-if="availableProblems.length === 0" class="no-problems">
                      没有找到题目
                    </div>
                    <div
                      v-else
                      v-for="problem in availableProblems"
                      :key="problem.id"
                      class="problem-item"
                    >
                      <input
                        type="checkbox"
                        :id="'problem-' + problem.id"
                        :value="problem.id"
                        v-model="selectedProblemIds"
                      />
                      <label :for="'problem-' + problem.id">
                        <span class="problem-id">{{ problem.id }}</span>
                        <span class="problem-title">{{ problem.title }}</span>
                        <span
                          class="problem-difficulty"
                          :class="'difficulty-' + problem.difficulty"
                          >{{ getDifficultyText(problem.difficulty) }}</span
                        >
                      </label>
                    </div>
                  </div>
                  <div class="pagination" v-if="!loadingProblems && availableProblems.length > 0">
                    <button
                      class="page-btn"
                      :disabled="currentPage === 1 || loadingProblems"
                      @click="changePage(currentPage - 1)"
                    >
                      上一页
                    </button>
                    <span class="page-info">第 {{ currentPage }} 页</span>
                    <button
                      class="page-btn"
                      @click="changePage(currentPage + 1)"
                      :disabled="loadingProblems"
                    >
                      下一页
                    </button>
                  </div>
                </div>
                <div class="selected-problems">
                  <h3>已选题目（拖动调整顺序）</h3>
                  <div class="problems-list" @dragover.prevent>
                    <div
                      v-for="(problem, index) in selectedProblems"
                      :key="problem.id"
                      class="problem-item selected"
                      :draggable="!loadingProblems"
                      @dragstart="dragStart(index)"
                      @dragover.prevent
                      @drop="drop(index)"
                      :style="{ cursor: loadingProblems ? 'not-allowed' : 'move' }"
                    >
                      <span
                        class="drag-handle"
                        :style="{ cursor: loadingProblems ? 'not-allowed' : 'grab' }"
                        >⋮⋮</span
                      >
                      <span class="problem-order">{{ index + 1 }}</span>
                      <span class="problem-title">{{ problem.title }}</span>
                      <span
                        class="problem-difficulty"
                        :class="'difficulty-' + problem.difficulty"
                        >{{ getDifficultyText(problem.difficulty) }}</span
                      >
                      <button
                        class="remove-btn"
                        @click="removeProblem(problem.id)"
                        :disabled="loadingProblems"
                      >
                        ×
                      </button>
                    </div>
                    <div v-if="selectedProblems.length === 0" class="no-problems">
                      请从左侧选择题目
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
          <div
            class="message-container"
            v-if="errorMessage"
            :class="{ 'success-message': errorMessage.includes('成功') }"
            v-html="errorMessage"
          ></div>
          <div class="dialog-footer">
            <Button @click="closeCreateDialog" :disabled="loadingCreate">取消</Button>
            <Button type="primary" @click="submitCreateProblemSet" :disabled="loadingCreate">
              {{ loadingCreate ? '创建中...' : '创建' }}
            </Button>
          </div>
        </div>
      </div>

      <!-- 训练概览 -->
      <div class="overview-section">
        <div v-if="loading" class="loading-text">加载中...</div>
        <div v-else class="stats-grid">
          <div class="stat-card">
            <div class="stat-icon">📚</div>
            <div class="stat-content">
              <div class="stat-value">{{ totalProblemSets }}</div>
              <div class="stat-label">我的题目组</div>
            </div>
          </div>
          <div class="stat-card">
            <div class="stat-icon">🎯</div>
            <div class="stat-content">
              <div class="stat-value">{{ completedProblemSets }}</div>
              <div class="stat-label">已完成题目组</div>
            </div>
          </div>
          <div class="stat-card">
            <div class="stat-icon">📊</div>
            <div class="stat-content">
              <div class="stat-value">{{ totalProblems }}</div>
              <div class="stat-label">已完成题目</div>
            </div>
          </div>
        </div>
      </div>

      <!-- 进行中的题目组 -->
      <div class="active-plans-section">
        <h2>进行中的题目组</h2>
        <div class="plans-list">
          <div v-if="loading" class="loading-text">加载中...</div>
          <div v-else-if="validActiveProblemSets.length === 0" class="empty-state">
            暂无进行中的题目组
          </div>
          <div v-else>
            <router-link
              v-for="set in validActiveProblemSets"
              :key="set.id"
              :to="`/training/set/${set.id}`"
              class="plan-card"
            >
              <div class="plan-header">
                <div class="plan-info">
                  <h3>{{ set.name }}</h3>
                  <p>{{ set.description }}</p>
                </div>
              </div>
              <div class="plan-details">
                <div class="plan-meta">
                  <span class="meta-item">
                    <span class="meta-icon">📝</span>
                    {{ set.totalProblems || 0 }} 题
                  </span>
                </div>
              </div>
            </router-link>
          </div>
        </div>
      </div>

      <!-- 题目组 -->
      <div class="recommended-plans-section">
        <h2>题目组</h2>
        <div class="plans-grid">
          <div v-if="loading || loadingRecommended" class="loading-text">加载中...</div>
          <div v-else-if="recommendedProblemSets.length === 0" class="empty-state">暂无题目组</div>
          <router-link
            v-else
            v-for="set in recommendedProblemSets"
            :key="set.id"
            :to="`/training/set/${set.id}`"
            class="plan-card recommended"
          >
            <div class="plan-info">
              <h3>{{ set.name }}</h3>
              <p>{{ set.description }}</p>
            </div>
            <div class="plan-meta">
              <span class="meta-item">
                <span class="meta-icon">📝</span>
                {{ set.totalProblems || 0 }} 题
              </span>
            </div>
          </router-link>
        </div>
        <div
          v-if="!loading && !loadingRecommended && recommendedProblemSets.length > 0"
          class="pagination"
        >
          <button
            class="page-btn"
            :disabled="currentPage === 1 || loadingRecommended"
            @click="loadRecommendedProblemSets(currentPage - 1, 10)"
          >
            上一页
          </button>
          <span class="page-info">
            第 {{ currentPage }} 页 / 共 {{ Math.ceil(totalProblemSetsCount / 10) }} 页
          </span>
          <button
            class="page-btn"
            :disabled="currentPage * 10 >= totalProblemSetsCount || loadingRecommended"
            @click="loadRecommendedProblemSets(currentPage + 1, 10)"
          >
            下一页
          </button>
        </div>
      </div>

      <!-- 训练历史 -->
      <div class="history-section">
        <h2>训练历史</h2>
        <div class="history-list">
          <div v-if="loading" class="loading-text">加载中...</div>
          <div v-else-if="trainingHistory.length === 0" class="empty-state">暂无训练历史</div>
          <div v-else v-for="record in trainingHistory" :key="record.id" class="history-item">
            <div class="history-date">{{ record.date }}</div>
            <div class="history-activity">
              <span class="activity-icon">📚</span>
              <span class="activity-text">{{ record.problemSetName }}</span>
            </div>
            <div
              class="history-status"
              :class="record.status === 2 ? 'completed' : record.status === 1 ? 'in-progress' : ''"
            >
              {{ record.status === 2 ? '已完成' : record.status === 1 ? '进行中' : '未开始' }}
            </div>
          </div>
        </div>
      </div>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, computed, onMounted, watch } from 'vue'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import { apiService } from '@/services/api'

// 响应式数据
const totalProblemSets = ref(0)
const completedProblemSets = ref(0)
const totalProblems = ref(0)

const activeProblemSets = ref([])
const recommendedProblemSets = ref([])

// 用户信息
const userInfo = ref({})
const isAdminOrTeacher = computed(() => {
  return userInfo.value.role === 'admin' || userInfo.value.role === 'teacher'
})
const includeHiddenProblems = ref(false)
const trainingHistory = ref([])

// 过滤有效的进行中的题目组并去重
const validActiveProblemSets = computed(() => {
  // 放宽过滤条件：只要有 set 存在就保留
  const validSets = activeProblemSets.value.filter((set) => set)
  // 使用 Map 基于 id 去重
  const uniqueMap = new Map()
  validSets.forEach((set) => {
    // 使用 set.id 作为 key，如果没有 id 则使用 name 作为备选 key
    const key = set.id !== undefined && set.id !== null ? set.id : set.name
    if (key !== undefined && key !== null && !uniqueMap.has(key)) {
      uniqueMap.set(key, set)
    }
  })
  return Array.from(uniqueMap.values())
})

// 加载状态
const loading = ref(false)
const loadingCreate = ref(false)
const loadingProblems = ref(false)
const loadingRecommended = ref(false)

// 创建题目组相关数据
const showCreateDialog = ref(false)
const newProblemSet = ref({
  name: '',
  description: '',
})
const selectedProblemIds = ref([])
const selectedProblems = ref([])
const errorMessage = ref('')
const availableProblems = ref([])
const draggedIndex = ref(-1)
const problemSearchKeyword = ref('')
const currentPage = ref(1)
const totalProblemSetsCount = ref(0)
const selectedTags = ref([])
const selectedDifficulty = ref(0)
const availableTags = ref([
  '数组',
  '字符串',
  '动态规划',
  '贪心',
  '回溯',
  '二分查找',
  '链表',
  '树',
  '图',
  '排序',
])
const newTag = ref('')

// 加载训练数据
const loadTrainingData = async () => {
  try {
    loading.value = true
    loadingRecommended.value = true
    errorMessage.value = ''

    // 获取训练概览
    try {
      const overviewResponse = await apiService.getTrainingOverview()
      if (overviewResponse && overviewResponse.code === 0 && overviewResponse.data) {
        totalProblemSets.value = overviewResponse.data.totalProblemSets || 0
        completedProblemSets.value = overviewResponse.data.completedProblemSets || 0
        totalProblems.value = overviewResponse.data.totalProblems || 0
      }
    } catch (err) {
      console.error('获取概览失败:', err)
    }

    // 获取进行中的题目组
    try {
      const activeSetsResponse = await apiService.getActiveProblemSets()
      if (activeSetsResponse && activeSetsResponse.code === 0) {
        let data = activeSetsResponse.data

        // 处理嵌套数组 [[{...}]]
        if (Array.isArray(data)) {
          if (data.length > 0 && Array.isArray(data[0])) {
            // 嵌套数组，取第一个元素
            activeProblemSets.value = data[0]
          } else {
            // 普通数组
            activeProblemSets.value = data
          }
        }
      }
    } catch (err) {
      console.error('获取进行中的题目组失败:', err)
    }

    // 获取推荐题目组
    try {
      loadingRecommended.value = true
      const recommendedSetsResponse = await apiService.getRecommendedProblemSets(
        currentPage.value,
        10,
      )
      if (
        recommendedSetsResponse &&
        recommendedSetsResponse.code === 0 &&
        recommendedSetsResponse.data
      ) {
        recommendedProblemSets.value = Array.isArray(recommendedSetsResponse.data.list)
          ? recommendedSetsResponse.data.list
          : []
        totalProblemSetsCount.value = recommendedSetsResponse.data.total || 0
      }
    } catch (err) {
      console.error('获取推荐题目组失败:', err)
    } finally {
      loadingRecommended.value = false
    }

    // 获取训练历史
    try {
      const historyResponse = await apiService.getTrainingHistory()
      if (historyResponse && historyResponse.code === 0) {
        let data = historyResponse.data

        // 处理嵌套数组 [[{...}]]
        if (Array.isArray(data)) {
          if (data.length > 0 && Array.isArray(data[0])) {
            // 嵌套数组，取第一个元素
            trainingHistory.value = data[0]
          } else {
            // 普通数组
            trainingHistory.value = data
          }
        }
      }
    } catch (err) {
      console.error('获取训练历史失败:', err)
    }
  } catch (error) {
    console.error('加载训练数据失败:', error)
  } finally {
    loading.value = false
    loadingRecommended.value = false
  }
}

// 方法
const loadAvailableProblems = async () => {
  searchProblems()
}

const searchProblems = async () => {
  try {
    loadingProblems.value = true
    errorMessage.value = ''
    const response = await apiService.searchProblems({
      keyword: problemSearchKeyword.value,
      tags: selectedTags.value.length > 0 ? selectedTags.value : undefined,
      difficulty: selectedDifficulty.value > 0 ? selectedDifficulty.value : undefined,
      page: currentPage.value,
      pageSize: 20,
      includeHidden: includeHiddenProblems.value,
    })
    if (response && response.code === 0 && response.data) {
      availableProblems.value = Array.isArray(response.data.list)
        ? response.data.list.map((problem) => ({
            id: problem.id || 0,
            title: problem.title || '',
            difficulty: problem.difficulty || 1,
            tags: Array.isArray(problem.tags) ? problem.tags : [],
          }))
        : []
    } else {
      console.error('搜索题目失败: 响应格式错误')
      availableProblems.value = []
      errorMessage.value = '搜索题目失败: 响应格式错误'
    }
  } catch (error) {
    console.error('搜索题目失败:', error)
    availableProblems.value = []
    errorMessage.value = '搜索题目失败: 网络错误'
  } finally {
    loadingProblems.value = false
  }
}

const changePage = (page) => {
  currentPage.value = page
  searchProblems()
}

const toggleTag = (tag) => {
  const index = selectedTags.value.indexOf(tag)
  if (index > -1) {
    selectedTags.value.splice(index, 1)
  } else {
    selectedTags.value.push(tag)
  }
  searchProblems()
}

const dragStart = (index) => {
  draggedIndex.value = index
}

const drop = (targetIndex) => {
  if (draggedIndex.value === -1 || draggedIndex.value === targetIndex) return

  const draggedProblem = selectedProblems.value[draggedIndex.value]
  selectedProblems.value.splice(draggedIndex.value, 1)
  selectedProblems.value.splice(targetIndex, 0, draggedProblem)

  // 更新selectedProblemIds以保持同步
  selectedProblemIds.value = selectedProblems.value.map((p) => p.id)

  draggedIndex.value = -1
}

const addNewTag = () => {
  const tag = newTag.value.trim()
  if (tag && !availableTags.value.includes(tag)) {
    availableTags.value.push(tag)
    selectedTags.value.push(tag)
    newTag.value = ''
    searchProblems()
  }
}

const openCreateDialog = async () => {
  // 重置表单
  newProblemSet.value = {
    name: '',
    description: '',
  }
  selectedProblemIds.value = []
  selectedProblems.value = []
  errorMessage.value = ''

  await loadAvailableProblems()
  showCreateDialog.value = true
}

const closeCreateDialog = () => {
  showCreateDialog.value = false
  // 重置表单
  newProblemSet.value = {
    name: '',
    description: '',
  }
  selectedProblemIds.value = []
  selectedProblems.value = []
  errorMessage.value = ''
}

const removeProblem = (problemId) => {
  const index = selectedProblemIds.value.indexOf(problemId)
  if (index > -1) {
    selectedProblemIds.value.splice(index, 1)
  }
  const problemIndex = selectedProblems.value.findIndex((p) => p.id === problemId)
  if (problemIndex > -1) {
    selectedProblems.value.splice(problemIndex, 1)
  }
}

const getProblemTitle = (problemId) => {
  const problem = availableProblems.value.find((p) => p.id === problemId)
  return problem ? problem.title : '未知题目'
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

const submitCreateProblemSet = async () => {
  try {
    errorMessage.value = ''

    if (!newProblemSet.value.name.trim()) {
      errorMessage.value = '请输入题目组名称'
      return
    }
    if (selectedProblems.value.length === 0) {
      errorMessage.value = '请至少选择一个题目'
      return
    }

    const problemIds = selectedProblems.value.map((p) => p.id)

    loadingCreate.value = true
    const response = await apiService.createProblemSet({
      name: newProblemSet.value.name,
      description: newProblemSet.value.description,
      problemIds: problemIds,
    })

    if (response && response.code === 0) {
      errorMessage.value = '<span style="color: #52c41a;">创建题目组成功</span>'
      setTimeout(() => {
        closeCreateDialog()
        loadTrainingData()
      }, 1500)
    } else {
      errorMessage.value = '创建题目组失败: ' + (response?.msg || '未知错误')
    }
  } catch (error) {
    console.error('创建题目组失败:', error)
    errorMessage.value = '创建题目组失败: 网络错误'
  } finally {
    loadingCreate.value = false
  }
}

const createProblemSet = async () => {
  await openCreateDialog()
}

const loadRecommendedProblemSets = async (page, pageSize) => {
  try {
    loadingRecommended.value = true
    errorMessage.value = ''
    const recommendedSetsResponse = await apiService.getRecommendedProblemSets(page, pageSize)
    if (
      recommendedSetsResponse &&
      recommendedSetsResponse.code === 0 &&
      recommendedSetsResponse.data
    ) {
      recommendedProblemSets.value = Array.isArray(recommendedSetsResponse.data.list)
        ? recommendedSetsResponse.data.list.map((set) => ({
            id: set.id || 0,
            name: set.name || '',
            description: set.description || '',
            totalProblems: set.totalProblems || 0,
          }))
        : []
      totalProblemSetsCount.value =
        typeof recommendedSetsResponse.data.total === 'number'
          ? recommendedSetsResponse.data.total
          : 0
      currentPage.value = typeof page === 'number' ? page : 1
    } else {
      console.error('获取题目组失败: 响应格式错误')
      recommendedProblemSets.value = []
      totalProblemSetsCount.value = 0
      errorMessage.value = '获取题目组失败: 响应格式错误'
    }
  } catch (error) {
    console.error('获取题目组失败:', error)
    recommendedProblemSets.value = []
    totalProblemSetsCount.value = 0
    errorMessage.value = '获取题目组失败: 网络错误'
  } finally {
    loadingRecommended.value = false
  }
}

// 监听selectedProblemIds变化，更新selectedProblems
watch(
  selectedProblemIds,
  (newIds) => {
    selectedProblems.value = newIds.map((id, index) => {
      const problem = availableProblems.value.find((p) => p.id === id)
      return {
        id,
        title: problem ? problem.title : '未知题目',
        difficulty: problem ? problem.difficulty : 1,
        order: index + 1,
      }
    })
  },
  { deep: true },
)

// 组件挂载时获取数据
onMounted(() => {
  // 获取用户信息
  const info = localStorage.getItem('userInfo')
  if (info) {
    userInfo.value = JSON.parse(info)
  }
  loadTrainingData()
})
</script>

<style scoped>
.training-page {
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

.header-controls {
  display: flex;
  align-items: center;
  gap: 20px;
}

.contest-toggle {
  display: flex;
  align-items: center;
}

.toggle-label {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;
}

.toggle-label:hover {
  color: #165dff;
}

.toggle-label input[type='checkbox'] {
  width: 16px;
  height: 16px;
  cursor: pointer;
  accent-color: #165dff;
}

/* 训练概览 */
.overview-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
  gap: 20px;
}

.stat-card {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 20px;
  background-color: #f7f8fa;
  border-radius: 8px;
  transition: all 0.2s ease;
}

.stat-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.stat-icon {
  font-size: 28px;
}

.stat-content {
  flex: 1;
}

.stat-value {
  font-size: 24px;
  font-weight: 600;
  color: #165dff;
  margin-bottom: 4px;
}

.stat-label {
  font-size: 14px;
  color: #4e5969;
}

/* 进行中的训练计划 */
.active-plans-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.active-plans-section h2 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 20px;
}

.plans-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.plan-card {
  padding: 20px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  transition: all 0.2s ease;
  position: relative;
  text-decoration: none;
  color: inherit;
  display: block;
}

.plan-card:hover {
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  transform: translateY(-2px);
}

.plan-card.recommended {
  border-color: #165dff;
}

.plan-badge {
  position: absolute;
  top: -8px;
  right: 20px;
  background-color: #165dff;
  color: #ffffff;
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.plan-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 16px;
  gap: 20px;
}

.plan-info {
  flex: 1;
}

.plan-info h3 {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 8px;
}

.plan-info p {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.5;
  margin-bottom: 0;
}

.plan-progress {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 8px;
  min-width: 120px;
}

.progress-bar {
  width: 100%;
  height: 8px;
  background-color: #f5f5f5;
  border-radius: 4px;
  overflow: hidden;
}

.progress-fill {
  height: 100%;
  background-color: #165dff;
  border-radius: 4px;
  transition: width 0.3s ease;
}

.progress-text {
  font-size: 14px;
  font-weight: 500;
  color: #165dff;
}

.plan-details {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding-top: 16px;
  border-top: 1px solid #e5e6eb;
}

.plan-meta {
  display: flex;
  gap: 16px;
}

.meta-item {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 14px;
  color: #4e5969;
}

.meta-icon {
  font-size: 16px;
}

/* 推荐训练计划 */
.recommended-plans-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.recommended-plans-section h2 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 20px;
}

.plans-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
}

/* 训练历史 */
.history-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.history-section h2 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 20px;
}

.history-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.history-item {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 12px;
  border-radius: 8px;
  background-color: #f7f8fa;
  transition: all 0.2s ease;
}

.history-item:hover {
  background-color: #e8f0ff;
}

.history-date {
  font-size: 14px;
  color: #86909c;
  min-width: 100px;
}

.history-activity {
  flex: 1;
  display: flex;
  align-items: center;
  gap: 8px;
}

.activity-icon {
  font-size: 16px;
}

.activity-text {
  font-size: 14px;
  color: #1d2129;
}

.history-status {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.history-status.completed {
  background-color: #e8f5e8;
  color: #00b42a;
}

.history-status.in-progress {
  background-color: #fff3e0;
  color: #ff7d00;
}

.history-status.pending {
  background-color: #f5f5f5;
  color: #86909c;
}

/* 创建题目组对话框 */
.dialog-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, 0.5) !important;
  display: flex !important;
  align-items: center !important;
  justify-content: center !important;
  z-index: 9999 !important;
  margin: 0 !important;
  padding: 0 !important;
}

.dialog-content {
  background-color: #ffffff !important;
  border-radius: 12px !important;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2) !important;
  width: 90% !important;
  max-width: 800px !important;
  max-height: 80vh !important;
  overflow-y: auto !important;
  position: relative !important;
  margin: 0 !important;
  padding: 0 !important;
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
  margin-bottom: 20px;
}

.form-group label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-group input,
.form-group textarea,
.form-group select {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.form-group input:focus,
.form-group textarea:focus,
.form-group select:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.form-group textarea {
  resize: vertical;
}

.time-fields {
  display: flex;
  gap: 20px;
}

.time-field {
  flex: 1;
}

.time-field input {
  width: 100%;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.time-field input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

@media (max-width: 768px) {
  .time-fields {
    flex-direction: column;
  }
}

.problems-selector {
  display: flex;
  gap: 20px;
  margin-top: 12px;
}

.available-problems,
.selected-problems {
  flex: 1;
  max-height: 300px;
  overflow-y: auto;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  padding: 12px;
  background-color: #f7f8fa;
}

.available-problems h3,
.selected-problems h3 {
  font-size: 14px;
  font-weight: 600;
  color: #1d2129;
  margin-top: 0;
  margin-bottom: 12px;
}

.problem-search {
  display: flex;
  gap: 8px;
  margin-bottom: 12px;
}

.problem-search input {
  flex: 1;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
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

.filter-options {
  margin-bottom: 12px;
  padding: 12px;
  background-color: #f7f8fa;
  border-radius: 6px;
  border: 1px solid #e5e6eb;
}

.filter-item {
  margin-bottom: 8px;
}

.filter-item label {
  display: inline-block;
  margin-right: 12px;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  width: 100px;
}

.filter-item label[for*='hidden'] {
  width: auto;
  white-space: nowrap;
}

.filter-item select {
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  font-size: 14px;
  width: 120px;
}

.filter-item input[type='checkbox'] {
  width: 16px;
  height: 16px;
  margin-top: 2px;
  cursor: pointer;
  accent-color: #165dff;
}

.tags-container {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 4px;
}

.tag-item {
  padding: 4px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 12px;
  font-size: 12px;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;
  background-color: #ffffff;
}

.tag-item:hover {
  border-color: #165dff;
  color: #165dff;
}

.tag-item.selected {
  border-color: #165dff;
  background-color: #165dff;
  color: #ffffff;
}

.tag-input-container {
  display: flex;
  gap: 8px;
  margin-bottom: 12px;
}

.tag-input-container input {
  flex: 1;
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  font-size: 14px;
}

.add-tag-btn {
  padding: 6px 12px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 4px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.add-tag-btn:hover {
  background-color: #0e42c2;
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

.drag-handle {
  font-size: 18px;
  color: #86909c;
  cursor: grab;
  margin-right: 12px;
  user-select: none;
}

.drag-handle:active {
  cursor: grabbing;
}

.problem-item.selected {
  cursor: move;
}

.problem-item.selected:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(22, 93, 255, 0.15);
}

.loading-text,
.no-problems {
  text-align: center;
  padding: 20px;
  color: #86909c;
  font-size: 14px;
}

.pagination {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
  margin-top: 12px;
  padding-top: 12px;
  border-top: 1px solid #e5e6eb;
}

.page-btn {
  padding: 4px 12px;
  border: 1px solid #e5e6eb;
  background-color: #ffffff;
  border-radius: 4px;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.page-btn:hover:not(:disabled) {
  border-color: #165dff;
  color: #165dff;
}

.page-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.page-info {
  font-size: 14px;
  color: #86909c;
}

.problems-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.problem-item {
  display: flex;
  align-items: center;
  padding: 12px;
  background-color: #ffffff;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  transition: all 0.2s ease;
  margin-bottom: 8px;
}

.problem-item:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.problem-item.selected {
  border-color: #165dff;
  background-color: #e8f0ff;
  box-shadow: 0 2px 8px rgba(22, 93, 255, 0.15);
}

.problem-item input[type='checkbox'] {
  margin-right: 16px;
  width: 16px;
  height: 16px;
  cursor: pointer;
}

.problem-item label {
  flex: 1;
  display: flex;
  align-items: center;
  gap: 16px;
  margin: 0;
  cursor: pointer;
  font-size: 14px;
}

.problem-id {
  font-size: 14px;
  font-weight: 600;
  color: #165dff;
  min-width: 80px;
}

.problem-title {
  flex: 1;
  font-size: 14px;
  color: #1d2129;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.problem-order {
  font-size: 14px;
  font-weight: 600;
  color: #165dff;
  min-width: 40px;
  text-align: center;
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

.remove-btn {
  background: none;
  border: none;
  font-size: 18px;
  color: #f53f3f;
  cursor: pointer;
  padding: 0;
  width: 24px;
  height: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 4px;
  transition: all 0.2s ease;
}

.remove-btn:hover {
  background-color: rgba(245, 63, 63, 0.1);
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

/* 响应式设计 */
@media (max-width: 768px) {
  .problems-selector {
    flex-direction: column;
  }

  .available-problems,
  .selected-problems {
    max-height: 200px;
  }
}
</style>
