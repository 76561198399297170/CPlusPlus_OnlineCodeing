<!-- src/views/problems/ReviewProblems.vue -->
<template>
  <MainLayout>
    <div class="review-problems-page container">
      <div class="page-header">
        <h1>题目审核</h1>
        <div class="header-actions">
          <Button type="primary" class="batch-ai-btn" @click="openAIPanel(true)">
            🤖 AI 辅助审核
          </Button>
        </div>
      </div>

      <div class="review-content">
        <!-- 审核列表 -->
        <div class="problems-list">
          <div class="problems-header">
            <div class="header-item ai-status">AI审核</div>
            <div class="header-item id">题目ID</div>
            <div class="header-item title">标题</div>
            <div class="header-item difficulty">难度</div>
            <div class="header-item tags">标签</div>
            <div class="header-item time">创建时间</div>
          </div>

          <div v-if="loading" class="loading">加载中...</div>
          <div v-else-if="error" class="error">{{ error }}</div>
          <div v-else-if="problems.length === 0" class="empty">
            <p>暂无待审核题目</p>
          </div>
          <div v-else class="problems-body">
            <div
              v-for="problem in problems"
              :key="problem.id"
              class="problem-item"
              :class="{ active: selectedProblem?.id === problem.id }"
              @click="selectProblem(problem)"
            >
              <div class="item ai-status">
                <span
                  v-if="problem.aiReviewResult !== undefined"
                  :class="['ai-review-tag', problem.aiReviewResult === 1 ? 'passed' : 'rejected']"
                >
                  {{ problem.aiReviewResult === 1 ? '通过' : '不通过' }}
                </span>
                <span v-else class="ai-review-tag pending">待审核</span>
              </div>
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
              <div class="item time">{{ formatTime(problem.createdAt) }}</div>
            </div>
          </div>

          <!-- 分页 -->
          <div v-if="!loading && !error && problems.length > 0" class="pagination">
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

        <!-- 题目详情预览 -->
        <div v-if="selectedProblem" class="problem-preview card">
          <div class="preview-header">
            <h3 class="preview-title">{{ selectedProblem.title }}</h3>
            <div class="preview-meta">
              <span
                class="preview-difficulty"
                :class="getDifficultyClass(selectedProblem.difficulty)"
              >
                {{ getDifficultyText(selectedProblem.difficulty) }}
              </span>
              <span class="preview-tags">
                <span v-for="(tag, index) in selectedProblem.tags" :key="index" class="tag">
                  {{ tag }}
                </span>
              </span>
            </div>
          </div>

          <div class="preview-content">
            <div class="preview-section">
              <h4>题目描述</h4>
              <p>{{ selectedProblem.description }}</p>
            </div>

            <div class="preview-section">
              <h4>输入格式</h4>
              <p>{{ selectedProblem.inputFormat }}</p>
            </div>

            <div class="preview-section">
              <h4>输出格式</h4>
              <p>{{ selectedProblem.outputFormat }}</p>
            </div>

            <div class="preview-section" v-if="selectedProblem.sample">
              <h4>样例</h4>
              <div class="sample-content" v-html="parseSample(selectedProblem.sample)"></div>
            </div>

            <div class="preview-section">
              <h4>运行限制</h4>
              <div class="limits-content">
                <div class="limit-item">
                  <span class="limit-label">时间限制：</span>
                  <span class="limit-value">{{ selectedProblem.timeLimit || 1 }} 秒</span>
                </div>
                <div class="limit-item">
                  <span class="limit-label">内存限制：</span>
                  <span class="limit-value">{{ selectedProblem.memoryLimit || 256 }} MB</span>
                </div>
                <div v-if="selectedProblem.languageLimits" class="limit-item">
                  <span class="limit-label">语言限制：</span>
                  <div class="language-limits">
                    <span
                      v-for="(limit, lang) in selectedProblem.languageLimits"
                      :key="lang"
                      class="language-limit"
                    >
                      {{ lang }}: {{ limit.timeLimit }}秒 / {{ limit.memoryLimit }}MB
                    </span>
                  </div>
                </div>
              </div>
            </div>

            <div
              class="preview-section"
              v-if="selectedProblem.testCases && selectedProblem.testCases.length > 0"
            >
              <h4>测试用例</h4>
              <div v-for="(tc, index) in selectedProblem.testCases" :key="index" class="test-case">
                <div class="test-case-title">用例 {{ index + 1 }}</div>
                <div class="test-case-content">
                  <div class="test-case-input">
                    <div class="test-case-label">输入</div>
                    <pre>{{ tc.input }}</pre>
                  </div>
                  <div class="test-case-output">
                    <div class="test-case-label">预期输出</div>
                    <pre>{{ tc.output }}</pre>
                  </div>
                </div>
              </div>
            </div>
          </div>

          <!-- 审核操作 -->
          <div v-if="canReview && selectedProblem.status === 'PENDING'" class="preview-actions">
            <Button type="secondary" @click="selectedProblem = null"> 延后 </Button>
            <Button type="danger" @click="handleReject(selectedProblem)"> 拒绝 </Button>
            <Button type="primary" @click="handleApprove(selectedProblem)"> 通过审核 </Button>
          </div>
        </div>
      </div>
    </div>

    <!-- AI 辅助面板 -->
    <SideAIPanel
      :visible="showAIPanel"
      mode="review"
      :context="selectedProblem"
      :problemsList="problems"
      :isBatchMode="isBatchMode"
      @close="closeAIPanel"
      @apply="handleAIApply"
      @batchComplete="handleBatchComplete"
    />
  </MainLayout>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import SideAIPanel from '@/components/Common/SideAIPanel.vue'
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
const showAIPanel = ref(false)
const isBatchMode = ref(false)
const problems = ref([])
const selectedProblem = ref(null)
const loading = ref(false)
const error = ref('')
const page = ref(1)
const pageSize = ref(10)
const total = ref(0)

// 拒绝相关
const rejectingProblem = ref(null)

// 下拉菜单相关
const openDropdown = ref(null)

// 切换下拉菜单
const toggleDropdown = (problemId) => {
  if (openDropdown.value === problemId) {
    openDropdown.value = null
  } else {
    openDropdown.value = problemId
  }
}

// 计算属性
const totalPages = computed(() => {
  return Math.ceil(total.value / pageSize.value)
})

// 获取状态文本
const getStatusText = (status) => {
  const statusMap = {
    PENDING: '待审核',
    APPROVED: '已通过',
    REJECTED: '已拒绝',
  }
  return statusMap[status] || status
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

// 解析样例内容
const parseSample = (sample) => {
  let parsed = sample

  // 处理代码块标记
  parsed = parsed.replace(/```/g, '')

  // 处理输入标记
  parsed = parsed.replace(
    /[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/g,
    '<div class="sample-input"><strong>输入：</strong><pre>$1</pre></div>',
  )
  // 处理输出标记
  parsed = parsed.replace(
    /[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/g,
    '<div class="sample-output"><strong>输出：</strong><pre>$1</pre></div>',
  )
  // 处理分割线
  parsed = parsed.replace(/[\\]{1,2}-/g, '<hr class="sample-divider">')

  // 处理删除线
  parsed = parsed.replace(/~~(.*?)~~/g, '<del>$1</del>')

  return parsed
}

// 打开AI面板
const openAIPanel = (batchMode = false) => {
  isBatchMode.value = batchMode
  showAIPanel.value = true
}

// 关闭AI面板
const closeAIPanel = () => {
  showAIPanel.value = false
  isBatchMode.value = false
}

// 选择题目
const selectProblem = async (problem) => {
  try {
    loading.value = true
    const response = await apiService.getProblem(problem.id)
    if (response.code === 0) {
      let problemData = response.data
      
      // 处理嵌套数组 [[{...}]]
      if (Array.isArray(problemData)) {
        if (problemData.length > 0 && Array.isArray(problemData[0])) {
          // 嵌套数组，取第一个元素
          problemData = problemData[0][0] || problemData[0]
        } else if (problemData.length > 0) {
          // 普通数组，取第一个元素
          problemData = problemData[0]
        }
      }
      
      // 为题目添加status属性，确保审核操作按钮显示
      selectedProblem.value = {
        ...problemData,
        status: 'PENDING',
      }
    }
  } catch (error) {
    // 获取题目详情失败
  } finally {
    loading.value = false
  }
}

// 切换页码
const changePage = (newPage) => {
  if (newPage >= 1 && newPage <= totalPages.value) {
    page.value = newPage
    fetchProblems()
  }
}

// 获取待审核题目列表
const fetchProblems = async () => {
  try {
    loading.value = true
    error.value = ''

    // 调用后端 API 获取待审核题目列表
    const response = await apiService.getPendingProblems(page.value, pageSize.value)

    if (response.code === 0) {
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

// 审核通过
const handleApprove = async (problem) => {
  try {
    // 调用 API 审核通过题目
    const response = await apiService.approveProblem(problem.id)

    if (response.code === 0) {
      problem.status = 'APPROVED'
      if (selectedProblem.value?.id === problem.id) {
        selectedProblem.value.status = 'APPROVED'
      }
      // 收起题目预览
      selectedProblem.value = null
      // 刷新题目列表
      await fetchProblems()
    }
  } catch (error) {
    // 审核失败
  } finally {
    // 收起下拉菜单
    openDropdown.value = null
  }
}

// 审核拒绝
const handleReject = async (problem) => {
  try {
    // 调用 API 审核拒绝题目
    const response = await apiService.rejectProblem(problem.id)

    if (response.code === 0) {
      problem.status = 'REJECTED'
      if (selectedProblem.value?.id === problem.id) {
        selectedProblem.value.status = 'REJECTED'
      }
      // 收起题目预览
      selectedProblem.value = null
      // 刷新题目列表
      await fetchProblems()
    }
  } catch (error) {
    // 拒绝失败
  } finally {
    // 收起下拉菜单
    openDropdown.value = null
  }
}

// 组件挂载
onMounted(() => {
  fetchProblems()
})

// 处理 AI 辅助结果应用
const handleAIApply = (content) => {
  console.log('handleAIApply called with:', content)
  // 如果 content 是对象且包含审核结果
  if (typeof content === 'object' && content !== null) {
    // 确保 reviewRes 是数字类型（1-通过，0-未通过）
    const reviewResult = content.reviewRes === 1 || content.reviewRes === true ? 1 : 0
    console.log(
      'reviewResult:',
      reviewResult,
      'isBatch:',
      content.isBatch,
      'problemId:',
      content.problemId,
    )

    // AI 审核模式：处理单个题目的审核结果
    if (content.isBatch && content.problemId !== undefined) {
      console.log('Processing batch mode for problem:', content.problemId)
      // 更新题目列表中的对应题目
      // 使用 == 进行宽松比较，避免类型不一致问题
      const problemIndex = problems.value.findIndex((p) => p.id == content.problemId)
      console.log('Found problem at index:', problemIndex)
      if (problemIndex !== -1) {
        // 使用 Object.assign 确保响应式更新
        problems.value[problemIndex] = {
          ...problems.value[problemIndex],
          aiReviewResult: reviewResult,
          aiReviewMsg: content.reviewMsg,
        }
        console.log('Updated problem:', problems.value[problemIndex])
      }
      return
    }

    // 单个题目审核模式
    if (content.reviewRes !== undefined) {
      console.log('Processing single review mode')
      // 如果有 problemId，使用它；否则使用当前选中的题目
      const targetProblemId = content.problemId || selectedProblem.value?.id
      console.log('targetProblemId:', targetProblemId)

      if (targetProblemId) {
        // 更新题目列表中的对应题目
        // 使用 == 进行宽松比较，避免类型不一致问题
        const problemIndex = problems.value.findIndex((p) => p.id == targetProblemId)
        console.log('Found problem at index:', problemIndex)
        if (problemIndex !== -1) {
          // 使用 Object.assign 确保响应式更新
          problems.value[problemIndex] = {
            ...problems.value[problemIndex],
            aiReviewResult: reviewResult,
            aiReviewMsg: content.reviewMsg,
          }
          console.log('Updated problem:', problems.value[problemIndex])
        }

        // 如果当前有选中的题目且与目标题目相同，也更新选中题目
        if (selectedProblem.value && selectedProblem.value.id === targetProblemId) {
          selectedProblem.value = {
            ...selectedProblem.value,
            aiReviewResult: reviewResult,
            aiReviewMsg: content.reviewMsg,
          }
          console.log('Updated selectedProblem:', selectedProblem.value)
        }
      }
    } else {
      console.log('content.reviewRes is undefined, skipping update')
    }
  } else {
    console.log('content is not an object or is null')
  }
}

// 处理 AI 审核完成
const handleBatchComplete = (batchResult) => {
  // AI 审核完成后的处理，可以在这里添加提示或其他逻辑
  console.log('AI 审核完成:', batchResult)
}
</script>

<style scoped>
.review-problems-page {
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
  margin: 0;
}

.header-actions {
  display: flex;
  gap: 12px;
}

.ai-btn {
  display: flex;
  align-items: center;
  gap: 6px;
}

.batch-ai-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  background: linear-gradient(135deg, #165dff 0%, #4080ff 100%);
  border: none;
}

.review-content {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
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

.problems-header {
  display: grid;
  grid-template-columns: 80px 80px 2fr 100px 1.5fr 120px;
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

.problems-body {
  flex: 1;
  overflow-y: auto;
  min-height: 0;
  position: relative;
  z-index: 1;
}

.problem-item {
  display: grid;
  grid-template-columns: 80px 80px 2fr 100px 1.5fr 120px;
  padding: 16px;
  border-bottom: 1px solid #e5e6eb;
  cursor: pointer;
  transition: background-color 0.2s ease;

  &:hover,
  &.active {
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

.item.time {
  font-size: 13px;
  color: #86909c;
}

.item.id {
  font-weight: 500;
  color: #165dff;
}

.item.title {
  font-weight: 500;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

/* 难度标签 */
.difficulty-tag {
  padding: 2px 10px;
  border-radius: 10px;
  font-size: 11px;
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

/* 状态标签 */
.status-tag {
  padding: 2px 10px;
  border-radius: 10px;
  font-size: 11px;
  font-weight: 500;

  &.pending {
    background-color: #fff3e0;
    color: #ff7d00;
  }

  &.approved {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.rejected {
    background-color: #fce4ec;
    color: #f53f3f;
  }
}

/* AI 审核结果标记 */
.ai-review-tag {
  padding: 2px 8px;
  border-radius: 10px;
  font-size: 11px;
  font-weight: 500;

  &.passed {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.rejected {
    background-color: #fce4ec;
    color: #f53f3f;
  }

  &.pending {
    background-color: #f2f3f5;
    color: #86909c;
  }
}

/* 标签样式 */
.tag {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  color: #165dff;
  background-color: #e8f0ff;
  margin-right: 8px;
}

/* 加载和错误状态 */
.loading,
.error,
.empty {
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
  padding: 4px 12px;
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
  font-size: 13px;
  color: #4e5969;
}

/* 题目预览 */
.problem-preview {
  padding: 24px;
  height: fit-content;
  max-height: calc(100vh - 200px);
  overflow-y: auto;
}

.preview-header {
  margin-bottom: 24px;
  padding-bottom: 16px;
  border-bottom: 1px solid #e5e6eb;
}

.preview-title {
  font-size: 20px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 12px;
}

.preview-meta {
  display: flex;
  align-items: center;
  gap: 12px;
}

.preview-difficulty {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;

  &.EASY {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.MEDIUM {
    background-color: #fff3e0;
    color: #ff7d00;
  }

  &.HARD {
    background-color: #fce4ec;
    color: #f53f3f;
  }
}

.preview-tags {
  display: flex;
  gap: 8px;
}

.tag {
  padding: 4px 12px;
  background-color: #e8f0ff;
  color: #165dff;
  border-radius: 12px;
  font-size: 12px;
}

.preview-content {
  margin-bottom: 24px;
}

.preview-section {
  margin-bottom: 20px;
}

.preview-section h4 {
  font-size: 15px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 8px;
}

.preview-section p {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.6;
}

.sample-content {
  background-color: #f7f8fa;
  border-radius: 4px;
  padding: 12px;
  margin-top: 8px;

  pre {
    margin: 0;
    font-family: 'Courier New', monospace;
    font-size: 13px;
    line-height: 1.4;
    color: #4e5969;
    white-space: pre-wrap;
  }

  .sample-input,
  .sample-output {
    margin-bottom: 12px;

    strong {
      color: #1d2129;
      margin-bottom: 4px;
      display: block;
    }

    pre {
      background-color: #ffffff;
      border: 1px solid #e5e6eb;
      border-radius: 4px;
      padding: 8px;
      margin-top: 4px;
    }
  }

  .sample-divider {
    margin: 12px 0;
    border: none;
    border-top: 1px dashed #e5e6eb;
  }
}

.limits-content {
  margin-top: 8px;

  .limit-item {
    margin-bottom: 8px;
    display: flex;
    align-items: flex-start;

    .limit-label {
      font-weight: 500;
      color: #4e5969;
      min-width: 80px;
    }

    .limit-value {
      color: #1d2129;
    }

    .language-limits {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      margin-top: 4px;

      .language-limit {
        background-color: #e8f0ff;
        color: #165dff;
        padding: 4px 8px;
        border-radius: 4px;
        font-size: 12px;
      }
    }
  }
}

.test-case {
  margin-bottom: 16px;
  padding: 16px;
  background-color: #f7f8fa;
  border-radius: 8px;
}

.test-case-title {
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 12px;
}

.test-case-content {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 16px;
}

.test-case-label {
  font-size: 13px;
  font-weight: 500;
  color: #4e5969;
  margin-bottom: 8px;
}

.test-case-content pre {
  background-color: #ffffff;
  padding: 12px;
  border-radius: 6px;
  font-family: monospace;
  font-size: 13px;
  color: #1d2129;
  margin: 0;
  white-space: pre-wrap;
  word-break: break-all;
}

.preview-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  padding-top: 16px;
  border-top: 1px solid #e5e6eb;
}

/* 弹窗 */
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

.dialog {
  background-color: #ffffff;
  border-radius: 8px;
  width: 480px;
  max-width: 90%;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
}

.dialog-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 24px;
  border-bottom: 1px solid #e5e6eb;
}

.dialog-header h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

.dialog-close {
  font-size: 24px;
  color: #86909c;
  cursor: pointer;
  line-height: 1;
}

.dialog-body {
  padding: 24px;
}

.dialog-footer {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  padding: 16px 24px;
  border-top: 1px solid #e5e6eb;
}

.form-item {
  margin-bottom: 0;
}

.form-item__label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.form-item__textarea {
  width: 100%;
  padding: 12px 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  font-size: 14px;
  color: #1d2129;
  font-family: inherit;
  resize: vertical;
  transition: all 0.2s ease;
}

.form-item__textarea:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

/* 下拉菜单样式 */
.dropdown {
  position: relative;
  display: inline-block;
}

.dropdown-toggle {
  padding: 4px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  background-color: #ffffff;
  color: #4e5969;
  cursor: pointer;
  font-size: 13px;
  display: flex;
  align-items: center;
  gap: 6px;
  transition: all 0.2s ease;

  &:hover {
    border-color: #165dff;
    color: #165dff;
  }
}

.dropdown-arrow {
  font-size: 10px;
  transition: transform 0.2s ease;
}

.dropdown-menu {
  position: absolute;
  top: 100%;
  right: 0;
  margin-top: 4px;
  background-color: #ffffff;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  z-index: 9999;
  min-width: 120px;
  overflow: visible;
  display: flex;
  flex-direction: column;
}

.dropdown-item {
  display: block;
  width: 100%;
  padding: 8px 12px;
  text-align: left;
  border: none;
  background: none;
  font-size: 13px;
  cursor: pointer;
  transition: background-color 0.2s ease;

  &:hover {
    background-color: #f7f8fa;
  }

  &.approve {
    color: #00b42a;

    &:hover {
      background-color: #e8f5e8;
    }
  }

  &.reject {
    color: #f53f3f;

    &:hover {
      background-color: #fce4ec;
    }
  }

  &.delay {
    color: #86909c;

    &:hover {
      background-color: #f5f5f5;
    }
  }
}
</style>
