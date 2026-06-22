<template>
  <div class="code-editor-container">
    <!-- 代码编辑工具栏 -->
    <div class="code-toolbar">
      <!-- 语言选择 -->
      <div class="language-selector">
        <select v-model="selectedLanguage" class="language-select">
          <option v-for="lang in languages" :key="lang.value" :value="lang.value">
            {{ lang.label }}
          </option>
        </select>
      </div>

      <!-- 风格切换 -->
      <button class="style-toggle" @click="toggleEditorStyle">切换风格</button>
    </div>

    <!-- 代码编辑器 -->
    <div class="code-editor" :class="{ 'dark-mode': isDarkMode }">
      <textarea
        v-model="code"
        class="code-textarea"
        placeholder="在此输入代码..."
        spellcheck="false"
        @keydown.tab="handleTabKey"
      ></textarea>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, computed } from 'vue'
import { getVisibleLanguages } from '@/config/languages'

// Props
const props = defineProps({
  modelValue: {
    type: String,
    default: ''
  },
  language: {
    type: String,
    default: 'JavaScript'
  }
})

// Emits
const emit = defineEmits(['update:modelValue', 'update:language'])

// 响应式数据
const code = ref(props.modelValue)
const selectedLanguage = ref(props.language)
const isDarkMode = ref(false)

// 支持的编程语言（从配置文件获取）
const languages = computed(() => {
  return getVisibleLanguages().map(lang => ({
    value: lang.value,
    label: lang.name
  }))
})

// 监听代码变化
watch(code, (newValue) => {
  emit('update:modelValue', newValue)
})

// 监听语言变化
watch(selectedLanguage, (newValue) => {
  emit('update:language', newValue)
})

// 切换编辑器风格
const toggleEditorStyle = () => {
  isDarkMode.value = !isDarkMode.value
}

// 处理Tab键事件
const handleTabKey = (event) => {
  event.preventDefault()
  
  const textarea = event.target
  const start = textarea.selectionStart
  const end = textarea.selectionEnd
  
  // 在光标位置插入4个空格
  const spaces = '    '
  const newValue = code.value.substring(0, start) + spaces + code.value.substring(end)
  
  code.value = newValue
  
  // 保持光标位置在插入空格之后
  setTimeout(() => {
    textarea.selectionStart = textarea.selectionEnd = start + spaces.length
  }, 0)
}
</script>

<style scoped>
/* 代码编辑器容器 */
.code-editor-container {
  display: flex;
  flex-direction: column;
  height: 100%;
}

/* 代码工具栏 */
.code-toolbar {
  display: flex;
  align-items: center;
  gap: 15px;
  padding: 12px 20px;
  background-color: #ffffff;
  border-bottom: 1px solid #e5e6eb;
}

.language-selector {
  flex-shrink: 0;
}

.language-select {
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #1d2129;
  background-color: #ffffff;
  cursor: pointer;
}

.style-toggle {
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-size: 14px;
  color: #4e5969;
  background-color: #ffffff;
  cursor: pointer;
  transition: all 0.2s ease;
}

.style-toggle:hover {
  border-color: #165dff;
  color: #165dff;
}

/* 代码编辑器 */
.code-editor {
  flex: 1;
  padding: 20px;
  background-color: #ffffff;
  transition: all 0.3s ease;
}

.code-editor.dark-mode {
  background-color: #1d2129;
}

.code-textarea {
  width: 100%;
  height: 100%;
  border: none;
  outline: none;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 14px;
  line-height: 1.5;
  resize: none;
  background: transparent;
  color: #1d2129;
  transition: color 0.3s ease;
}

.code-editor.dark-mode .code-textarea {
  color: #ffffff;
}
</style>