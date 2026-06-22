<!-- src/views/admin/Problems.vue -->
<template>
  <MainLayout>
    <div class="admin-problems-page container">
      <div class="page-header">
        <h1>题目管理</h1>
        <div class="header-actions">
          <input
            v-model="searchKeyword"
            type="text"
            class="search-input"
            placeholder="搜索题目ID或标题..."
            @input="handleSearch"
          />
          <select v-model="filterStatus" class="filter-select" @change="fetchProblems">
            <option value="ALL">全部题目</option>
            <option value="PENDING">待审核</option>
            <option value="APPROVED">已通过</option>
            <option value="REJECTED">已拒绝</option>
            <option value="HIDDEN">已隐藏</option>
          </select>
        </div>
      </div>

      <div class="review-content">
        <!-- 题目列表 -->
        <div class="problems-list">
          <div class="problems-header">
            <div class="header-item status">状态</div>
            <div class="header-item id">题目ID</div>
            <div class="header-item title">标题</div>
            <div class="header-item difficulty">难度</div>
            <div class="header-item tags">标签</div>
            <div class="header-item time">创建时间</div>
            <div class="header-item actions">操作</div>
          </div>

          <div v-if="loading" class="loading">加载中...</div>
          <div v-else-if="error" class="error">{{ error }}</div>
          <div v-else-if="problems.length === 0" class="empty">
            <p>暂无题目</p>
          </div>
          <div v-else class="problems-body">
            <div
              v-for="problem in problems"
              :key="problem.id"
              class="problem-item"
              :class="{ active: selectedProblem?.id === problem.id }"
              @click="selectProblem(problem)"
            >
              <div class="item status">
                <span :class="['status-tag', getStatusClass(problem.status)]">
                  {{ getStatusText(problem.status) }}
                </span>
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
              <div class="item actions" @click.stop>
                <div class="dropdown">
                  <button class="dropdown-toggle" @click="toggleDropdown(problem.id)">
                    操作 <span class="dropdown-arrow">▼</span>
                  </button>
                  <div v-if="openDropdown === problem.id" class="dropdown-menu">
                    <button class="dropdown-item approve" @click="handleApprove(problem)">
                      通过
                    </button>
                    <button class="dropdown-item reject" @click="handleReject(problem)">
                      驳回
                    </button>
                    <button class="dropdown-item hide" @click="handleHide(problem)">隐藏</button>
                  </div>
                </div>
              </div>
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
              <span :class="['status-tag', getStatusClass(selectedProblem.status)]">
                {{ getStatusText(selectedProblem.status) }}
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

          <!-- 操作按钮 -->
          <div class="preview-actions">
            <Button type="secondary" @click="handleHide(selectedProblem)"> 隐藏 </Button>
            <Button
              v-if="selectedProblem.status === 'PENDING' || selectedProblem.status === 'REJECTED'"
              type="danger"
              @click="handleReject(selectedProblem)"
            >
              驳回
            </Button>
            <Button
              v-if="
                selectedProblem.status === 'PENDING' ||
                selectedProblem.status === 'REJECTED' ||
                selectedProblem.status === 'HIDDEN'
              "
              type="primary"
              @click="handleApprove(selectedProblem)"
            >
              通过
            </Button>
          </div>
        </div>
      </div>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import { apiService } from '@/services/api'

// 响应式数据
const problems = ref([])
const selectedProblem = ref(null)
const loading = ref(false)
const error = ref('')
const page = ref(1)
const pageSize = ref(10)
const total = ref(0)
const filterStatus = ref('ALL')
const searchKeyword = ref('')

// 下拉菜单相关
const openDropdown = ref(null)

// 搜索处理
const handleSearch = () => {
  page.value = 1
  fetchProblems()
}

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
    REJECTED: '已驳回',
    HIDDEN: '已隐藏',
  }
  return statusMap[status] || status
}

// 获取状态类名
const getStatusClass = (status) => {
  const classMap = {
    PENDING: 'pending',
    APPROVED: 'approved',
    REJECTED: 'rejected',
    HIDDEN: 'hidden',
  }
  return classMap[status] || ''
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
  parsed = parsed.replace(/```/g, '')
  parsed = parsed.replace(
    /[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/g,
    '<div class="sample-input"><strong>输入：</strong><pre>$1</pre></div>',
  )
  parsed = parsed.replace(
    /[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/g,
    '<div class="sample-output"><strong>输出：</strong><pre>$1</pre></div>',
  )
  parsed = parsed.replace(/[\\]{1,2}-/g, '<hr class="sample-divider">')
  parsed = parsed.replace(/~~(.*?)~~/g, '<del>$1</del>')
  return parsed
}

// 选择题目
const selectProblem = async (problem) => {
  try {
    loading.value = true
    console.log('开始获取题目详情，Problem对象:', problem)
    console.log('使用的ID:', problem.id, '类型:', typeof problem.id)

    // 确保ID是数字类型
    const problemId = parseInt(problem.id)
    console.log('转换后的ID:', problemId, '类型:', typeof problemId)

    const response = await apiService.getProblem(problemId)
    console.log('题目详情 API 返回:', response)
    if (response.code === 0) {
      let problemData = response.data
      console.log('原始题目详情数据:', problemData)

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

      console.log('处理后的题目详情数据:', problemData)
      selectedProblem.value = {
        ...problemData,
        status: problem.status,
      }
      console.log('设置后的 selectedProblem:', selectedProblem.value)
    } else {
      console.error('获取题目详情失败，错误码:', response.code, '错误信息:', response.msg)
    }
  } catch (error) {
    console.error('获取题目详情失败:', error)
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

// 获取题目列表
const fetchProblems = async () => {
  try {
    loading.value = true
    error.value = ''

    let allProblems = []

    if (filterStatus.value === 'ALL') {
      // 获取所有状态的题目
      const [pendingRes, approvedRes, rejectedRes, hiddenRes] = await Promise.all([
        apiService.getPendingProblemsList(page.value, pageSize.value),
        apiService.getApprovedProblemsList(page.value, pageSize.value),
        apiService.getRejectedProblemsList(page.value, pageSize.value),
        apiService.getHiddenProblemsList(page.value, pageSize.value),
      ])

      // 合并所有题目并添加状态标识
      if (pendingRes.code === 0 && pendingRes.data.list) {
        pendingRes.data.list.forEach((p) => {
          console.log('Pending problem:', p)
          allProblems.push({ ...p, status: 'PENDING' })
        })
      }
      if (approvedRes.code === 0 && approvedRes.data.list) {
        approvedRes.data.list.forEach((p) => {
          console.log('Approved problem:', p)
          allProblems.push({ ...p, status: 'APPROVED' })
        })
      }
      if (rejectedRes.code === 0 && rejectedRes.data.list) {
        rejectedRes.data.list.forEach((p) => {
          console.log('Rejected problem:', p)
          allProblems.push({ ...p, status: 'REJECTED' })
        })
      }
      if (hiddenRes.code === 0 && hiddenRes.data.list) {
        hiddenRes.data.list.forEach((p) => {
          console.log('Hidden problem:', p)
          allProblems.push({ ...p, status: 'HIDDEN' })
        })
      }

      // 按创建时间降序排序
      allProblems.sort((a, b) => {
        const timeA = new Date(a.createdAt || 0).getTime()
        const timeB = new Date(b.createdAt || 0).getTime()
        return timeB - timeA
      })

      // 搜索过滤
      if (searchKeyword.value.trim()) {
        const keyword = searchKeyword.value.trim().toLowerCase()
        allProblems = allProblems.filter((p) => {
          const idMatch = String(p.id).includes(keyword)
          const titleMatch = p.title && p.title.toLowerCase().includes(keyword)
          return idMatch || titleMatch
        })
      }

      problems.value = allProblems
      total.value = allProblems.length
    } else {
      // 按指定状态获取题目
      let response
      switch (filterStatus.value) {
        case 'PENDING':
          response = await apiService.getPendingProblemsList(page.value, pageSize.value)
          if (response.code === 0 && response.data.list) {
            response.data.list.forEach((p) => {
              allProblems.push({ ...p, status: 'PENDING' })
            })
          }
          break
        case 'APPROVED':
          response = await apiService.getApprovedProblemsList(page.value, pageSize.value)
          if (response.code === 0 && response.data.list) {
            response.data.list.forEach((p) => {
              allProblems.push({ ...p, status: 'APPROVED' })
            })
          }
          break
        case 'REJECTED':
          response = await apiService.getRejectedProblemsList(page.value, pageSize.value)
          if (response.code === 0 && response.data.list) {
            response.data.list.forEach((p) => {
              allProblems.push({ ...p, status: 'REJECTED' })
            })
          }
          break
        case 'HIDDEN':
          // 隐藏的题目
          response = await apiService.getHiddenProblemsList(page.value, pageSize.value)
          if (response.code === 0 && response.data.list) {
            response.data.list.forEach((p) => {
              allProblems.push({ ...p, status: 'HIDDEN' })
            })
          }
          break
        default:
          response = await apiService.getPendingProblemsList(page.value, pageSize.value)
          if (response.code === 0 && response.data.list) {
            response.data.list.forEach((p) => {
              allProblems.push({ ...p, status: 'PENDING' })
            })
          }
      }

      // 搜索过滤
      if (searchKeyword.value.trim()) {
        const keyword = searchKeyword.value.trim().toLowerCase()
        allProblems = allProblems.filter((p) => {
          const idMatch = String(p.id).includes(keyword)
          const titleMatch = p.title && p.title.toLowerCase().includes(keyword)
          return idMatch || titleMatch
        })
      }

      problems.value = allProblems
      total.value = allProblems.length
    }
  } catch (err) {
    console.error('获取题目列表失败:', err)
    error.value = err.msg || '网络错误，无法获取题目列表'
  } finally {
    loading.value = false
  }
}

// 审核通过
const handleApprove = async (problem) => {
  try {
    const response = await apiService.approveProblem(problem.id)

    if (response.code === 0) {
      problem.status = 'APPROVED'
      if (selectedProblem.value?.id === problem.id) {
        selectedProblem.value.status = 'APPROVED'
      }
      await fetchProblems()
    }
  } catch (error) {
    console.error('审核通过失败:', error)
  } finally {
    openDropdown.value = null
  }
}

// 审核驳回
const handleReject = async (problem) => {
  try {
    const response = await apiService.rejectProblem(problem.id)

    if (response.code === 0) {
      problem.status = 'REJECTED'
      if (selectedProblem.value?.id === problem.id) {
        selectedProblem.value.status = 'REJECTED'
      }
      selectedProblem.value = null
      await fetchProblems()
    }
  } catch (error) {
    console.error('驳回失败:', error)
  } finally {
    openDropdown.value = null
  }
}

// 隐藏题目
const handleHide = async (problem) => {
  try {
    const response = await apiService.hideProblem(problem.id)

    if (response.code === 0) {
      problem.status = 'HIDDEN'
      if (selectedProblem.value?.id === problem.id) {
        selectedProblem.value.status = 'HIDDEN'
      }
      await fetchProblems()
    }
  } catch (error) {
    console.error('隐藏题目失败:', error)
  } finally {
    openDropdown.value = null
  }
}

// 取消隐藏（已移除，使用通过审核代替）

// 组件挂载
onMounted(() => {
  fetchProblems()
})
</script>

<style scoped>
.admin-problems-page {
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
  align-items: center;
}

.filter-select {
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #1d2129;
  background-color: #ffffff;
  cursor: pointer;
  transition: all 0.2s ease;
}

.filter-select:focus {
  outline: none;
  border-color: #165dff;
}

.search-input {
  padding: 8px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #1d2129;
  background-color: #ffffff;
  transition: all 0.2s ease;
  min-width: 200px;
}

.search-input:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

.search-input::placeholder {
  color: #8f959e;
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
  grid-template-columns: 80px 80px 2fr 100px 1.5fr 120px 80px;
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
  grid-template-columns: 80px 80px 2fr 100px 1.5fr 120px 80px;
  padding: 16px;
  border-bottom: 1px solid #e5e6eb;
  cursor: pointer;
  transition: background-color 0.2s ease;
}

.problem-item:hover,
.problem-item.active {
  background-color: #f7f8fa;
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

.item.actions {
  justify-content: center;
}

/* 难度标签 */
.difficulty-tag {
  padding: 2px 10px;
  border-radius: 10px;
  font-size: 11px;
  font-weight: 500;
}

.difficulty-tag.easy {
  background-color: #e8f5e8;
  color: #00b42a;
}

.difficulty-tag.medium {
  background-color: #fff3e0;
  color: #ff7d00;
}

.difficulty-tag.hard {
  background-color: #fce4ec;
  color: #f53f3f;
}

/* 状态标签 */
.status-tag {
  padding: 2px 10px;
  border-radius: 10px;
  font-size: 11px;
  font-weight: 500;
}

.status-tag.pending {
  background-color: #fff3e0;
  color: #ff7d00;
}

.status-tag.approved {
  background-color: #e8f5e8;
  color: #00b42a;
}

.status-tag.rejected {
  background-color: #fce4ec;
  color: #f53f3f;
}

.status-tag.hidden {
  background-color: #f2f3f5;
  color: #86909c;
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
}

.pagination-btn:hover:not(:disabled) {
  border-color: #165dff;
  color: #165dff;
}

.pagination-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
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
  flex-wrap: wrap;
}

.preview-difficulty {
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.preview-difficulty.easy {
  background-color: #e8f5e8;
  color: #00b42a;
}

.preview-difficulty.medium {
  background-color: #fff3e0;
  color: #ff7d00;
}

.preview-difficulty.hard {
  background-color: #fce4ec;
  color: #f53f3f;
}

.preview-tags {
  display: flex;
  gap: 8px;
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
}

.sample-content pre {
  margin: 0;
  font-family: 'Courier New', monospace;
  font-size: 13px;
  line-height: 1.4;
  color: #4e5969;
  white-space: pre-wrap;
}

.sample-content .sample-input,
.sample-content .sample-output {
  margin-bottom: 12px;
}

.sample-content .sample-input strong,
.sample-content .sample-output strong {
  color: #1d2129;
  margin-bottom: 4px;
  display: block;
}

.sample-content .sample-input pre,
.sample-content .sample-output pre {
  background-color: #ffffff;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  padding: 8px;
  margin-top: 4px;
}

.sample-content .sample-divider {
  margin: 12px 0;
  border: none;
  border-top: 1px dashed #e5e6eb;
}

.limits-content {
  margin-top: 8px;
}

.limits-content .limit-item {
  margin-bottom: 8px;
  display: flex;
  align-items: flex-start;
}

.limits-content .limit-label {
  font-weight: 500;
  color: #4e5969;
  min-width: 80px;
}

.limits-content .limit-value {
  color: #1d2129;
}

.limits-content .language-limits {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  margin-top: 4px;
}

.limits-content .language-limit {
  background-color: #e8f0ff;
  color: #165dff;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 12px;
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
}

.dropdown-toggle:hover {
  border-color: #165dff;
  color: #165dff;
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
}

.dropdown-item:hover {
  background-color: #f7f8fa;
}

.dropdown-item.approve {
  color: #00b42a;
}

.dropdown-item.approve:hover {
  background-color: #e8f5e8;
}

.dropdown-item.reject {
  color: #f53f3f;
}

.dropdown-item.reject:hover {
  background-color: #fce4ec;
}

.dropdown-item.hide {
  color: #86909c;
}

.dropdown-item.hide:hover {
  background-color: #f5f5f5;
}

.dropdown-item.unhide {
  color: #165dff;
}

.dropdown-item.unhide:hover {
  background-color: #e8f0ff;
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
</style>
