<template>
  <div class="problem-description">
    <!-- 左侧标签页 -->
    <div class="problem-tabs">
      <button
        v-for="tab in problemTabs"
        :key="tab.key"
        class="tab-button"
        :class="{ active: activeProblemTab === tab.key }"
        @click="activeProblemTab = tab.key"
      >
        {{ tab.label }}
      </button>
    </div>

    <!-- 题目内容区域 -->
    <div class="problem-content">
      <!-- 加载状态 -->
      <div v-if="loading" class="loading-content">
        <div class="loading-spinner"></div>
        <p>加载题目中...</p>
      </div>

      <!-- 错误提示 -->
      <div v-else-if="error" class="error-content">
        <p class="error-message">{{ error }}</p>
        <button class="retry-btn" @click="onRetry">重试</button>
      </div>

      <!-- 题目列表 -->
      <div v-else-if="activeProblemTab === 'problemList'" class="problem-list-content">
        <div v-if="problemListLoading" class="loading-content">
          <div class="loading-spinner"></div>
          <p>加载题目列表中...</p>
        </div>
        <div v-else-if="problemListError" class="error-content">
          <p class="error-message">{{ problemListError }}</p>
          <button class="retry-btn" @click="fetchProblemList">重试</button>
        </div>
        <div v-else class="problem-list">
          <div class="list-header">
            <h3>{{ problemSetName || '题目列表' }}</h3>
            <p class="list-description">{{ problemSetDescription || '' }}</p>
          </div>
          <div class="problems-grid">
            <div v-for="(problem, index) in problemList" :key="problem.id" class="problem-card">
              <div class="problem-number">{{ index + 1 }}</div>
              <div class="problem-info">
                <h4 class="problem-title">{{ problem.title }}</h4>
                <div class="problem-meta">
                  <span class="problem-difficulty" :class="'difficulty-' + problem.difficulty">
                    {{ getDifficultyText(problem.difficulty) }}
                  </span>
                  <span class="problem-status" :class="getStatusCodeClass(problem.completed)">
                    {{ getProblemStatusText(problem.completed) }}
                  </span>
                </div>
              </div>
              <router-link
                :to="`/editor/${problem.id}?problemSetId=${problemSetId}`"
                class="view-btn"
              >
                查看
              </router-link>
            </div>
          </div>
        </div>
      </div>

      <!-- 题目描述 -->
      <div v-else-if="activeProblemTab === 'description'" class="description-content">
        <div class="markdown-content">
          <template v-for="(block, index) in markdownBlocks" :key="index">
            <!-- 代码块 -->
            <CodeBlock
              v-if="block.type === 'code'"
              :code="block.code"
              :original-code="block.originalCode"
              :language="block.language"
              @copy="onCodeCopy"
              @apply-to-test="onApplyToTest"
            />
            <!-- 文本内容 -->
            <div v-else-if="block.type === 'text'" v-html="block.content"></div>
          </template>
        </div>

        <!-- 题目类型标签 -->
        <div v-if="problem" class="problem-tags">
          <span class="tag-label">题目类型：</span>
          <span v-for="(tag, index) in problem.tags" :key="index" class="tag-item">
            {{ tag }}
          </span>
        </div>

        <!-- 审核状态 -->
        <div
          v-if="problem && problem.audited !== undefined && problem.audited !== -1"
          class="audit-status"
        >
          <span class="audit-status-text"
            >本条题目（{{ getAuditStatusText(problem.audited) }}）</span
          >
        </div>
      </div>

      <!-- 提交情况 -->
      <div v-else-if="activeProblemTab === 'submission'" class="submission-content">
        <div v-if="submissionLoading" class="loading-content">
          <div class="loading-spinner"></div>
          <p>加载提交情况中...</p>
        </div>
        <div v-else-if="submissionError" class="error-content">
          <p class="error-message">{{ submissionError }}</p>
          <button class="retry-btn" @click="fetchSubmissionStats">重试</button>
        </div>
        <div v-else class="submission-stats">
          <div class="stats-header">
            <h3>排名</h3>
          </div>
          <div v-if="rankedSubmissions.length > 0" class="ranking-table-container">
            <table class="submission-table ranking-table">
              <thead>
                <tr>
                  <th>排名</th>
                  <th>用户ID</th>
                  <th>状态</th>
                  <th>分数</th>
                </tr>
              </thead>
              <tbody>
                <tr
                  v-for="(record, index) in rankedSubmissions"
                  :key="index"
                  :class="{ 'rank-top-3': index < 3 }"
                >
                  <td>
                    <span v-if="index === 0" class="rank-badge gold">🥇</span>
                    <span v-else-if="index === 1" class="rank-badge silver">🥈</span>
                    <span v-else-if="index === 2" class="rank-badge bronze">🥉</span>
                    <span v-else class="rank-number">{{ index + 1 }}</span>
                  </td>
                  <td>{{ record.userId }}</td>
                  <td>
                    <span :class="['status-tag', getStatusClass(record.status)]">
                      {{ getStatusText(record.status) }}
                    </span>
                  </td>
                  <td class="score-cell">{{ record.score }}</td>
                </tr>
              </tbody>
            </table>
          </div>

          <div class="records-section">
            <h4>提交记录</h4>
            <div v-if="sortedByTimeSubmissions.length === 0" class="empty-message">
              暂无提交记录
            </div>
            <div v-else class="submission-table-container">
              <table class="submission-table">
                <thead>
                  <tr>
                    <th>用户ID</th>
                    <th>题目</th>
                    <th>语言</th>
                    <th>状态</th>
                    <th>分数</th>
                    <th>提交时间</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="(record, index) in sortedByTimeSubmissions" :key="index">
                    <td>{{ record.userId }}</td>
                    <td>{{ record.questionTitle || record.question_id }}</td>
                    <td>{{ record.language }}</td>
                    <td>
                      <span :class="['status-tag', getStatusClass(record.status)]">
                        {{ getStatusText(record.status) }}
                      </span>
                    </td>
                    <td class="score-cell">{{ record.score }}</td>
                    <td>{{ record.submittedAt }}</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted } from 'vue'
import { useRoute } from 'vue-router'
import CodeBlock from '@/components/Common/CodeBlock.vue'
import { apiService } from '@/services/api'

// Props
const props = defineProps({
  problem: {
    type: Object,
    default: null,
  },
  loading: {
    type: Boolean,
    default: false,
  },
  error: {
    type: String,
    default: '',
  },
})

// Emits
const emit = defineEmits(['retry', 'code-copy', 'apply-to-test'])

const route = useRoute()

// 响应式数据
const activeProblemTab = ref('description')
const problemSetId = ref(route.query.problemSetId)
const currentProblemId = ref(Number(route.params.id) || 0)
const problemList = ref([])
const problemListLoading = ref(false)
const problemListError = ref('')
const problemSetName = ref('')
const problemSetDescription = ref('')
const submissionStats = ref([])
const submissionLoading = ref(false)
const submissionError = ref('')

const rankedSubmissions = computed(() => {
  const userProblemScores = new Map()

  submissionStats.value.forEach((record) => {
    if (!userProblemScores.has(record.userId)) {
      userProblemScores.set(record.userId, new Map())
    }
    const problemScores = userProblemScores.get(record.userId)
    const questionKey = record.question_id || 'unknown'

    if (!problemScores.has(questionKey) || record.score > problemScores.get(questionKey)) {
      problemScores.set(questionKey, record.score)
    }
  })

  const userTotalScores = []
  userProblemScores.forEach((problemScores, userId) => {
    const totalScore = Array.from(problemScores.values()).reduce((sum, score) => sum + score, 0)
    userTotalScores.push({
      userId,
      score: totalScore,
      status: totalScore > 0 ? 3 : 4,
    })
  })

  return userTotalScores.sort((a, b) => {
    if (b.score !== a.score) {
      return b.score - a.score
    }
    return a.userId - b.userId
  })
})

const sortedByTimeSubmissions = computed(() => {
  return [...submissionStats.value].sort((a, b) => {
    if (a.submittedAt < b.submittedAt) return 1
    if (a.submittedAt > b.submittedAt) return -1
    return 0
  })
})

// 题目标签页
const problemTabs = computed(() => {
  const tabs = [{ key: 'description', label: '题目描述' }]

  // 如果是从题目组进入，添加题目列表和提交情况选项卡
  if (problemSetId.value) {
    // 插入到题目描述之前
    tabs.unshift({ key: 'problemList', label: '题目列表' })
    tabs.push({ key: 'submission', label: '提交情况' })
  }

  return tabs
})

// 获取题目组列表
const fetchProblemList = async () => {
  if (!problemSetId.value) return

  try {
    problemListLoading.value = true
    problemListError.value = ''

    const response = await apiService.getProblemSetDetail(Number(problemSetId.value))

    if (response.code === 0) {
      problemList.value = response.data.problems || []
      problemSetName.value = response.data.name || ''
      problemSetDescription.value = response.data.description || ''
    } else {
      problemListError.value = response.msg || '获取题目列表失败'
    }
  } catch (err) {
    console.error('获取题目列表失败:', err)
    problemListError.value = '网络错误，获取题目列表失败'
  } finally {
    problemListLoading.value = false
  }
}

// 获取提交统计
const fetchSubmissionStats = async () => {
  if (!problemSetId.value) return

  try {
    submissionLoading.value = true
    submissionError.value = ''

    // 调用 API 获取提交统计
    const response = await apiService.getSubmissionStats(Number(problemSetId.value))

    if (response.code === 0) {
      submissionStats.value = Array.isArray(response.data) ? response.data : []
    } else {
      submissionError.value = response.msg || '获取提交情况失败'
    }
  } catch (err) {
    console.error('获取提交情况失败:', err)
    submissionError.value = '网络错误，获取提交情况失败'
  } finally {
    submissionLoading.value = false
  }
}

// 获取状态文本
const getStatusText = (status) => {
  const statusMap = {
    '-1': '未部署',
    0: '编译错误',
    1: '运行错误',
    2: '超时',
    3: '通过',
    4: '答案错误',
    ND: '未部署',
    CE: '编译错误',
    RE: '运行错误',
    TLE: '超时',
    AC: '通过',
    WA: '答案错误',
    UNKNOW: '未知',
  }
  return statusMap[String(status)] || '未知'
}

// 获取状态样式类
const getStatusClass = (status) => {
  const classMap = {
    '-1': 'status-undeployed',
    0: 'status-ce',
    1: 'status-re',
    2: 'status-tle',
    3: 'status-ac',
    4: 'status-wa',
    ND: 'status-undeployed',
    CE: 'status-ce',
    RE: 'status-re',
    TLE: 'status-tle',
    AC: 'status-ac',
    WA: 'status-wa',
    UNKNOW: '',
  }
  return classMap[String(status)] || ''
}

// 监听选项卡切换
watch(activeProblemTab, (newTab) => {
  if (newTab === 'submission') {
    fetchSubmissionStats()
  } else if (newTab === 'problemList') {
    fetchProblemList()
  }
})

// 监听题目组ID变化
watch(problemSetId, (newId) => {
  if (newId && activeProblemTab.value === 'problemList') {
    fetchProblemList()
  }
})

// 组件挂载时
onMounted(() => {
  // 如果有题目组ID且当前是题目列表选项卡，获取题目列表
  if (problemSetId.value && activeProblemTab.value === 'problemList') {
    fetchProblemList()
  }
})

// 转义HTML实体
const escapeHtml = (text) => {
  if (!text) return ''
  // 先处理已经转义的情况（\< 和 \>），将它们暂时替换为占位符
  let escaped = text.replace(/\\</g, '##LT##').replace(/\\>/g, '##GT##')
  // 然后替换未转义的 < 和 >
  escaped = escaped.replace(/</g, '&lt;').replace(/>/g, '&gt;')
  // 最后恢复占位符
  return escaped.replace(/##LT##/g, '<').replace(/##GT##/g, '>')
}

// 生成题目markdown
const problemMarkdown = computed(() => {
  if (!props.problem) return ''

  let markdown = `# ${escapeHtml(props.problem.title)}\n\n`

  // 问题描述
  markdown += `#### **问题描述**\n${escapeHtml(props.problem.description)}\n\n`

  // 输入格式
  markdown += `#### **输入格式**\n${escapeHtml(props.problem.inputFormat)}\n\n`

  // 输出格式
  markdown += `#### **输出格式**\n${escapeHtml(props.problem.outputFormat)}\n\n`

  // 样例部分
  if (props.problem.sample) {
    markdown += `#### **样例**\n${escapeHtml(props.problem.sample)}\n\n`
  }

  // 运行限制
  markdown += `### 运行限制\n| 语言 | 最大运行时间 | 最大运行内存 |\n| ---------- | ------------ | ------------ |\n`

  // 检查是否有 languageLimits
  if (props.problem.languageLimits) {
    // 语言顺序
    const languages = ['C++', 'C', 'Java', 'Python3', 'PyPy3', 'Go', 'JavaScript']
    languages.forEach((lang) => {
      if (props.problem.languageLimits[lang]) {
        const limit = props.problem.languageLimits[lang]
        const timeLimit = limit.timeLimit || props.problem.timeLimit
        const memoryLimit = limit.memoryLimit || props.problem.memoryLimit || 256
        markdown += `| ${lang} | ${timeLimit}s | ${memoryLimit}M |\n`
      } else {
        // 如果没有该语言的限制，使用默认值
        const defaultMemoryLimit = props.problem.memoryLimit || 256
        markdown += `| ${lang} | ${props.problem.timeLimit}s | ${defaultMemoryLimit}M |\n`
      }
    })
  } else {
    // 如果没有 languageLimits，使用默认值
    const defaultMemoryLimit = props.problem.memoryLimit || 256
    markdown += `| C++ | ${props.problem.timeLimit}s | ${defaultMemoryLimit}M |\n`
    markdown += `| C | ${props.problem.timeLimit}s | ${defaultMemoryLimit}M |\n`
    markdown += `| Java | ${props.problem.timeLimit * 2}s | ${defaultMemoryLimit}M |\n`
    markdown += `| Python3 | ${props.problem.timeLimit * 3}s | ${defaultMemoryLimit}M |\n`
    markdown += `| PyPy3 | ${props.problem.timeLimit * 3}s | ${defaultMemoryLimit}M |\n`
    markdown += `| Go | ${props.problem.timeLimit * 3}s | ${defaultMemoryLimit}M |\n`
    markdown += `| JavaScript | ${props.problem.timeLimit * 3}s | ${defaultMemoryLimit}M |\n`
  }

  return markdown
})

// 解析markdown为内容块
const markdownBlocks = computed(() => {
  let markdown = problemMarkdown.value

  // 首先处理所有换行符，确保统一格式
  markdown = markdown.replace(/\r\n/g, '\n')

  const blocks = []

  // 分割代码块和普通文本
  const parts = markdown.split(/(```[\s\S]*?```)/g)

  parts.forEach((part, index) => {
    if (part.startsWith('```')) {
      // 代码块
      const codeContent = part.slice(3, -3)
      // 不将第一行作为语言标识符，直接使用全部内容
      const language = 'code'
      let code = codeContent.trim()

      // 保存原始代码内容（用于拷贝到测试功能）
      const originalCode = code

      // 处理代码块中的特殊标记（支持单斜杠和双斜杠）
      // 处理输入标记
      code = code.replace(
        /[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/g,
        '<div class="sample-input"><strong>输入：</strong><pre>$1</pre></div>',
      )
      // 处理输出标记
      code = code.replace(
        /[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/g,
        '<div class="sample-output"><strong>输出：</strong><pre>$1</pre></div>',
      )
      // 处理分割线
      code = code.replace(/[\\]{1,2}-/g, '<hr class="sample-divider">')

      // 处理删除线
      code = code.replace(/~~(.*?)~~/g, '<del>$1</del>')

      const codeBlock = {
        type: 'code',
        language,
        code,
        originalCode, // 添加原始代码内容
      }
      blocks.push(codeBlock)
    } else if (part.trim()) {
      // 普通文本
      let text = part

      // 处理特殊标记（支持单斜杠和双斜杠）
      // 处理输入标记
      text = text.replace(
        /[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/g,
        '<div class="sample-input"><strong>输入：</strong><pre>$1</pre></div>',
      )
      // 处理输出标记
      text = text.replace(
        /[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/g,
        '<div class="sample-output"><strong>输出：</strong><pre>$1</pre></div>',
      )
      // 处理分割线
      text = text.replace(/[\\]{1,2}-/g, '<hr class="sample-divider">')

      // 处理标题
      text = text.replace(/^# (.*$)/gim, '<h1>$1</h1>')
      text = text.replace(/^## (.*$)/gim, '<h2>$1</h2>')
      text = text.replace(/^### (.*$)/gim, '<h3>$1</h3>')
      text = text.replace(/^#### (.*$)/gim, '<h4>$1</h4>')

      // 处理加粗
      text = text.replace(/\*\*(.*?)\*\*/g, '<strong>$1</strong>')

      // 处理删除线
      text = text.replace(/~~(.*?)~~/g, '<del>$1</del>')

      // 处理表格
      text = text.replace(/(^\|.*(?:\n\|.*)*)/gm, (match) => {
        const rows = match.split('\n').filter((row) => row.trim())
        if (rows.length < 2) return match

        let tableHTML = '<table class="markdown-table"><thead><tr>'
        // 表头
        const headers = rows[0]
          .split('|')
          .map((h) => h.trim())
          .filter((h) => h)
        headers.forEach((header) => {
          tableHTML += `<th>${header}</th>`
        })
        tableHTML += '</tr></thead><tbody>'

        // 表格内容
        for (let i = 1; i < rows.length; i++) {
          // 跳过分隔行
          if (rows[i].match(/^\|?\s*-+\s*\|/)) continue

          const cells = rows[i]
            .split('|')
            .map((c) => c.trim())
            .filter((c) => c)
          tableHTML += '<tr>'
          cells.forEach((cell) => {
            tableHTML += `<td>${cell}</td>`
          })
          tableHTML += '</tr>'
        }
        tableHTML += '</tbody></table>'
        return tableHTML
      })

      // 处理段落（排除已经处理过的元素）
      text = text.replace(/^(?!<h|<table|<div|<hr|\s*$)(.*$)/gim, '<p>$1</p>')

      // 处理段落内的换行符
      text = text.replace(/<p>(.*?)<\/p>/g, (match, content) => {
        const processedContent = content.replace(/\n/g, '<br>')
        return `<p>${processedContent}</p>`
      })

      // 处理空行
      text = text.replace(/\n{3,}/g, '\n\n')

      const textBlock = {
        type: 'text',
        content: text,
      }
      blocks.push(textBlock)
    }
  })

  return blocks
})

// 获取审核状态文本
const getAuditStatusText = (status) => {
  switch (status) {
    case 0:
      return '等待审核'
    case 1:
      return '通过'
    case 2:
      return '未通过'
    case 3:
      return '隐藏'
    default:
      return '未知'
  }
}

// 事件处理
const onRetry = () => {
  emit('retry')
}

const onCodeCopy = () => {
  emit('code-copy')
}

const onApplyToTest = (data) => {
  emit('apply-to-test', data)
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

const getProblemStatusText = (status) => {
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
</script>

<style scoped>
/* 左侧题目区域 */
.problem-description {
  width: 100%;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  transition: width 0.1s ease;
}

/* 左侧标签页 */
.problem-tabs {
  display: flex;
  border-bottom: 1px solid #e5e6eb;
  background-color: #f7f8fa;
}

.tab-button {
  padding: 12px 20px;
  border: none;
  background: none;
  font-size: 14px;
  font-weight: 500;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;
  border-bottom: 2px solid transparent;
  white-space: nowrap;
}

.tab-button:hover {
  color: #165dff;
}

.tab-button.active {
  color: #165dff;
  border-bottom-color: #165dff;
  background-color: #ffffff;
}

/* 题目内容 */
.problem-content {
  flex: 1;
  padding: 20px;
  overflow-y: auto;
}

/* Markdown内容 */
.markdown-content {
  line-height: 1.6;
  color: #1d2129;
  word-wrap: break-word;
  overflow-wrap: break-word;
  word-break: break-all;
}

.markdown-content h1 {
  font-size: 24px;
  font-weight: 600;
  margin-bottom: 20px;
  color: #1d2129;
}

.markdown-content h2 {
  font-size: 18px;
  font-weight: 600;
  margin: 20px 0 10px 0;
  color: #1d2129;
}

.markdown-content p {
  margin-bottom: 15px;
}

.markdown-content ul {
  margin: 15px 0;
  padding-left: 20px;
}

.markdown-content li {
  margin-bottom: 5px;
}

/* 表格 */
.markdown-table {
  width: 100%;
  border-collapse: collapse;
  margin: 15px 0;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  overflow: hidden;
}

.markdown-table th {
  background-color: #f7f8fa;
  padding: 10px 12px;
  text-align: left;
  font-weight: 500;
  color: #1d2129;
  border-bottom: 1px solid #e5e6eb;
}

.markdown-table td {
  padding: 10px 12px;
  border-bottom: 1px solid #e5e6eb;
  color: #4e5969;
}

.markdown-table tr:last-child td {
  border-bottom: none;
}

.markdown-table tr:hover {
  background-color: #f7f8fa;
}

/* 题目列表样式 */
.problem-list-content {
  height: 100%;
}

.problem-list {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.list-header {
  text-align: center;
  padding-bottom: 20px;
  border-bottom: 1px solid #e5e6eb;
}

.list-header h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 8px 0;
}

.list-description {
  font-size: 14px;
  color: #86909c;
  margin: 0;
  line-height: 1.5;
}

.problems-grid {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.problem-card {
  display: flex;
  align-items: center;
  padding: 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  transition: all 0.2s ease;
  gap: 16px;
  background-color: #ffffff;
}

.problem-card:hover {
  border-color: #165dff;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  transform: translateY(-1px);
}

.problem-card.current-problem {
  border-color: #165dff;
  background-color: #f0f5ff;
  box-shadow: 0 2px 8px rgba(22, 93, 255, 0.15);
}

.problem-number {
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

.view-btn {
  padding: 6px 12px;
  background-color: #165dff;
  color: #ffffff;
  border: none;
  border-radius: 6px;
  font-size: 13px;
  text-decoration: none;
  transition: all 0.2s ease;
  white-space: nowrap;
}

.view-btn:hover {
  background-color: #0e42c2;
}

/* 提交情况样式 */
.submission-content {
  height: 100%;
}

.submission-stats {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.stats-header h3 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin: 0;
}

.stats-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
}

.stat-card {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 16px;
  background-color: #f7f8fa;
  border-radius: 8px;
  border: 1px solid #e5e6eb;
}

.stat-icon {
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  background-color: #ffffff;
  border-radius: 8px;
  font-size: 20px;
}

.stat-info {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.stat-value {
  font-size: 24px;
  font-weight: 600;
  color: #165dff;
}

.stat-label {
  font-size: 12px;
  color: #86909c;
}

.stats-chart {
  background-color: #f7f8fa;
  border-radius: 8px;
  padding: 16px;
  border: 1px solid #e5e6eb;
}

.stats-chart h4 {
  font-size: 14px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 16px 0;
}

.language-bars {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.language-bar-item {
  display: flex;
  align-items: center;
  gap: 12px;
}

.language-name {
  width: 80px;
  font-size: 13px;
  color: #4e5969;
  text-align: right;
}

.language-bar-container {
  flex: 1;
  height: 8px;
  background-color: #e5e6eb;
  border-radius: 4px;
  overflow: hidden;
}

.language-bar {
  height: 100%;
  background-color: #165dff;
  border-radius: 4px;
  transition: width 0.3s ease;
}

.language-count {
  width: 60px;
  font-size: 12px;
  color: #86909c;
}

/* 提交记录表格 */
.empty-message {
  text-align: center;
  padding: 40px;
  color: #86909c;
  font-size: 14px;
}

.ranking-section,
.records-section {
  margin-top: 16px;
}

.ranking-section h4,
.records-section h4 {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 12px 0;
}

.ranking-table-container {
  margin-bottom: 20px;
}

.ranking-table {
  background-color: #fafafa;
}

.ranking-table td.score-cell {
  font-weight: 600;
  color: #165dff;
}

.rank-top-3 {
  background-color: #fffbf0;
}

.rank-top-3:hover {
  background-color: #fff8e0;
}

.rank-badge {
  font-size: 18px;
}

.rank-number {
  display: inline-block;
  min-width: 24px;
  text-align: center;
  font-weight: 500;
}

.submission-table-container {
  overflow-x: auto;
}

.submission-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}

.submission-table th,
.submission-table td {
  padding: 12px 8px;
  text-align: left;
  border-bottom: 1px solid #e5e6eb;
}

.submission-table th {
  background-color: #f7f8fa;
  font-weight: 600;
  color: #1d2129;
  white-space: nowrap;
}

.submission-table td {
  color: #4e5969;
}

.submission-table tr:hover {
  background-color: #f7f8fa;
}

/* 状态标签 */
.status-tag {
  padding: 2px 8px;
  border-radius: 10px;
  font-size: 11px;
  font-weight: 500;
  white-space: nowrap;
}

.status-ac {
  background-color: #e8f5e8;
  color: #00b42a;
}

.status-wa {
  background-color: #fce4ec;
  color: #f53f3f;
}

.status-ce {
  background-color: #fff3e0;
  color: #ff7d00;
}

.status-re {
  background-color: #fff3e0;
  color: #ff7d00;
}

.status-tle {
  background-color: #fff3e0;
  color: #ff7d00;
}

.status-undeployed {
  background-color: #f2f3f5;
  color: #86909c;
}

/* 空内容 */
.empty-content {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #86909c;
}

/* 加载状态 */
.loading-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #4e5969;
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid #e5e6eb;
  border-top: 3px solid #165dff;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 16px;
}

@keyframes spin {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}

/* 错误提示 */
.error-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  padding: 20px;
  text-align: center;
}

.error-message {
  color: #f53f3f;
  margin-bottom: 16px;
  font-size: 14px;
  line-height: 1.5;
}

.retry-btn {
  padding: 8px 16px;
  border: 1px solid #165dff;
  border-radius: 6px;
  background-color: #ffffff;
  color: #165dff;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.retry-btn:hover {
  background-color: rgba(22, 93, 255, 0.05);
}

/* 题目类型标签 */
.problem-tags {
  margin-top: 20px;
  padding-top: 15px;
  border-top: 1px solid #e5e6eb;
  font-size: 12px;
  color: #86909c;
}

/* 审核状态 */
.audit-status {
  margin-top: 12px;
  padding-top: 10px;
  border-top: 1px solid #f0f0f0;
  font-size: 11px;
  color: #86909c;
  text-align: right;
}

.audit-status-text {
  font-style: italic;
}

.tag-label {
  margin-right: 8px;
  font-weight: 500;
}

.tag-item {
  display: inline-block;
  padding: 2px 8px;
  margin-right: 8px;
  margin-bottom: 8px;
  background-color: #f7f8fa;
  border: 1px solid #e5e6eb;
  border-radius: 12px;
  color: #4e5969;
  font-size: 12px;
  transition: all 0.2s ease;
}

.tag-item:hover {
  background-color: #e6f7ff;
  border-color: #91d5ff;
  color: #165dff;
}

/* 样例样式 */
.markdown-content .sample-input,
.markdown-content .sample-output {
  margin-bottom: 16px;
  background-color: #f7f8fa;
  border-radius: 6px;
  padding: 12px;
}

.markdown-content .sample-input strong,
.markdown-content .sample-output strong {
  color: #1d2129;
  margin-bottom: 8px;
  display: block;
  font-size: 14px;
}

.markdown-content .sample-input pre,
.markdown-content .sample-output pre {
  background-color: #ffffff;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  padding: 10px;
  margin-top: 8px;
  font-family: 'Courier New', monospace;
  font-size: 13px;
  line-height: 1.4;
  color: #4e5969;
  white-space: pre-wrap;
}

.markdown-content .sample-divider {
  margin: 16px 0;
  border: none;
  border-top: 1px dashed #e5e6eb;
}

/* 滚动条样式 */
::-webkit-scrollbar {
  width: 8px;
  height: 8px;
}

::-webkit-scrollbar-track {
  background: #f7f8fa;
  border-radius: 4px;
}

::-webkit-scrollbar-thumb {
  background: #c9cdd4;
  border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
  background: #86909c;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .problem-card {
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }

  .problem-number {
    align-self: flex-start;
  }

  .view-btn {
    align-self: flex-start;
  }

  .stats-grid {
    grid-template-columns: 1fr;
  }

  .language-bar-item {
    flex-direction: column;
    align-items: flex-start;
    gap: 8px;
  }

  .language-name {
    width: auto;
    text-align: left;
  }

  .language-count {
    width: auto;
  }
}
</style>
