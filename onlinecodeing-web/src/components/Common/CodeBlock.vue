<template>
  <div class="code-block-container">
    <!-- 头部 -->
    <div class="code-block-header">
      <div class="code-block-title"></div>
      <div class="code-block-actions" v-if="hasInput || hasOutput">
        <div class="copy-with-selector">
          <select v-model="copySelector" class="copy-selector">
            <option v-if="hasInput" value="input">输入</option>
            <option v-if="hasOutput" value="output">预期输出</option>
          </select>
          <button class="action-btn copy-btn" @click="copyCode" title="复制代码">
            <span class="icon">📋</span>
            <span class="tooltip">复制</span>
          </button>
        </div>
        <button class="action-btn apply-btn" @click="applyToTest" title="复制到测试">
          <span class="icon">➡️</span>
          <span class="tooltip">复制到测试</span>
        </button>
      </div>
    </div>

    <!-- 代码内容 -->
    <div class="code-block-content">
      <pre v-html="parsedContent"></pre>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch, onMounted } from 'vue'

// 定义Props
const props = defineProps({
  // 代码内容
  code: {
    type: String,
    required: true,
    default: '',
  },
  // 原始代码内容（用于拷贝到测试功能）
  originalCode: {
    type: String,
    default: '',
  },
  // 代码语言
  language: {
    type: String,
    default: 'code',
  },
})

// 定义事件
const emit = defineEmits(['copy', 'apply-to-test'])

// 组件初始化时输出
onMounted(() => {
})

// 计算属性
const originalCodeToUse = computed(() => {
  const value = props.originalCode || props.code
  return value
})

// 检查是否存在输入和输出标记
const hasInput = computed(() => {
  const value = /[\\]{1,2}is[\s\S]*?[\\]{1,2}ie/.test(originalCodeToUse.value)
  return value
})

const hasOutput = computed(() => {
  const value = /[\\]{1,2}os[\s\S]*?[\\]{1,2}oe/.test(originalCodeToUse.value)
  return value
})

// 响应式变量
const copySelector = ref('')

// 监听 hasInput 和 hasOutput 的变化，设置默认值
watch(
  [hasInput, hasOutput],
  (newValues) => {
    const [newHasInput, newHasOutput] = newValues
    
    if (newHasInput) {
      copySelector.value = 'input'
    } else if (newHasOutput) {
      copySelector.value = 'output'
    } else {
      copySelector.value = ''
    }
  },
  { immediate: true },
)

// 解析输入内容
const inputContent = computed(() => {
  const match = originalCodeToUse.value.match(/[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/)
  const value = match ? match[1].trim() : ''
  return value
})

// 解析输出内容
const outputContent = computed(() => {
  const match = originalCodeToUse.value.match(/[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/)
  const value = match ? match[1].trim() : ''
  return value
})

// 解析后的内容（用于显示）
const parsedContent = computed(() => {
  let content = originalCodeToUse.value
  
  // 处理输入标记
  const inputRegex = /[\\]{1,2}is([\s\S]*?)[\\]{1,2}ie/g
  content = content.replace(inputRegex, '$1')
  
  // 处理输出标记
  const outputRegex = /[\\]{1,2}os([\s\S]*?)[\\]{1,2}oe/g
  content = content.replace(outputRegex, '$1')
  
  // 处理分割线
  const dividerRegex = /[\\]{1,2}-/g
  content = content.replace(dividerRegex, '<hr class="sample-divider">')
  
  return content
})

// 复制代码
const copyCode = () => {
  let textToCopy = ''

  if (copySelector.value === 'input') {
    textToCopy = inputContent.value
  } else if (copySelector.value === 'output') {
    textToCopy = outputContent.value
  }

  // 确保换行符正确处理
  textToCopy = textToCopy.replace(/\n/g, '\r\n')
  
  navigator.clipboard.writeText(textToCopy)
  emit('copy')
}

// 应用到测试
const applyToTest = () => {
  // 解析输入和输出内容
  const input = inputContent.value
  const output = outputContent.value
  
  emit('apply-to-test', { input, output })
}
</script>

<style scoped>
/* 代码块容器 */
.code-block-container {
  width: 100%;
  margin: 15px 0;
  border-radius: 8px;
  overflow: hidden;
  background-color: #ffffff;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  border: 1px solid #e5e6eb;
}

/* 头部 */
.code-block-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 6px 12px;
  background-color: #f7f8fa;
  border-bottom: 1px solid #e5e6eb;
}

.code-block-title {
  font-size: 12px;
  color: #86909c;
  font-weight: 500;
}

/* 操作按钮 */
.code-block-actions {
  display: flex;
  gap: 8px;
  align-items: center;
}

.copy-with-selector {
  display: flex;
  align-items: center;
  gap: 4px;
  position: relative;
}

.copy-selector {
  padding: 4px 8px;
  border: 1px solid #e5e6eb;
  border-radius: 4px;
  font-size: 12px;
  color: #4e5969;
  background-color: #ffffff;
  cursor: pointer;
  min-width: 80px;
  position: relative;
  z-index: 10;
}

.action-btn {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 26px;
  height: 26px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  background-color: #ffffff;
  cursor: pointer;
  transition: all 0.2s ease;
  overflow: visible;
  z-index: 5;
}

.action-btn:hover {
  border-color: #165dff;
  background-color: rgba(22, 93, 255, 0.05);
}

.icon {
  font-size: 14px;
}

/* 提示文本 */
.tooltip {
  position: absolute;
  top: 32px;
  right: 0;
  padding: 4px 8px;
  background-color: #1d2129;
  color: #ffffff;
  font-size: 12px;
  border-radius: 4px;
  opacity: 0;
  visibility: hidden;
  transition: all 0.2s ease;
  white-space: nowrap;
  z-index: 9999;
  pointer-events: none;
}

.action-btn:hover .tooltip {
  opacity: 1;
  visibility: visible;
}

/* 代码内容 */
.code-block-content {
  padding: 16px;
  background-color: #f7f8fa;
  border: none;
  pointer-events: none;
  overflow-x: auto;
  border-radius: 0 0 8px 8px;
}

.code-block-content pre {
  margin: 0;
  padding: 10px;
  background-color: #ffffff;
  border-radius: 4px;
  border: 1px solid #e5e6eb;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 14px;
  line-height: 1.5;
  color: #4e5969;
  overflow-x: auto;
  white-space: pre-wrap;
  word-wrap: break-word;
}

/* 分割线 */
.sample-divider {
  margin: 15px 0;
  border: none;
  height: 1px;
  background-color: #e5e6eb;
  opacity: 0.6;
}
</style>