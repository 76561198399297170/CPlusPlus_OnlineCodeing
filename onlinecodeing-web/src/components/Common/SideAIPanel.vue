<!-- src/components/Common/SideAIPanel.vue -->
<template>
  <div v-if="visible" class="ai-panel-overlay" @click="handleOverlayClick">
    <div class="ai-panel" :class="{ 'panel-open': visible }" @click.stop>
      <!-- 面板头部 -->
      <div class="panel-header">
        <div class="panel-title">
          <span class="ai-icon">🤖</span>
          <span>{{ panelTitle }}</span>
        </div>
        <button class="panel-close" @click="handleClose">×</button>
      </div>

      <!-- 面板内容 -->
      <div class="panel-content">
        <!-- 辅助功能选项 -->
        <div v-if="!activeFunction" class="assistant-functions">
          <div
            v-for="func in functions"
            :key="func.id"
            class="function-item"
            @click="handleFunctionClick(func)"
          >
            <div class="function-icon">{{ func.icon }}</div>
            <div class="function-info">
              <div class="function-name">{{ func.name }}</div>
              <div class="function-desc">{{ func.description }}</div>
            </div>
            <div class="function-arrow">→</div>
          </div>
        </div>

        <!-- 功能执行区域 -->
        <div v-else class="function-execution">
          <div class="function-header">
            <div class="function-title">{{ activeFunction.name }}</div>
            <Button type="default" size="small" @click="resetPanel"> ← 返回 </Button>
          </div>

          <div class="function-content">
            <div class="function-desc">{{ activeFunction.description }}</div>

            <!-- AI 审核模式提示 -->
            <div v-if="isBatchMode" class="batch-mode-info">
              <div class="batch-info-header">
                <span class="batch-icon">📋</span>
                <span class="batch-title">AI 审核模式</span>
              </div>
              <div class="batch-progress" v-if="batchProgress.total > 0">
                <div class="progress-bar">
                  <div
                    class="progress-fill"
                    :style="{ width: batchProgress.percentage + '%' }"
                  ></div>
                </div>
                <div class="progress-text">
                  进度: {{ batchProgress.current }} / {{ batchProgress.total }} ({{
                    batchProgress.percentage
                  }}%)
                </div>
              </div>
              <div class="batch-status" v-if="batchProgress.current > 0">
                <span class="status-item passed">通过: {{ batchProgress.passed }}</span>
                <span class="status-item rejected">不通过: {{ batchProgress.rejected }}</span>
              </div>
            </div>

            <div class="input-section">
              <label class="input-label">需求提示</label>
              <textarea v-model="userInput" placeholder="请输入您的需求..." rows="4"></textarea>
            </div>

            <div v-if="loading" class="loading-section">
              <div class="loading-indicator">
                <div class="loading-spinner"></div>
                <div class="loading-text">
                  {{ isBatchMode ? 'AI 审核中...' : '思考中...' }}
                </div>
              </div>
            </div>

            <div v-else class="result-section" v-if="aiResult">
              <div class="result-header">AI 结果</div>
              <div class="result-content">{{ aiResult }}</div>
              <Button
                v-if="props.mode === 'create' || (props.mode === 'review' && !isBatchMode)"
                type="primary"
                size="medium"
                class="apply-btn"
                @click="applyResult"
                :disabled="!aiResult || aiQuestionData === null"
              >
                应用结果
              </Button>
            </div>
          </div>

          <div class="function-actions">
            <div class="button-container">
              <div class="right-buttons">
                <Button type="default" size="medium" @click="resetPanel"> 取消 </Button>
                <Button type="primary" size="medium" @click="handleGenerate" :loading="loading">
                  {{
                    loading
                      ? isBatchMode
                        ? '审核中...'
                        : '生成中...'
                      : isBatchMode
                        ? '开始 AI 审核'
                        : '生成'
                  }}
                </Button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, nextTick } from 'vue'
import { apiService } from '@/services/api'
import Button from '@/components/Common/Button.vue'

const props = defineProps({
  visible: {
    type: Boolean,
    default: false,
  },
  mode: {
    type: String,
    default: 'create', // create 或 review
    validator: (value) => ['create', 'review'].includes(value),
  },
  context: {
    type: Object,
    default: () => ({}),
  },
  // 批量审核相关属性
  problemsList: {
    type: Array,
    default: () => [],
  },
  isBatchMode: {
    type: Boolean,
    default: false,
  },
})

const emit = defineEmits(['close', 'apply', 'batchComplete'])

// 响应式数据
const activeFunction = ref(null)
const userInput = ref('')
const loading = ref(false)
const aiResult = ref('')
const aiQuestionData = ref(null)

// 批量审核进度
const batchProgress = ref({
  current: 0,
  total: 0,
  passed: 0,
  rejected: 0,
  percentage: 0,
})

// 创建题目的辅助功能
const createFunctions = [
  {
    id: 'generate-description',
    name: '生成题目描述',
    description: '根据关键词生成详细的题目描述',
    icon: '📄',
  },
  {
    id: 'generate-test-cases',
    name: '生成测试用例',
    description: '自动生成多种测试用例，包括边界情况',
    icon: '🧪',
  },
  {
    id: 'evaluate-difficulty',
    name: '评估题目难度',
    description: '根据题目内容智能评估难度等级',
    icon: '📊',
  },
  {
    id: 'pre-check',
    name: '题目预先检查',
    description: '检查题目描述中的语法和表达问题',
    icon: '✏️',
  },
  {
    id: 'add-tags',
    name: '推荐标签',
    description: '根据题目内容推荐合适的标签',
    icon: '🏷️',
  },
  {
    id: 'complete-problem',
    name: '补齐题目信息',
    description: '根据当前已填信息补齐题目内容',
    icon: '🔧',
  },
]

// 审核题目的辅助功能
const reviewFunctions = [
  {
    id: 'check-completeness',
    name: '完整性检查',
    description: '检查题目是否包含所有必要信息',
    icon: '✅',
  },
  {
    id: 'plagiarism-check',
    name: '查重检测',
    description: '检测题目与现有题目的相似度',
    icon: '🔍',
  },
  {
    id: 'difficulty-review',
    name: '难度复核',
    description: 'AI 评估题目难度是否合理',
    icon: '⚖️',
  },
  {
    id: 'generate-review',
    name: '生成审核意见',
    description: '自动生成审核建议和意见',
    icon: '💡',
  },
  {
    id: 'validate-test-cases',
    name: '验证测试用例',
    description: '检查测试用例的合理性和完整性',
    icon: '🧪',
  },
  {
    id: 'language-check',
    name: '语言规范检查',
    description: '检查题目描述的语言规范性',
    icon: '📖',
  },
  {
    id: 'batch-review',
    name: 'AI 审核',
    description: '对所有待审核题目进行 AI 审核',
    icon: '📋',
  },
]

// 计算当前模式的功能列表
const functions = computed(() => {
  return props.mode === 'create' ? createFunctions : reviewFunctions
})

// 计算面板标题
const panelTitle = computed(() => {
  if (props.isBatchMode) return 'AI 审核助手'
  return props.mode === 'create' ? 'AI 题目创建助手' : 'AI 题目审核助手'
})

// 处理功能点击
const handleFunctionClick = (func) => {
  activeFunction.value = func
  userInput.value = ''
  aiResult.value = ''
  aiQuestionData.value = null
  // 重置批量进度
  batchProgress.value = {
    current: 0,
    total: 0,
    passed: 0,
    rejected: 0,
    percentage: 0,
  }
}

// 获取题目详情
const fetchProblemDetail = async (problemId) => {
  try {
    const response = await apiService.getProblem(problemId)
    if (response.code === 0) {
      return response.data
    }
    return null
  } catch (error) {
    return null
  }
}

// AI 审核处理
const handleBatchReview = async () => {
  if (!activeFunction.value) {
    aiResult.value = '请先选择一个功能'
    return
  }

  if (!props.problemsList || props.problemsList.length === 0) {
    aiResult.value = '暂无待审核题目'
    return
  }

  const totalProblems = props.problemsList.length
  batchProgress.value = {
    current: 0,
    total: totalProblems,
    passed: 0,
    rejected: 0,
    percentage: 0,
  }

  const batchResults = []

  for (let i = 0; i < props.problemsList.length; i++) {
    const problem = props.problemsList[i]
    batchProgress.value.current = i + 1
    batchProgress.value.percentage = Math.round(((i + 1) / totalProblems) * 100)

    try {
      // 获取题目详情（如果信息不全）
      let problemDetail = problem
      if (!problem.description || !problem.inputFormat || !problem.outputFormat) {
        problemDetail = await fetchProblemDetail(problem.id)
        if (!problemDetail) {
          continue
        }
      }

      // 构建请求参数
      // 批量审核使用 check-completeness 工具循环处理所有题目
      const fullFunctionId = 'review-check-completeness'

      // 转换字段名以匹配后端期望
      const questionData = {
        title: problemDetail?.title || '',
        difficulty: problemDetail?.difficulty || -1,
        tags: problemDetail?.tags || [],
        problem_content: problemDetail?.description || '',
        input_content: problemDetail?.inputFormat || '',
        output_content: problemDetail?.outputFormat || '',
        test_case: [],
      }

      // 转换测试用例格式
      if (problemDetail?.testCases && Array.isArray(problemDetail.testCases)) {
        questionData.test_case = problemDetail.testCases.map((testCase) => {
          if (typeof testCase === 'object' && testCase.input && testCase.output) {
            return [testCase.input, testCase.output]
          }
          return ['', '']
        })
      }

      const requestData = {
        mode: 'review',
        functionId: fullFunctionId,
        context: userInput.value,
        question: questionData,
        toolId: fullFunctionId,
      }

      // 调用后端 API 获取 AI 响应
      const response = await apiService.getAIHelp(requestData)

      if (response.code === 0 && response.data.reviewRes !== undefined) {
        const reviewResult = response.data.reviewRes ? 1 : 0

        if (reviewResult === 1) {
          batchProgress.value.passed++
        } else {
          batchProgress.value.rejected++
        }

        batchResults.push({
          problemId: problem.id,
          reviewRes: reviewResult,
          reviewMsg: response.data.reviewMsg,
        })

        // 实时发送单个结果给父组件
        emit('apply', {
          problemId: problem.id,
          reviewRes: reviewResult,
          reviewMsg: response.data.reviewMsg,
          isBatch: true,
        })
      }
    } catch (error) {
      // 单个题目审核失败，继续处理下一个
    }
  }

  // 生成 AI 审核总结
  let summary = `## AI 审核完成\n\n`
  summary += `共审核 ${totalProblems} 道题目\n`
  summary += `通过: ${batchProgress.value.passed} 道\n`
  summary += `不通过: ${batchProgress.value.rejected} 道\n\n`
  summary += `通过率: ${Math.round((batchProgress.value.passed / totalProblems) * 100)}%`

  aiResult.value = summary
  aiQuestionData.value = {
    batchResults: batchResults,
    summary: summary,
  }

  // 发送 AI 审核完成事件
  emit('batchComplete', {
    total: totalProblems,
    passed: batchProgress.value.passed,
    rejected: batchProgress.value.rejected,
    results: batchResults,
  })
}

// 生成 AI 结果
const handleGenerate = async () => {
  if (loading.value) return

  // 检查是否选择了功能
  if (!activeFunction.value) {
    aiResult.value = '请先选择一个功能'
    return
  }

  // 如果是 AI 审核模式
  if (props.isBatchMode && activeFunction.value.id === 'batch-review') {
    loading.value = true
    await handleBatchReview()
    loading.value = false
    return
  }

  // 如果是review模式且没有选中题目，提示用户
  if (props.mode === 'review' && !props.context?.id && !props.isBatchMode) {
    aiResult.value = '请先选择一道题目，或使用批量审核功能'
    return
  }

  loading.value = true
  aiResult.value = ''

  try {
    // 构建请求参数
    const functionId = activeFunction.value?.id || ''
    const fullFunctionId = `${props.mode}-${functionId}`

    // 转换字段名以匹配后端期望
    const questionData = {
      title: props.context?.title || '',
      difficulty: props.context?.difficulty || -1,
      tags: props.context?.tags || [],
      problem_content: props.context?.description || '',
      input_content: props.context?.inputFormat || '',
      output_content: props.context?.outputFormat || '',
      test_case: [],
    }

    // 转换测试用例格式
    if (props.context?.testCases && Array.isArray(props.context.testCases)) {
      questionData.test_case = props.context.testCases.map((testCase) => {
        if (typeof testCase === 'object' && testCase.input && testCase.output) {
          return [testCase.input, testCase.output]
        }
        return ['', '']
      })
    }

    const requestData = {
      mode: props.mode,
      functionId: fullFunctionId,
      context: userInput.value,
      question: questionData,
      toolId: fullFunctionId,
    }

    // 调用后端 API 获取 AI 响应
    const response = await apiService.getAIHelp(requestData)

    if (response.code === 0) {
      // 根据模式处理不同的响应格式
      if (props.mode === 'create') {
        // 检查是否包含完整的题目信息（可能直接在data中，也可能在data.question中）
        if (response.data.description || response.data.question) {
          const questionData = response.data.question || response.data
          // 构建详细的预览信息
          let preview = `# ${questionData.title || '未设置标题'}\n\n`
          preview += `## 题目描述\n${questionData.description || '未设置描述'}\n\n`
          preview += `## 输入格式\n${questionData.inputFormat || '未设置输入格式'}\n\n`
          preview += `## 输出格式\n${questionData.outputFormat || '未设置输出格式'}\n\n`
          if (questionData.tags && questionData.tags.length > 0) {
            preview += `## 标签\n${questionData.tags.join(', ')}\n\n`
          }
          if (questionData.difficulty) {
            const difficultyMap = { 1: '入门', 2: '基础', 3: '中等', 4: '进阶', 5: '困难' }
            preview += `## 难度\n${difficultyMap[questionData.difficulty] || '未设置'}\n\n`
          }
          if (questionData.testCases && questionData.testCases.length > 0) {
            preview += `## 测试用例\n`
            questionData.testCases.forEach((testCase, index) => {
              if (Array.isArray(testCase) && testCase.length === 2) {
                preview += `### 测试用例 ${index + 1}\n输入: ${testCase[0]}\n输出: ${testCase[1]}\n\n`
              } else if (typeof testCase === 'object' && testCase.input && testCase.output) {
                preview += `### 测试用例 ${index + 1}\n输入: ${testCase.input}\n输出: ${testCase.output}\n\n`
              }
            })
          }
          aiResult.value = preview
          // 存储题目数据，供应用结果时使用
          aiQuestionData.value = questionData
        } else {
          // 如果没有完整的题目信息，只显示content
          aiResult.value = response.data.content
        }
      } else if (props.mode === 'review') {
        // 处理review模式的响应
        console.log('review mode response:', response.data)
        if (response.data.reviewRes !== undefined) {
          let preview = `## 审查结果\n`
          preview += `状态: ${response.data.reviewRes ? '通过' : '不通过'}\n\n`
          if (response.data.reviewMsg) {
            preview += `## 审查建议\n${response.data.reviewMsg}\n`
          }
          aiResult.value = preview
          // 存储审核结果，供应用时使用
          // 包含 problemId 以便正确应用到题目列表
          aiQuestionData.value = {
            reviewRes: response.data.reviewRes ? 1 : 0,
            reviewMsg: response.data.reviewMsg,
            problemId: props.context?.id,
          }
          console.log('aiQuestionData set:', aiQuestionData.value)

          // 自动应用结果（无需点击按钮）
          // 无论是单个审核还是批量审核中的单题审核，都自动应用
          console.log('Checking auto apply conditions:', {
            isBatchMode: props.isBatchMode,
            contextId: props.context?.id,
          })
          if (props.context?.id) {
            console.log('Auto applying review result for problem:', props.context.id)
            nextTick(() => {
              console.log('nextTick callback, calling applyResult')
              applyResult()
            })
          } else {
            console.log('Auto apply skipped: no contextId')
          }
        } else {
          // 如果没有reviewRes，只显示content
          console.log('reviewRes is undefined, setting aiQuestionData to null')
          aiResult.value = response.data.content
          aiQuestionData.value = null
        }
      }
    } else {
      aiResult.value = response.msg || 'AI 响应失败，请重试'
    }
  } catch (error) {
    aiResult.value = '网络错误，无法获取 AI 响应'
  } finally {
    loading.value = false
  }
}

// 重置面板
const resetPanel = () => {
  activeFunction.value = null
  userInput.value = ''
  aiResult.value = ''
  aiQuestionData.value = null
  batchProgress.value = {
    current: 0,
    total: 0,
    passed: 0,
    rejected: 0,
    percentage: 0,
  }
}

// 关闭面板
const handleClose = () => {
  emit('close')
  resetPanel()
}

// 点击遮罩层
const handleOverlayClick = () => {
  handleClose()
}

// 应用结果
const applyResult = () => {
  console.log('applyResult called, aiQuestionData:', aiQuestionData.value)
  if (aiQuestionData.value) {
    // 使用存储的题目数据
    console.log('applyResult: emitting aiQuestionData')
    emit('apply', aiQuestionData.value)
    // 清除存储的数据
    aiQuestionData.value = null
  } else if (props.mode === 'review' && props.context?.id) {
    // review模式下，如果没有aiQuestionData，但有题目ID，尝试从aiResult解析结果
    // 这种情况可能发生在后端返回格式不符合预期时
    console.warn('applyResult: aiQuestionData is null, trying to parse from aiResult')
    emit('apply', {
      problemId: props.context.id,
      reviewRes: 0, // 默认不通过
      reviewMsg: aiResult.value || '审核结果',
    })
  } else if (aiResult.value) {
    // 如果没有存储的题目数据，直接传递aiResult.value
    console.log('applyResult: emitting aiResult')
    emit('apply', aiResult.value)
  }
}

// 暴露给父组件的方法
defineExpose({
  applyResult,
})
</script>

<style scoped>
/* 遮罩层 */
.ai-panel-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, 0.5);
  z-index: 9998;
  animation: fadeIn 0.3s ease;
}

/* 面板 */
.ai-panel {
  position: fixed;
  top: 0;
  right: 0;
  bottom: 0;
  width: 450px;
  max-width: 100%;
  background-color: #ffffff;
  box-shadow: -4px 0 20px rgba(0, 0, 0, 0.15);
  display: flex;
  flex-direction: column;
  transform: translateX(100%);
  transition: transform 0.3s ease;
  z-index: 9999;
}

.panel-open {
  transform: translateX(0);
}

/* 面板头部 */
.panel-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 24px;
  border-bottom: 1px solid #e5e6eb;
  background: linear-gradient(135deg, #165dff 0%, #4080ff 100%);
}

.panel-title {
  display: flex;
  align-items: center;
  gap: 10px;
  font-size: 18px;
  font-weight: 600;
  color: #ffffff;
}

.ai-icon {
  font-size: 24px;
}

.panel-close {
  width: 32px;
  height: 32px;
  border: none;
  background: rgba(255, 255, 255, 0.2);
  color: #ffffff;
  font-size: 24px;
  line-height: 1;
  cursor: pointer;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s ease;
}

.panel-close:hover {
  background: rgba(255, 255, 255, 0.3);
}

/* 面板内容 */
.panel-content {
  flex: 1;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
}

/* 辅助功能列表 */
.assistant-functions {
  padding: 16px;
}

.function-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 16px;
  margin-bottom: 12px;
  background-color: #f7f8fa;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s ease;
  border: 1px solid transparent;
}

.function-item:hover {
  background-color: #e8f0ff;
  border-color: #165dff;
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(22, 93, 255, 0.15);
}

.function-icon {
  font-size: 28px;
  flex-shrink: 0;
}

.function-info {
  flex: 1;
  min-width: 0;
}

.function-name {
  font-size: 15px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 4px;
}

.function-desc {
  font-size: 13px;
  color: #86909c;
  line-height: 1.4;
}

.function-arrow {
  font-size: 18px;
  color: #165dff;
  font-weight: 300;
}

/* 功能执行区域 */
.function-execution {
  flex: 1;
  display: flex;
  flex-direction: column;
  height: 100%;
}

.function-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border-bottom: 1px solid #e5e6eb;
  background-color: #f7f8fa;
}

.function-title {
  font-size: 15px;
  font-weight: 600;
  color: #1d2129;
}

.function-content {
  flex: 1;
  padding: 20px;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.function-content .function-desc {
  font-size: 14px;
  color: #86909c;
  line-height: 1.4;
  margin-bottom: 8px;
}

/* 批量审核模式信息 */
.batch-mode-info {
  background: linear-gradient(135deg, #e8f0ff 0%, #f0f7ff 100%);
  border: 1px solid #165dff;
  border-radius: 8px;
  padding: 16px;
  margin-bottom: 16px;
}

.batch-info-header {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 12px;
}

.batch-icon {
  font-size: 20px;
}

.batch-title {
  font-size: 15px;
  font-weight: 600;
  color: #165dff;
}

.batch-progress {
  margin-bottom: 12px;
}

.progress-bar {
  width: 100%;
  height: 8px;
  background-color: #e5e6eb;
  border-radius: 4px;
  overflow: hidden;
  margin-bottom: 8px;
}

.progress-fill {
  height: 100%;
  background: linear-gradient(90deg, #165dff 0%, #4080ff 100%);
  transition: width 0.3s ease;
}

.progress-text {
  font-size: 13px;
  color: #4e5969;
  text-align: center;
}

.batch-status {
  display: flex;
  gap: 16px;
  justify-content: center;
}

.status-item {
  font-size: 13px;
  font-weight: 500;
  padding: 4px 12px;
  border-radius: 12px;
}

.status-item.passed {
  background-color: #e8f5e8;
  color: #00b42a;
}

.status-item.rejected {
  background-color: #fce4ec;
  color: #f53f3f;
}

/* 输入区域 */
.input-section {
  margin-bottom: 16px;
}

.input-label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.input-section textarea {
  width: 100%;
  padding: 12px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  font-size: 14px;
  resize: vertical;
  font-family: inherit;
  transition: all 0.2s ease;
  min-height: 100px;
}

.input-section textarea:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
}

/* 加载区域 */
.loading-section {
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 40px 0;
}

.loading-indicator {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 12px;
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 4px solid #e5e6eb;
  border-top: 4px solid #165dff;
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

.loading-text {
  font-size: 14px;
  color: #86909c;
}

@keyframes spin {
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
}

/* 结果区域 */
.result-section {
  background-color: #f7f8fa;
  border-radius: 8px;
  padding: 16px;
  margin-top: 16px;
}

.result-header {
  font-size: 15px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 12px;
}

.result-content {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.6;
  white-space: pre-wrap;
  word-break: break-word;
  margin-bottom: 16px;
}

.apply-btn {
  width: 100%;
  margin-top: 8px;
}

/* 功能操作按钮 */
.function-actions {
  padding: 16px;
  border-top: 1px solid #e5e6eb;
  background-color: #ffffff;
}

/* 按钮容器 */
.button-container {
  display: flex;
  gap: 12px;
  align-items: center;
}

/* 右侧按钮容器 */
.right-buttons {
  display: flex;
  gap: 12px;
  align-items: center;
}

/* 应用结果按钮样式 */
.button-container .apply-btn {
  flex: 1;
  min-width: 0;
  height: 40px; /* 固定高度，与medium尺寸按钮一致 */
}

/* 确保所有按钮高度一致 */
.button-container Button {
  height: 40px; /* 固定高度，与medium尺寸按钮一致 */
}

/* 动画 */
@keyframes fadeIn {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

/* 响应式 */
@media (max-width: 768px) {
  .ai-panel {
    width: 100%;
  }
}
</style>
