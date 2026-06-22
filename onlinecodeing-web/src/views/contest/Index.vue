<!-- src/views/contest/Index.vue -->
<template>
  <MainLayout>
    <div class="contest-page container">
      <div class="page-header">
        <h1>竞赛</h1>
        <div class="header-controls">
          <Button type="primary" @click="showCreateDialog = true" :disabled="loading">
            + 创建竞赛
          </Button>
        </div>
      </div>

      <!-- 创建竞赛对话框 -->
      <div v-if="showCreateDialog" class="dialog-overlay" @click="closeCreateDialog">
        <div class="dialog-content" @click.stop>
          <div class="dialog-header">
            <h2>创建竞赛</h2>
            <button class="close-btn" @click="closeCreateDialog">×</button>
          </div>
          <div class="dialog-body">
            <!-- 竞赛信息 -->
            <div class="form-group">
              <label>竞赛名称</label>
              <input type="text" v-model="newContest.name" placeholder="请输入竞赛名称" />
            </div>
            <div class="form-group">
              <label>竞赛描述</label>
              <textarea
                v-model="newContest.description"
                placeholder="请输入竞赛描述"
                rows="3"
              ></textarea>
            </div>

            <!-- 时间设置 -->
            <div class="form-group">
              <label>开始时间</label>
              <input
                type="datetime-local"
                v-model="newContest.startTime"
                placeholder="请选择开始时间"
              />
            </div>
            <div class="form-group">
              <label>结束时间</label>
              <input
                type="datetime-local"
                v-model="newContest.endTime"
                placeholder="请选择结束时间"
              />
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
            <Button type="primary" @click="submitCreateContest" :disabled="loadingCreate">
              {{ loadingCreate ? '创建中...' : '创建' }}
            </Button>
          </div>
        </div>
      </div>

      <!-- 进行中的竞赛 -->
      <div class="active-plans-section">
        <h2>进行中的竞赛</h2>
        <div class="plans-list">
          <div v-if="loading" class="loading-text">加载中...</div>
          <div v-else-if="validActiveContests.length === 0" class="empty-state">
            暂无进行中的竞赛
          </div>
          <div v-else>
            <router-link
              v-for="set in validActiveContests"
              :key="set.id"
              :to="`/training/set/${set.id}?isContest=1`"
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

      <!-- 竞赛列表 -->
      <div class="recommended-plans-section">
        <h2>竞赛列表</h2>
        <div class="contest-filter">
          <div class="filter-row">
            <select
              v-model="contestStatusFilter"
              class="status-filter"
              @change="loadRecommendedContests(1, 10)"
            >
              <option value="all">全部状态</option>
              <option value="ongoing">进行中</option>
              <option value="upcoming">未开始</option>
              <option value="ended">已结束</option>
            </select>
          </div>
        </div>
        <div class="plans-grid">
          <div v-if="loading || loadingRecommended" class="loading-text">加载中...</div>
          <div v-else-if="recommendedContests.length === 0" class="empty-state">暂无竞赛</div>
          <router-link
            v-else
            v-for="set in recommendedContests"
            :key="set.id"
            :to="`/training/set/${set.id}?isContest=1`"
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
          v-if="!loading && !loadingRecommended && recommendedContests.length > 0"
          class="pagination"
        >
          <button
            class="page-btn"
            :disabled="currentPage === 1 || loadingRecommended"
            @click="loadRecommendedContests(currentPage - 1, 10)"
          >
            上一页
          </button>
          <span class="page-info">
            第 {{ currentPage }} 页 / 共 {{ Math.ceil(totalContestsCount / 10) }} 页
          </span>
          <button
            class="page-btn"
            :disabled="currentPage * 10 >= totalContestsCount || loadingRecommended"
            @click="loadRecommendedContests(currentPage + 1, 10)"
          >
            下一页
          </button>
        </div>
      </div>

      <!-- 竞赛提交历史 -->
      <div class="history-section">
        <h2>竞赛提交历史</h2>
        <div class="history-list">
          <div v-if="loading" class="loading-text">加载中...</div>
          <div v-else-if="contestHistory.length === 0" class="empty-state">暂无竞赛历史</div>
          <div v-else v-for="record in contestHistory" :key="record.id" class="history-item">
            <div class="history-date">{{ record.date }}</div>
            <div class="history-activity">
              <span class="activity-icon">🏆</span>
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

        <!-- 竞赛历史分页 -->
        <div v-if="!loadingHistory && !loading && allContestHistory.length > 0" class="pagination">
          <button
            class="page-btn"
            :disabled="currentHistoryPage === 1"
            @click="changeHistoryPage(currentHistoryPage - 1)"
          >
            上一页
          </button>
          <span class="page-info">
            第 {{ currentHistoryPage }} 页 / 共
            {{ Math.ceil(allContestHistory.length / historyPageSize) }} 页
          </span>
          <button
            class="page-btn"
            :disabled="currentHistoryPage * historyPageSize >= allContestHistory.length"
            @click="changeHistoryPage(currentHistoryPage + 1)"
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
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import { apiService } from '@/services/api'

// 响应式数据
const activeContests = ref([])
const recommendedContests = ref([])
const contestHistory = ref([])
const allContestHistory = ref([])
const currentHistoryPage = ref(1)
const historyPageSize = ref(10)
const loadingHistory = ref(false)

// 用户信息
const userInfo = ref({})
const isAdminOrTeacher = computed(() => {
  return userInfo.value.role === 'admin' || userInfo.value.role === 'teacher'
})
const includeHiddenProblems = ref(false)

// 过滤有效的进行中的竞赛并去重
const validActiveContests = computed(() => {
  console.log('去重前的 activeContests:', activeContests.value)
  // 放宽过滤条件：只要有 set 存在就保留
  const validSets = activeContests.value.filter((set) => set)
  console.log('过滤后的 validSets:', validSets)
  // 使用 Map 基于 id 去重
  const uniqueMap = new Map()
  validSets.forEach((set) => {
    // 使用 set.id 作为 key，如果没有 id 则使用 name 作为备选 key
    const key = set.id !== undefined && set.id !== null ? set.id : set.name
    if (key !== undefined && key !== null && !uniqueMap.has(key)) {
      uniqueMap.set(key, set)
    }
  })
  const result = Array.from(uniqueMap.values())
  console.log('去重后的结果:', result)
  return result
})

// 加载状态
const loading = ref(false)
const loadingCreate = ref(false)
const loadingProblems = ref(false)
const loadingRecommended = ref(false)

// 生成默认时间
const getDefaultDateTime = () => {
  const now = new Date()
  // 开始时间：一个星期后的此时
  const startTime = new Date(now.getTime() + 7 * 24 * 60 * 60 * 1000)
  // 结束时间：开始时间后5小时
  const endTime = new Date(startTime.getTime() + 5 * 60 * 60 * 1000)

  // 格式化为 datetime-local 格式 (YYYY-MM-DDTHH:mm)
  const formatForInput = (date) => {
    const year = date.getFullYear()
    const month = String(date.getMonth() + 1).padStart(2, '0')
    const day = String(date.getDate()).padStart(2, '0')
    const hours = String(date.getHours()).padStart(2, '0')
    const minutes = String(date.getMinutes()).padStart(2, '0')
    return `${year}-${month}-${day}T${hours}:${minutes}`
  }

  return {
    startTime: formatForInput(startTime),
    endTime: formatForInput(endTime),
  }
}

// 创建竞赛相关数据
const showCreateDialog = ref(false)
const defaultTimes = getDefaultDateTime()
const newContest = ref({
  name: '',
  description: '',
  startTime: defaultTimes.startTime,
  endTime: defaultTimes.endTime,
})
const selectedProblemIds = ref([])
const selectedProblems = ref([])
const errorMessage = ref('')
const availableProblems = ref([])
const draggedIndex = ref(-1)
const problemSearchKeyword = ref('')
const currentPage = ref(1)
const totalContestsCount = ref(0)
const selectedTags = ref([])
const selectedDifficulty = ref(0)
const contestStatusFilter = ref('all')
const contestSearchKeyword = ref('')
const allContests = ref([])
const isLoadingMore = ref(false)
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

// 格式化时间为 API 要求的格式
const formatDateTime = (dateTimeStr) => {
  if (!dateTimeStr) return ''
  // 将 datetime-local 格式 (YYYY-MM-DDTHH:mm) 转换为 YYYY-MM-DD HH:mm:ss
  return dateTimeStr.replace('T', ' ') + ':00'
}

// 加载竞赛数据
const loadContestData = async () => {
  try {
    loading.value = true
    loadingRecommended.value = true
    errorMessage.value = ''

    // 获取进行中的竞赛
    try {
      const activeSetsResponse = await apiService.getActiveProblemSets(1)
      console.log('进行中的竞赛 API 返回:', activeSetsResponse)
      if (activeSetsResponse && activeSetsResponse.code === 0) {
        let data = activeSetsResponse.data

        // 处理嵌套数组 [[{...}]]
        if (Array.isArray(data)) {
          if (data.length > 0 && Array.isArray(data[0])) {
            // 嵌套数组，取第一个元素
            activeContests.value = data[0]
          } else {
            // 普通数组
            activeContests.value = data
          }
        }
        console.log('处理后的 activeContests:', activeContests.value)
      }
    } catch (err) {
      console.error('获取进行中的竞赛失败:', err)
    }

    // 获取推荐竞赛
    try {
      loadingRecommended.value = true
      const recommendedSetsResponse = await apiService.getRecommendedProblemSets(
        currentPage.value,
        10,
        1,
      )
      if (
        recommendedSetsResponse &&
        recommendedSetsResponse.code === 0 &&
        recommendedSetsResponse.data
      ) {
        recommendedContests.value = Array.isArray(recommendedSetsResponse.data.list)
          ? recommendedSetsResponse.data.list
          : []
        totalContestsCount.value = recommendedSetsResponse.data.total || 0
      }
    } catch (err) {
      console.error('获取推荐竞赛失败:', err)
    } finally {
      loadingRecommended.value = false
    }

    // 获取竞赛历史
    try {
      loadingHistory.value = true
      const historyResponse = await apiService.getTrainingHistory(1)
      console.log('竞赛历史 API 返回:', historyResponse)
      if (historyResponse && historyResponse.code === 0) {
        let data = historyResponse.data
        console.log('竞赛历史原始数据:', data)

        // 处理嵌套数组 [[{...}]]
        if (Array.isArray(data)) {
          if (data.length > 0 && Array.isArray(data[0])) {
            // 嵌套数组，取第一个元素
            allContestHistory.value = data[0]
          } else {
            // 普通数组
            allContestHistory.value = data
          }
        }
        console.log('处理后的 allContestHistory:', allContestHistory.value)

        // 初始化分页
        currentHistoryPage.value = 1
        updateContestHistoryPage()
      }
    } catch (err) {
      console.error('获取竞赛历史失败:', err)
    } finally {
      loadingHistory.value = false
    }
  } catch (error) {
    console.error('加载竞赛数据失败:', error)
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

  // 更新selecteProblemIds以保持同步
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
  // 获取默认时间
  const defaultTimes = getDefaultDateTime()
  // 重置表单
  newContest.value = {
    name: '',
    description: '',
    startTime: defaultTimes.startTime,
    endTime: defaultTimes.endTime,
  }
  selectedProblemIds.value = []
  selectedProblems.value = []
  errorMessage.value = ''

  await loadAvailableProblems()
  showCreateDialog.value = true
}

const closeCreateDialog = () => {
  showCreateDialog.value = false
  // 获取默认时间
  const defaultTimes = getDefaultDateTime()
  // 重置表单
  newContest.value = {
    name: '',
    description: '',
    startTime: defaultTimes.startTime,
    endTime: defaultTimes.endTime,
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

const submitCreateContest = async () => {
  try {
    errorMessage.value = ''

    if (!newContest.value.name.trim()) {
      errorMessage.value = '请输入竞赛名称'
      return
    }
    if (!newContest.value.startTime) {
      errorMessage.value = '请选择开始时间'
      return
    }
    if (!newContest.value.endTime) {
      errorMessage.value = '请选择结束时间'
      return
    }
    if (selectedProblems.value.length === 0) {
      errorMessage.value = '请至少选择一个题目'
      return
    }

    const problemIds = selectedProblems.value.map((p) => p.id)

    const formattedStartTime = formatDateTime(newContest.value.startTime)
    const formattedEndTime = formatDateTime(newContest.value.endTime)

    // 调试信息 - 打印发送给后端的数据
    const requestData = {
      name: newContest.value.name,
      description: newContest.value.description,
      problemIds: problemIds,
      isContest: 1,
    }

    // 只在时间有值时才添加到请求中
    if (formattedStartTime) {
      requestData.startTime = formattedStartTime
    }
    if (formattedEndTime) {
      requestData.endTime = formattedEndTime
    }

    console.log('发送给后端的竞赛创建数据:', requestData)
    console.log('开始时间格式:', formattedStartTime)
    console.log('结束时间格式:', formattedEndTime)

    loadingCreate.value = true
    const response = await apiService.createProblemSet(requestData)

    if (response && response.code === 0) {
      errorMessage.value = '<span style="color: #52c41a;">创建竞赛成功</span>'
      setTimeout(() => {
        closeCreateDialog()
        loadContestData()
      }, 1500)
    } else {
      errorMessage.value = '创建竞赛失败: ' + (response?.msg || '未知错误')
    }
  } catch (error) {
    console.error('创建竞赛失败:', error)
    errorMessage.value = '创建竞赛失败: 网络错误'
  } finally {
    loadingCreate.value = false
  }
}

const loadRecommendedContests = async (page, pageSize) => {
  try {
    // 如果是第一页且没有存储所有竞赛，先获取所有竞赛
    if (page === 1 && allContests.value.length === 0) {
      loadingRecommended.value = true
      errorMessage.value = ''

      let allContestsData = []
      let currentPageNum = 1
      let hasMore = true

      // 循环获取所有页的竞赛数据
      while (hasMore) {
        const response = await apiService.getRecommendedProblemSets(currentPageNum, 50, 1)
        if (response && response.code === 0 && response.data && Array.isArray(response.data.list)) {
          allContestsData = [
            ...allContestsData,
            ...response.data.list.map((set) => ({
              id: set.id || 0,
              name: set.name || '',
              description: set.description || '',
              totalProblems: set.totalProblems || 0,
              startTime: set.startTime || '',
              endTime: set.endTime || '',
            })),
          ]

          // 检查是否还有更多数据
          if (response.data.list.length < 50) {
            hasMore = false
          } else {
            currentPageNum++
          }
        } else {
          hasMore = false
        }
      }

      allContests.value = allContestsData
      totalContestsCount.value = allContestsData.length
    }

    // 根据状态筛选竞赛
    let filteredContests = allContests.value
    if (contestStatusFilter.value !== 'all') {
      const now = new Date()
      filteredContests = allContests.value.filter((contest) => {
        const startTime = contest.startTime ? new Date(contest.startTime) : null
        const endTime = contest.endTime ? new Date(contest.endTime) : null

        switch (contestStatusFilter.value) {
          case 'upcoming':
            // 未开始：startTime > 当前时间
            return startTime && startTime > now
          case 'ongoing':
            // 进行中：startTime <= 当前时间 && endTime > 当前时间
            return startTime && endTime && startTime <= now && endTime > now
          case 'ended':
            // 已结束：endTime <= 当前时间
            return endTime && endTime <= now
          default:
            return true
        }
      })
    }

    // 实现分页逻辑
    const startIndex = (page - 1) * pageSize
    const endIndex = startIndex + pageSize
    recommendedContests.value = filteredContests.slice(startIndex, endIndex)

    // 如果当前页数据不足，尝试加载更多（如果还有更多数据）
    if (recommendedContests.value.length < pageSize && endIndex < filteredContests.length) {
      const remaining = pageSize - recommendedContests.value.length
      const additionalContests = filteredContests.slice(endIndex, endIndex + remaining)
      recommendedContests.value = [...recommendedContests.value, ...additionalContests]
    }

    currentPage.value = typeof page === 'number' ? page : 1
  } catch (error) {
    console.error('获取竞赛失败:', error)
    recommendedContests.value = []
    totalContestsCount.value = 0
    errorMessage.value = '获取竞赛失败: 网络错误'
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

// 更新竞赛历史分页
const updateContestHistoryPage = () => {
  const startIndex = (currentHistoryPage.value - 1) * historyPageSize.value
  const endIndex = startIndex + historyPageSize.value
  contestHistory.value = allContestHistory.value.slice(startIndex, endIndex)
}

// 切换竞赛历史页码
const changeHistoryPage = (newPage) => {
  if (
    newPage >= 1 &&
    newPage <= Math.ceil(allContestHistory.value.length / historyPageSize.value)
  ) {
    currentHistoryPage.value = newPage
    updateContestHistoryPage()
  }
}

// 组件挂载时获取数据
onMounted(() => {
  // 获取用户信息
  const info = localStorage.getItem('userInfo')
  if (info) {
    userInfo.value = JSON.parse(info)
  }
  loadContestData()
})
</script>

<style scoped>
.contest-page {
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

/* 进行中的竞赛 */
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

/* 推荐竞赛 */
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
  margin-bottom: 16px;
}

/* 竞赛筛选 */
.contest-filter {
  margin-bottom: 20px;
  padding: 16px;
  background-color: #f7f8fa;
  border-radius: 8px;
  border: 1px solid #e5e6eb;
}

.filter-row {
  display: flex;
  align-items: center;
  gap: 16px;
  flex-wrap: wrap;
  justify-content: flex-start;
}

.status-filter {
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  font-size: 14px;
  background-color: #ffffff;
  cursor: pointer;
  min-width: 120px;
}

.status-filter:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.search-box {
  display: flex;
  align-items: center;
  gap: 8px;
  flex: 1;
  min-width: 300px;
}

.search-box input {
  flex: 1;
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.search-box input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.search-btn {
  padding: 8px 16px;
  background-color: #165dff;
  color: white;
  border: none;
  border-radius: 4px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.search-btn:hover {
  background-color: #0d47a1;
}

.search-btn:active {
  transform: scale(0.98);
}

.plans-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
}

/* 竞赛历史 */
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

/* 创建竞赛对话框 */
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
.no-problems,
.empty-state {
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
