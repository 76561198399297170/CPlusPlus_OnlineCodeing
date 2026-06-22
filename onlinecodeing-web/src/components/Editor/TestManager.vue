<template>
  <div class="test-manager">
    <!-- 测试标签栏 -->
    <div class="test-tabs">
      <div
        v-for="(test, index) in localTests"
        :key="index"
        class="test-tab"
        :class="{ active: localActiveTestTab === index }"
        @click="localActiveTestTab = index"
      >
        <span class="tab-title">{{ test.name || `测试 ${index + 1}` }}</span>
        <button class="tab-close" @click.stop="removeTest(index)">&times;</button>
      </div>
      <button class="add-test-tab" @click="addTest">+</button>
    </div>

    <!-- 测试内容 -->
    <div class="test-content">
      <div v-if="localActiveTestTab !== null && localTests[localActiveTestTab]" class="test-panel">
        <!-- 测试输入 - 仅在非提交结果选项卡显示 -->
        <div v-if="localTests[localActiveTestTab].name !== '提交结果'" class="test-input-section">
          <label class="test-label">输入：</label>
          <textarea
            v-model="localTests[localActiveTestTab].input"
            class="test-textarea"
            placeholder="输入测试数据..."
          ></textarea>
        </div>

        <!-- 测试输出 - 仅在非提交结果选项卡显示 -->
        <div v-if="localTests[localActiveTestTab].name !== '提交结果'" class="test-output-section">
          <label class="test-label">预期输出：</label>
          <textarea
            v-model="localTests[localActiveTestTab].output"
            class="test-textarea"
            placeholder="输入预期输出..."
          ></textarea>
        </div>

        <!-- 测试结果 -->
        <div v-if="localTests[localActiveTestTab].result" class="test-result">
          <label class="test-label">实际输出：</label>
          <div class="result-content" :class="localTests[localActiveTestTab].result.status">
            {{ localTests[localActiveTestTab].result.output }}
          </div>
          <!-- 错误信息 -->
          <div v-if="localTests[localActiveTestTab].result.error" class="test-error">
            <label class="test-label">错误信息：</label>
            <div class="error-content">
              {{ localTests[localActiveTestTab].result.error }}
            </div>
          </div>
        </div>
      </div>
      <div v-else class="empty-test">
        <p>请添加测试用例</p>
      </div>
    </div>

    <!-- 测试控制按钮 -->
    <div class="test-controls">
      <button class="test-button" @click="runTest" :disabled="localActiveTestTab === null || (localActiveTestTab !== null && localTests[localActiveTestTab] && localTests[localActiveTestTab].name === '提交结果')">提交测试</button>
      <button class="submit-button" @click="submitCode">提交代码</button>
    </div>
  </div>
</template>

<script setup>
import { ref, watch } from 'vue'

// Props
const props = defineProps({
  tests: {
    type: Array,
    default: () => []
  },
  activeTestTab: {
    type: Number,
    default: null
  }
})

// Emits
const emit = defineEmits(['update:tests', 'update:activeTestTab', 'run-test', 'submit-code'])

// 响应式数据
const localTests = ref([...props.tests])
const localActiveTestTab = ref(props.activeTestTab)

// 监听外部数据变化
watch(() => props.tests, (newValue) => {
  localTests.value = [...newValue]
}, { deep: true })

watch(() => props.activeTestTab, (newValue) => {
  localActiveTestTab.value = newValue
})

// 监听本地数据变化
watch(localTests, (newValue) => {
  emit('update:tests', [...newValue])
}, { deep: true })

watch(localActiveTestTab, (newValue) => {
  emit('update:activeTestTab', newValue)
})

// 测试相关方法
const addTest = () => {
  localTests.value.push({
    name: `测试 ${localTests.value.length + 1}`,
    input: '',
    output: '',
    result: null,
  })
  localActiveTestTab.value = localTests.value.length - 1
}

const removeTest = (index) => {
  localTests.value.splice(index, 1)
  if (localActiveTestTab.value >= localTests.value.length) {
    localActiveTestTab.value = localTests.value.length - 1
  }
}

// 运行测试
const runTest = () => {
  if (localActiveTestTab.value === null) return
  emit('run-test', localActiveTestTab.value)
}

// 提交代码
const submitCode = () => {
  emit('submit-code')
}
</script>

<style scoped>
/* 测试区域 */
.test-manager {
  border-top: 1px solid #e5e6eb;
  background-color: #ffffff;
  max-height: 300px;
  display: flex;
  flex-direction: column;
}

/* 测试标签栏 */
.test-tabs {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 20px;
  border-bottom: 1px solid #e5e6eb;
  background-color: #f7f8fa;
  overflow-x: auto;
  white-space: nowrap;
  scrollbar-width: thin;
  scrollbar-color: #c9cdd4 #f7f8fa;
}

.test-tabs::-webkit-scrollbar {
  height: 6px;
}

.test-tabs::-webkit-scrollbar-track {
  background: #f7f8fa;
  border-radius: 3px;
}

.test-tabs::-webkit-scrollbar-thumb {
  background: #c9cdd4;
  border-radius: 3px;
}

.test-tabs::-webkit-scrollbar-thumb:hover {
  background: #86909c;
}

.test-tab {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 12px;
  border: 1px solid #e5e6eb;
  border-radius: 16px;
  background-color: #ffffff;
  font-size: 14px;
  color: #4e5969;
  cursor: pointer;
  transition: all 0.2s ease;
  white-space: nowrap;
}

.test-tab:hover {
  border-color: #165dff;
  color: #165dff;
}

.test-tab.active {
  border-color: #165dff;
  background-color: #165dff;
  color: #ffffff;
}

.tab-close {
  width: 16px;
  height: 16px;
  border: none;
  background: none;
  font-size: 16px;
  line-height: 1;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
  transition: all 0.2s ease;
}

.test-tab:hover .tab-close {
  background-color: rgba(0, 0, 0, 0.1);
}

.test-tab.active .tab-close {
  color: #ffffff;
  background-color: rgba(255, 255, 255, 0.2);
}

.add-test-tab {
  width: 32px;
  height: 32px;
  border: 1px dashed #e5e6eb;
  border-radius: 16px;
  background-color: #ffffff;
  font-size: 18px;
  color: #86909c;
  cursor: pointer;
  transition: all 0.2s ease;
  flex-shrink: 0;
}

.add-test-tab:hover {
  border-color: #165dff;
  color: #165dff;
  background-color: rgba(22, 93, 255, 0.05);
}

/* 测试内容 */
.test-content {
  flex: 1;
  padding: 20px;
  overflow-y: auto;
}

.test-panel {
  display: flex;
  flex-direction: column;
  gap: 15px;
}

.test-input-section,
.test-output-section {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.test-label {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
}

.test-textarea {
  padding: 12px;
  border: 1px solid #e5e6eb;
  border-radius: 6px;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 14px;
  line-height: 1.5;
  resize: vertical;
  min-height: 100px;
}

.test-result {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-top: 10px;
}

.result-content {
  padding: 12px;
  border-radius: 6px;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 14px;
  line-height: 1.5;
  white-space: pre-wrap;
}

.result-content.AC {
  background-color: rgba(0, 180, 42, 0.1);
  border: 1px solid #00b42a;
  color: #00b42a;
}

.result-content.WA,
.result-content.CE,
.result-content.TLE,
.result-content.ERROR {
  background-color: rgba(245, 63, 63, 0.1);
  border: 1px solid #f53f3f;
  color: #f53f3f;
}

.test-error {
  margin-top: 10px;
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.error-content {
  padding: 12px;
  border-radius: 6px;
  font-family: 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
  font-size: 14px;
  line-height: 1.5;
  white-space: pre-wrap;
  background-color: rgba(245, 63, 63, 0.1);
  border: 1px solid #f53f3f;
  color: #f53f3f;
}

.empty-test {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: #86909c;
}

/* 测试控制按钮 */
.test-controls {
  display: flex;
  gap: 10px;
  padding: 15px 20px;
  border-top: 1px solid #e5e6eb;
  background-color: #f7f8fa;
}

.test-button,
.submit-button {
  padding: 8px 20px;
  border: none;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.test-button {
  background-color: #ffffff;
  border: 1px solid #165dff;
  color: #165dff;
}

.test-button:hover:not(:disabled) {
  background-color: rgba(22, 93, 255, 0.05);
}

.test-button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.submit-button {
  background-color: #165dff;
  color: #ffffff;
}

.submit-button:hover {
  background-color: #0e42c5;
}
</style>