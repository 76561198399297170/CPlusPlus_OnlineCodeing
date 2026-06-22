<!-- src/views/editor/Index.vue -->
<template>
  <MainLayout>
    <div class="editor-page">
      <!-- 复制成功提示 -->
      <div class="copy-message" v-if="copyMessage" v-html="copyMessage"></div>

      <!-- 主编辑区域 -->
      <div class="editor-container">
        <!-- 左侧题目区域 -->
        <div class="problem-section" ref="problemSection">
          <ProblemDescription
            :problem="problem"
            :loading="loading"
            :error="error"
            @retry="fetchProblem"
            @code-copy="handleCodeCopy"
            @apply-to-test="handleApplyToTest"
          />
        </div>

        <!-- 分隔线 -->
        <div class="resizer" @mousedown="startResizing"></div>

        <!-- 右侧代码编辑区域 -->
        <div class="code-section" ref="codeSection">
          <!-- 代码编辑器 -->
          <CodeEditor v-model="code" v-model:language="selectedLanguage" />

          <!-- 测试管理器 -->
          <TestManager
            v-model:tests="tests"
            v-model:activeTestTab="activeTestTab"
            @run-test="handleRunTest"
            @submit-code="submitCode"
          />
        </div>
      </div>
    </div>

    <!-- 成功弹窗 -->
    <SuccessDialog
      v-if="showSuccessDialog"
      :visible="showSuccessDialog"
      :title="dialogTitle"
      :message="dialogMessage"
      sound-path="/sounds/success.mp3"
      @close="showSuccessDialog = false"
    />

    <!-- 错误弹窗 -->
    <CustomDialog
      v-if="showErrorDialog"
      :visible="showErrorDialog"
      :title="dialogTitle"
      type="error"
      :buttons="[{ text: '确定', type: 'error', callback: () => (showErrorDialog = false) }]"
      @close="showErrorDialog = false"
    >
      <p>{{ dialogMessage }}</p>
    </CustomDialog>
  </MainLayout>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import CustomDialog from '@/components/Common/CustomDialog.vue'
import SuccessDialog from '@/components/Common/SuccessDialog.vue'
import ProblemDescription from '@/components/Editor/ProblemDescription.vue'
import CodeEditor from '@/components/Editor/CodeEditor.vue'
import TestManager from '@/components/Editor/TestManager.vue'
import { apiService } from '@/services/api'
import { cacheService } from '@/services/cache'

// 路由实例
const route = useRoute()
const router = useRouter()

// 响应式数据
const problemSection = ref(null)
const codeSection = ref(null)
const isResizing = ref(false)
const selectedLanguage = ref('JavaScript')
const code = ref('')
const activeTestTab = ref(null)
const problemId = ref(Number(route.params.id) || 0)
const problemSetId = ref(route.query.problemSetId || null)
const problem = ref(null)
const loading = ref(false)
const error = ref('')

// 弹窗状态
const showSuccessDialog = ref(false)
const showErrorDialog = ref(false)
const dialogTitle = ref('')
const dialogMessage = ref('')

// 测试用例
const tests = ref([])

// 复制成功提示
const copyMessage = ref('')

// 使用防抖保存缓存
let saveTimer = null
const saveToCache = () => {
  if (saveTimer) {
    clearTimeout(saveTimer)
  }
  saveTimer = setTimeout(() => {
    cacheService.setEditorContent(
      problemId.value,
      problemSetId.value,
      code.value,
      selectedLanguage.value,
    )
  }, 500)
}

// 处理代码复制
const handleCodeCopy = () => {
  copyMessage.value = '<span style="color: #52c41a;">代码已复制到剪贴板</span>'
  setTimeout(() => {
    copyMessage.value = ''
  }, 1500)
}

// 处理应用到测试
const handleApplyToTest = (data) => {
  tests.value.push({
    name: `测试 ${tests.value.length + 1}`,
    input: data.input || '',
    output: data.output || '',
    result: null,
  })
  activeTestTab.value = tests.value.length - 1
}

// 解析样例并创建测试用例
const parseSampleAndCreateTests = (sample) => {
  // 清空现有的测试用例
  tests.value = []

  // 分割多个样例
  const sampleParts = sample.split(/样例\d+/).filter((part) => part.trim())

  sampleParts.forEach((part, index) => {
    // 解析输入
    const inputMatch = part.match(/[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/)
    const input = inputMatch ? inputMatch[1].trim() : ''

    // 解析输出
    const outputMatch = part.match(/[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/)
    const output = outputMatch ? outputMatch[1].trim() : ''

    // 创建测试用例
    if (input || output) {
      tests.value.push({
        name: `样例 ${index + 1}`,
        input: input,
        output: output,
        result: null,
      })
    }
  })

  // 如果创建了测试用例，激活第一个
  if (tests.value.length > 0) {
    activeTestTab.value = 0
  }
}

// 获取题目数据
const fetchProblem = async () => {
  try {
    loading.value = true
    error.value = ''

    // 确定题目ID，确保始终是数字类型
    const targetProblemId = problemId.value === 0 ? 0 : problemId.value

    // 从API获取（每次都请求，不使用缓存）
    const response = await apiService.getProblem(targetProblemId, problemSetId.value)
    if (response.code === 0) {
      problem.value = response.data

      // 自动解析样例并创建测试用例
      if (problem.value.sample) {
        parseSampleAndCreateTests(problem.value.sample)
      }
    } else {
      error.value = response.msg || '获取题目失败'
    }
  } catch (err) {
    error.value = err.msg || '网络错误，无法获取题目'
  } finally {
    loading.value = false
  }
}

// 运行测试
const handleRunTest = async (testIndex) => {
  try {
    loading.value = true
    error.value = ''

    // 确定题目ID，确保始终是数字类型
    const targetProblemId = problemId.value === 0 ? 0 : problemId.value

    // 验证题目ID
    if (targetProblemId === 0) {
      dialogTitle.value = '测试失败'
      dialogMessage.value = '请先选择一个题目'
      showErrorDialog.value = true
      return
    }

    const testCase = {
      input: tests.value[testIndex].input,
      output: tests.value[testIndex].output,
    }

    const response = await apiService.runTest(
      targetProblemId,
      code.value,
      selectedLanguage.value,
      testCase,
      problemSetId.value || undefined,
    )

    if (response.code === 0) {
      const result = response.data
      tests.value[testIndex].result = {
        status: result.status,
        output: result.output,
        error: result.errorMessage || '',
      }

      // 检查是否是服务器繁忙的情况
      const isServerBusy =
        (result.errorMessage &&
          (result.errorMessage === 'retry' || result.errorMessage.includes('重试'))) ||
        (response.msg && (response.msg === 'retry' || response.msg.includes('重试')))

      if (isServerBusy) {
        dialogTitle.value = '服务器繁忙'
        dialogMessage.value = '服务器当前繁忙，请稍后重试...'
        showErrorDialog.value = true
      } else if (result.status === 'AC') {
        dialogTitle.value = '测试通过！'
        dialogMessage.value = '恭喜你，测试用例通过了！'
        showSuccessDialog.value = true
      } else {
        let errorMessage = ''
        switch (result.status) {
          case 'WA':
            errorMessage = '答案错误，请检查你的代码逻辑。'
            break
          case 'CE':
            errorMessage = '编译错误，请检查你的代码语法。'
            break
          case 'TLE':
            errorMessage = '超时错误，你的代码运行时间过长。'
            break
          case 'ERROR':
            errorMessage = '运行错误，请检查你的代码是否有异常。'
            break
          default:
            errorMessage = '测试失败，请重试。'
        }
        // 如果有错误信息，添加到错误消息中
        if (result.errorMessage) {
          errorMessage += '\n\n错误详情：\n' + result.errorMessage
        }
        dialogTitle.value = '测试失败'
        dialogMessage.value = errorMessage
        showErrorDialog.value = true
      }
    } else {
      if (response.msg === '题目截止') {
        dialogTitle.value = '测试失败'
        dialogMessage.value = '该竞赛题目已截止提交，无法运行测试。'
      } else if (response.msg === 'discussion not start') {
        dialogTitle.value = '测试失败'
        dialogMessage.value = '该题目组尚未开始，无法运行测试。'
      } else if (response.msg === 'discussion end') {
        dialogTitle.value = '测试失败'
        dialogMessage.value = '该题目组已结束，无法运行测试。'
      } else if (response.msg === 'retry' || response.msg.includes('重试')) {
        dialogTitle.value = '服务器繁忙'
        dialogMessage.value = '服务器当前繁忙，请稍后重试...'
      } else {
        dialogTitle.value = '测试失败'
        dialogMessage.value = response.msg || '运行测试失败'
      }
      showErrorDialog.value = true
    }
  } catch (err) {
    dialogTitle.value = '测试失败'
    dialogMessage.value = err.msg || '网络错误，无法运行测试'
    showErrorDialog.value = true
  } finally {
    loading.value = false
  }
}

// 提交代码
const submitCode = async () => {
  try {
    loading.value = true
    error.value = ''

    // 确定题目ID，确保始终是数字类型
    const targetProblemId = problemId.value === 0 ? 0 : problemId.value

    // 验证题目ID
    if (targetProblemId === 0) {
      dialogTitle.value = '提交失败'
      dialogMessage.value = '请先选择一个题目'
      showErrorDialog.value = true
      return
    }

    const response = await apiService.submitCode(
      targetProblemId,
      code.value,
      selectedLanguage.value,
      problemSetId.value || undefined,
    )

    if (response.code === 0) {
      const result = response.data

      // 查找是否已存在提交结果选项卡
      let submissionTabIndex = -1
      for (let i = 0; i < tests.value.length; i++) {
        if (tests.value[i].name === '提交结果') {
          submissionTabIndex = i
          break
        }
      }

      // 创建或更新提交结果选项卡
      const submissionResult = {
        name: '提交结果',
        input: '',
        output: '',
        result: {
          status: result.status,
          output: `执行时间: ${result.executionTime}ms\n内存使用: ${result.memoryUsed}MB\n得分: ${result.score}`,
          error: result.error || result.errorMessage || '',
        },
      }

      if (submissionTabIndex >= 0) {
        // 覆盖已存在的提交结果选项卡
        tests.value[submissionTabIndex] = submissionResult
      } else {
        // 创建新的提交结果选项卡
        tests.value.push(submissionResult)
        submissionTabIndex = tests.value.length - 1
      }

      // 切换到提交结果选项卡
      activeTestTab.value = submissionTabIndex

      // 检查是否是服务器繁忙的情况
      const isServerBusy =
        (result.error && (result.error === 'retry' || result.error.includes('重试'))) ||
        (result.errorMessage &&
          (result.errorMessage === 'retry' || result.errorMessage.includes('重试'))) ||
        (response.msg && (response.msg === 'retry' || response.msg.includes('重试')))

      if (isServerBusy) {
        dialogTitle.value = '服务器繁忙'
        dialogMessage.value = '服务器当前繁忙，请稍后重试...'
        showErrorDialog.value = true
      } else if (result.status === 'AC') {
        dialogTitle.value = '代码提交成功！'
        dialogMessage.value = `恭喜你，所有测试用例都通过了！\n\n执行时间: ${result.executionTime}ms\n内存使用: ${result.memoryUsed}MB\n得分: ${result.score}`
        showSuccessDialog.value = true
      } else {
        let errorMessage = ''
        switch (result.status) {
          case 'WA':
            errorMessage = '答案错误，请检查你的代码逻辑。'
            break
          case 'CE':
            errorMessage = '编译错误，请检查你的代码语法。'
            break
          case 'TLE':
            errorMessage = '超时错误，你的代码运行时间过长。'
            break
          case 'ERROR':
            errorMessage = '运行错误，请检查你的代码是否有异常。'
            break
          default:
            errorMessage = '代码提交失败，请重试。'
        }
        // 添加执行信息
        errorMessage += `\n\n执行时间: ${result.executionTime}ms\n内存使用: ${result.memoryUsed}MB\n得分: ${result.score}`
        // 如果有错误信息，添加到错误消息中
        if (result.error || result.errorMessage) {
          errorMessage += '\n\n错误详情：\n' + (result.error || result.errorMessage)
        }
        dialogTitle.value = '代码提交失败'
        dialogMessage.value = errorMessage
        showErrorDialog.value = true
      }
    } else {
      if (response.msg === '题目截止') {
        dialogTitle.value = '提交失败'
        dialogMessage.value = '该竞赛题目已截止提交，无法提交代码。'
      } else if (response.msg === 'discussion not start') {
        dialogTitle.value = '提交失败'
        dialogMessage.value = '该题目组尚未开始，无法提交代码。'
      } else if (response.msg === 'discussion end') {
        dialogTitle.value = '提交失败'
        dialogMessage.value = '该题目组已结束，无法提交代码。'
      } else if (response.msg === 'retry' || response.msg.includes('重试')) {
        dialogTitle.value = '服务器繁忙'
        dialogMessage.value = '服务器当前繁忙，请稍后重试...'
      } else {
        dialogTitle.value = '代码提交失败'
        dialogMessage.value = response.msg || '提交代码失败'
      }
      showErrorDialog.value = true
    }
  } catch (err) {
    dialogTitle.value = '代码提交失败'
    dialogMessage.value = err.msg || '网络错误，无法提交代码'
    showErrorDialog.value = true
  } finally {
    loading.value = false
  }
}

// 调整大小相关
const startResizing = (e) => {
  e.preventDefault()
  e.stopPropagation()
  isResizing.value = true
  document.addEventListener('mousemove', resize)
  document.addEventListener('mouseup', stopResizing)
  // 添加一个覆盖层防止文本选择
  const overlay = document.createElement('div')
  overlay.style.position = 'fixed'
  overlay.style.top = '0'
  overlay.style.left = '0'
  overlay.style.width = '100%'
  overlay.style.height = '100%'
  overlay.style.zIndex = '9999'
  overlay.style.cursor = 'col-resize'
  overlay.id = 'resizing-overlay'
  document.body.appendChild(overlay)
}

const resize = (e) => {
  e.preventDefault()
  e.stopPropagation()

  if (problemSection.value && codeSection.value) {
    const containerRect = problemSection.value.parentElement.getBoundingClientRect()
    const newProblemWidth = ((e.clientX - containerRect.left) / containerRect.width) * 100

    // 限制最小宽度为20%，最大宽度为80%
    if (newProblemWidth >= 20 && newProblemWidth <= 80) {
      problemSection.value.style.width = `${newProblemWidth}%`
      problemSection.value.style.minWidth = `${newProblemWidth}%`
      problemSection.value.style.maxWidth = `${newProblemWidth}%`

      const newCodeWidth = 100 - newProblemWidth
      codeSection.value.style.width = `${newCodeWidth}%`
      codeSection.value.style.minWidth = `${newCodeWidth}%`
      codeSection.value.style.maxWidth = `${newCodeWidth}%`
    }
  }
}

const stopResizing = () => {
  isResizing.value = false
  document.removeEventListener('mousemove', resize)
  document.removeEventListener('mouseup', stopResizing)
  // 移除覆盖层
  const overlay = document.getElementById('resizing-overlay')
  if (overlay) {
    document.body.removeChild(overlay)
  }
}

// 初始化
onMounted(() => {
  // 尝试从缓存加载编辑器内容
  const cachedEditor = cacheService.getEditorContent(problemId.value, problemSetId.value)
  if (cachedEditor) {
    code.value = cachedEditor.code
    selectedLanguage.value = cachedEditor.language
  }

  // 获取题目数据
  fetchProblem()
})

onUnmounted(() => {
  // 清理事件监听器
  document.removeEventListener('mousemove', resize)
  document.removeEventListener('mouseup', stopResizing)
})

// 监听代码变化，自动保存
watch(code, () => {
  saveToCache()
})

// 监听语言变化，自动保存
watch(selectedLanguage, () => {
  saveToCache()
})

// 监听路由变化，重新加载题目
watch(
  () => [route.params.id, route.query.problemSetId],
  ([newId, newProblemSetId]) => {
    const newProblemId = Number(newId) || 0
    const newPsId = newProblemSetId || null

    if (newProblemId !== problemId.value || newPsId !== problemSetId.value) {
      problemId.value = newProblemId
      problemSetId.value = newPsId

      // 尝试从缓存加载编辑器内容
      const cachedEditor = cacheService.getEditorContent(problemId.value, problemSetId.value)
      if (cachedEditor) {
        code.value = cachedEditor.code
        selectedLanguage.value = cachedEditor.language
      } else {
        code.value = ''
        selectedLanguage.value = 'JavaScript'
      }

      // 重新获取题目数据
      fetchProblem()
    }
  },
  { immediate: false },
)
</script>

<style scoped>
/* 页面容器 */
.editor-page {
  width: 100%;
  height: calc(100vh - 80px);
  min-height: 600px;
  display: flex;
  flex-direction: column;
  background-color: #f7f8fa;
  margin: 0;
  padding: 0;
  overflow: hidden;
}

/* 复制成功提示 */
.copy-message {
  position: fixed;
  top: 20px;
  right: 20px;
  padding: 12px 20px;
  background-color: #f6ffed;
  border: 1px solid #b7eb8f;
  border-radius: 6px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
  z-index: 1000;
  animation: fadeIn 0.3s ease-in-out;
}

@keyframes fadeIn {
  from {
    opacity: 0;
    transform: translateY(-10px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

/* 主编辑容器 */
.editor-container {
  width: 100%;
  height: 100%;
  display: flex;
  overflow: hidden;
}

/* 左侧题目区域 */
.problem-section {
  width: 40%;
  min-width: 40%;
  max-width: 40%;
  padding: 20px;
  background-color: #ffffff;
  overflow-y: auto;
  border-right: 1px solid #e5e6eb;
  transition: all 0.3s ease;
}

/* 分隔线 */
.resizer {
  width: 4px;
  background-color: #e5e6eb;
  cursor: col-resize;
  transition: all 0.3s ease;
}

.resizer:hover {
  background-color: #165dff;
}

/* 右侧代码编辑区域 */
.code-section {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-width: 60%;
  max-width: 60%;
  transition: all 0.3s ease;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .editor-container {
    flex-direction: column;
  }

  .problem-section {
    width: 100%;
    min-width: 100%;
    max-width: 100%;
    height: 40%;
    min-height: 40%;
    max-height: 40%;
    border-right: none;
    border-bottom: 1px solid #e5e6eb;
  }

  .resizer {
    width: 100%;
    height: 4px;
    cursor: row-resize;
  }

  .code-section {
    flex: 1;
    min-width: 100%;
    max-width: 100%;
    min-height: 60%;
    max-height: 60%;
  }
}
</style>
