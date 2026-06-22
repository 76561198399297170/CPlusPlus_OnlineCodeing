<!-- src/views/problems/CreateProblem.vue -->
<template>
  <MainLayout>
    <div class="create-problem-page container">
      <div class="page-header">
        <h1>创建算法题目</h1>
        <Button type="secondary" class="ai-btn" @click="showAIPanel = true"> 🤖 AI 辅助 </Button>
      </div>

      <div class="problem-form card">
        <!-- 基本信息 -->
        <div class="form-section">
          <h3 class="section-title">基本信息</h3>

          <div class="form-item">
            <label class="form-item__label">题目标题 <span class="required">*</span></label>
            <input
              type="text"
              class="form-item__input"
              v-model="form.title"
              placeholder="请输入题目标题"
            />
          </div>

          <div class="form-row">
            <div class="form-item">
              <label class="form-item__label">难度 <span class="required">*</span></label>
              <select class="form-item__select" v-model.number="form.difficulty">
                <option :value="1">入门</option>
                <option :value="2">基础</option>
                <option :value="3">中等</option>
                <option :value="4">进阶</option>
                <option :value="5">困难</option>
              </select>
            </div>

            <div class="form-item">
              <label class="form-item__label">时间限制（秒）</label>
              <input
                type="number"
                class="form-item__input"
                v-model="form.timeLimit"
                min="1"
                max="60"
              />
              <div class="form-item__hint">默认限制，可在语言限制中为特定语言设置不同值</div>
            </div>

            <div class="form-item">
              <label class="form-item__label">内存限制（MB）</label>
              <input
                type="number"
                class="form-item__input"
                v-model="form.memoryLimit"
                min="32"
                max="1024"
              />
              <div class="form-item__hint">默认限制，可在语言限制中为特定语言设置不同值</div>
            </div>
          </div>

          <div class="form-item">
            <label class="form-item__label">题目标签</label>
            <div class="tags-input">
              <span v-for="(tag, index) in form.tags" :key="index" class="tag">
                {{ tag }}
                <span class="tag-remove" @click="removeTag(index)">×</span>
              </span>
              <input
                type="text"
                class="tag-input"
                v-model="tagInput"
                placeholder="输入标签后按回车添加"
                @keyup.enter="addTag"
              />
            </div>
          </div>

          <!-- 语言限制 -->
          <div class="language-limits-section">
            <div class="section-header">
              <h4 class="sub-section-title">语言限制</h4>
              <Button
                type="primary"
                size="small"
                @click="addLanguageLimit"
                :disabled="allLanguagesAdded"
              >
                + 添加语言限制
              </Button>
            </div>

            <div v-if="form.languageLimits.length === 0" class="no-language-limits">
              <p>默认限制：时间限制 {{ form.timeLimit }} 秒，内存限制 {{ form.memoryLimit }} MB</p>
              <p>点击上方按钮添加特定语言的限制</p>
            </div>

            <div v-else class="language-limits-table">
              <table>
                <thead>
                  <tr>
                    <th>编程语言</th>
                    <th>时间限制（秒）</th>
                    <th>内存限制（MB）</th>
                    <th>操作</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="(limit, index) in form.languageLimits" :key="index">
                    <td>
                      <select v-model="limit.language" class="form-item__select">
                        <option
                          v-for="lang in getAvailableLanguages(index)"
                          :key="lang"
                          :value="lang"
                        >
                          {{ lang }}
                        </option>
                      </select>
                    </td>
                    <td>
                      <input
                        type="number"
                        v-model="limit.timeLimit"
                        class="form-item__input"
                        min="1"
                        max="60"
                      />
                    </td>
                    <td>
                      <input
                        type="number"
                        v-model="limit.memoryLimit"
                        class="form-item__input"
                        min="32"
                        max="1024"
                      />
                    </td>
                    <td>
                      <span class="remove-btn" @click="removeLanguageLimit(index)"> 删除 </span>
                    </td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>

        <!-- 题目内容 -->
        <div class="form-section">
          <h3 class="section-title">题目内容</h3>

          <div class="form-item">
            <label class="form-item__label">题目描述 <span class="required">*</span></label>
            <textarea
              class="form-item__textarea"
              v-model="form.description"
              placeholder="请输入题目描述"
              rows="6"
            ></textarea>
          </div>

          <div class="form-item">
            <label class="form-item__label">输入格式 <span class="required">*</span></label>
            <textarea
              class="form-item__textarea"
              v-model="form.inputFormat"
              placeholder="请描述输入格式"
              rows="4"
            ></textarea>
          </div>

          <div class="form-item">
            <label class="form-item__label">输出格式 <span class="required">*</span></label>
            <textarea
              class="form-item__textarea"
              v-model="form.outputFormat"
              placeholder="请描述输出格式"
              rows="4"
            ></textarea>
          </div>

          <div class="form-item">
            <label class="form-item__label">样例</label>
            <textarea
              class="form-item__textarea"
              v-model="form.sample"
              placeholder="样例1: ```\\is输入数据\\ie\\-\\os输出数据\\oe```  \\is和\\ie标记输入内容开始和结束  \\os和\\oe标记输出内容开始和结束  \\-标记分隔横线可自行选择是否使用"
              rows="4"
            ></textarea>
            <div class="form-item__hint">样例用于在题目详情页展示，帮助用户理解题目要求</div>
          </div>
        </div>

        <!-- 测试用例 -->
        <div class="form-section">
          <div class="section-header">
            <h3 class="section-title">测试用例</h3>
            <Button type="primary" size="small" @click="addTestCase"> + 添加用例 </Button>
          </div>

          <div v-for="(testCase, index) in form.testCases" :key="index" class="test-case-item">
            <div class="test-case-header">
              <span class="test-case-label">用例 {{ index + 1 }}</span>
              <span
                v-if="form.testCases.length > 1"
                class="test-case-remove"
                @click="removeTestCase(index)"
              >
                删除
              </span>
            </div>

            <div class="test-case-row">
              <div class="form-item">
                <label class="form-item__label">输入</label>
                <textarea
                  class="form-item__textarea"
                  v-model="testCase.input"
                  placeholder="输入数据"
                  rows="3"
                ></textarea>
              </div>

              <div class="form-item">
                <label class="form-item__label">预期输出</label>
                <textarea
                  class="form-item__textarea"
                  v-model="testCase.output"
                  placeholder="预期输出"
                  rows="3"
                ></textarea>
              </div>
            </div>
          </div>
        </div>

        <!-- 操作按钮 -->
        <div class="form-actions">
          <Button type="secondary" @click="handleCancel"> 取消 </Button>
          <Button type="primary" @click="handleSubmit"> 提交审核 </Button>
        </div>
      </div>

      <!-- 提示信息 -->
      <div
        v-if="tipMsg"
        class="tip-text"
        :class="{ success: tipType === 'success', error: tipType === 'error' }"
      >
        {{ tipMsg }}
      </div>
    </div>

    <!-- AI 辅助面板 -->
    <SideAIPanel
      :visible="showAIPanel"
      mode="create"
      :context="form"
      @close="showAIPanel = false"
      @apply="handleAIApply"
    />
  </MainLayout>
</template>

<script setup>
import { ref, computed } from 'vue'
import { useRouter } from 'vue-router'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'
import SideAIPanel from '@/components/Common/SideAIPanel.vue'
import { apiService } from '@/services/api'
import { getVisibleLanguages } from '@/config/languages'

const router = useRouter()

// AI 面板状态
const showAIPanel = ref(false)

// 支持的编程语言列表（从配置文件获取）
const supportedLanguages = computed(() => {
  return getVisibleLanguages().map((lang) => lang.value)
})

// 表单数据
const form = ref({
  title: '',
  difficulty: 1,
  tags: [],
  timeLimit: 3, // 默认时间限制为3秒
  memoryLimit: 256,
  description: '',
  inputFormat: '',
  outputFormat: '',
  sample: '',
  testCases: [{ input: '', output: '' }],
  languageLimits: [
    { language: 'C++', timeLimit: 1, memoryLimit: 256 },
    { language: 'C', timeLimit: 1, memoryLimit: 256 },
  ],
})

// 检查是否所有语言都已添加
const allLanguagesAdded = computed(() => {
  const selectedLanguages = form.value.languageLimits.map((limit) => limit.language)
  return selectedLanguages.length >= supportedLanguages.value.length
})

const tagInput = ref('')
const tipMsg = ref('')
const tipType = ref('error')

// 添加标签
const addTag = () => {
  const tag = tagInput.value.trim()
  if (tag && !form.value.tags.includes(tag)) {
    form.value.tags.push(tag)
    tagInput.value = ''
  }
}

// 移除标签
const removeTag = (index) => {
  form.value.tags.splice(index, 1)
}

// 添加测试用例
const addTestCase = () => {
  form.value.testCases.push({ input: '', output: '' })
}

// 移除测试用例
const removeTestCase = (index) => {
  form.value.testCases.splice(index, 1)
}

// 添加语言限制
const addLanguageLimit = () => {
  // 找出未选择的语言
  const selectedLanguages = form.value.languageLimits.map((limit) => limit.language)
  const availableLanguages = supportedLanguages.value.filter(
    (lang) => !selectedLanguages.includes(lang),
  )

  if (availableLanguages.length === 0) {
    return
  }

  // 默认选择Python3，如果已被选择则选择第一个可用语言
  let defaultLanguage = 'Python3'
  if (selectedLanguages.includes(defaultLanguage) && availableLanguages.length > 0) {
    defaultLanguage = availableLanguages[0]
  }

  // 为C和C++设置特殊的时间限制
  let defaultTimeLimit = 3 // 默认时间限制为3秒
  if (defaultLanguage === 'C' || defaultLanguage === 'C++') {
    defaultTimeLimit = 1 // C和C++的时间限制为1秒
  }

  form.value.languageLimits.push({
    language: defaultLanguage,
    timeLimit: defaultTimeLimit,
    memoryLimit: 256,
  })
}

// 移除语言限制
const removeLanguageLimit = (index) => {
  form.value.languageLimits.splice(index, 1)
}

// 获取可用的语言（排除已选择的）
const getAvailableLanguages = (currentIndex) => {
  const selectedLanguages = form.value.languageLimits
    .filter((_, index) => index !== currentIndex)
    .map((limit) => limit.language)
  return supportedLanguages.value.filter((lang) => !selectedLanguages.includes(lang))
}

// 验证表单
const validateForm = () => {
  if (!form.value.title.trim()) {
    tipMsg.value = '请输入题目标题'
    return false
  }
  if (!form.value.description.trim()) {
    tipMsg.value = '请输入题目描述'
    return false
  }
  if (!form.value.inputFormat.trim()) {
    tipMsg.value = '请输入输入格式'
    return false
  }
  if (!form.value.outputFormat.trim()) {
    tipMsg.value = '请输入输出格式'
    return false
  }
  for (let i = 0; i < form.value.testCases.length; i++) {
    const tc = form.value.testCases[i]
    // 只检查输出是否为空，输入可以为空
    if (!tc) {
      tipMsg.value = `第 ${i + 1} 个测试用例格式错误`
      return false
    }
    if (!tc.output) {
      tipMsg.value = `请完善第 ${i + 1} 个测试用例的输出`
      return false
    }
    if (tc.output.trim() === '') {
      tipMsg.value = `请完善第 ${i + 1} 个测试用例的输出`
      return false
    }
  }
  return true
}

// 提交
const handleSubmit = async () => {
  tipMsg.value = ''

  if (!validateForm()) {
    return
  }

  try {
    // 调用 API 提交题目
    const response = await apiService.createProblem(form.value)

    if (response.code === 0) {
      tipType.value = 'success'
      tipMsg.value = '题目提交成功，等待审核'

      setTimeout(() => {
        router.push('/problems')
      }, 1500)
    } else {
      tipType.value = 'error'
      tipMsg.value = response.msg || '提交失败，请重试'
    }
  } catch (error) {
    tipType.value = 'error'
    tipMsg.value = error.msg || '提交失败，请重试'
  }
}

// 取消
const handleCancel = () => {
  router.push('/problems')
}

// 处理 AI 辅助结果应用
const handleAIApply = (content) => {
  // 如果 content 是对象且包含完整的题目信息，直接填充表单
  if (typeof content === 'object' && content !== null) {
    // 填充基本信息
    if (content.title) form.value.title = content.title
    if (content.difficulty) form.value.difficulty = content.difficulty
    if (content.tags) form.value.tags = content.tags

    // 填充题目内容
    if (content.description) form.value.description = content.description
    if (content.inputFormat) form.value.inputFormat = content.inputFormat
    if (content.outputFormat) form.value.outputFormat = content.outputFormat

    // 填充测试用例
    if (content.testCases) {
      // 检查测试用例格式，如果是数组格式（[[input, output], ...]），转换为对象格式
      if (Array.isArray(content.testCases) && content.testCases.length > 0) {
        const firstItem = content.testCases[0]
        if (Array.isArray(firstItem) && firstItem.length === 2) {
          // 转换格式：[[input, output], ...] -> [{ input, output }, ...]
          form.value.testCases = content.testCases.map((item) => ({
            input: item[0],
            output: item[1],
          }))
        } else {
          // 已经是对象格式，直接使用
          form.value.testCases = content.testCases
        }
      }
    }
  }
}
</script>

<style scoped>
.create-problem-page {
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

.ai-btn {
  display: flex;
  align-items: center;
  gap: 6px;
}

.problem-form {
  padding: 32px;
}

.form-section {
  margin-bottom: 32px;
  padding-bottom: 32px;
  border-bottom: 1px solid #e5e6eb;

  &:last-child {
    border-bottom: none;
    margin-bottom: 0;
    padding-bottom: 0;
  }
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.section-title {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 20px;
}

.form-row {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 20px;
}

.form-item {
  margin-bottom: 20px;
}

.form-item__label {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 8px;
}

.required {
  color: #f53f3f;
}

.form-item__input,
.form-item__select {
  width: 100%;
  height: 44px;
  padding: 0 16px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  font-size: 14px;
  color: #1d2129;
  transition: all 0.2s ease;
}

.form-item__input:focus,
.form-item__select:focus {
  outline: none;
  border-color: #165dff;
  box-shadow: 0 0 0 2px rgba(22, 93, 255, 0.1);
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

.form-item__hint {
  font-size: 12px;
  color: #86909c;
  margin-top: 4px;
  line-height: 1.4;
}

/* 标签输入 */
.tags-input {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  padding: 8px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  min-height: 44px;
}

.tag {
  display: inline-flex;
  align-items: center;
  gap: 4px;
  padding: 4px 12px;
  background-color: #e8f0ff;
  color: #165dff;
  border-radius: 12px;
  font-size: 13px;
}

.tag-remove {
  cursor: pointer;
  font-size: 16px;
  line-height: 1;

  &:hover {
    color: #f53f3f;
  }
}

.tag-input {
  flex: 1;
  min-width: 120px;
  border: none;
  outline: none;
  font-size: 14px;
  padding: 4px;
}

/* 测试用例 */
.test-case-item {
  padding: 16px;
  background-color: #f7f8fa;
  border-radius: 8px;
  margin-bottom: 16px;
}

.test-case-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 12px;
}

.test-case-label {
  font-weight: 500;
  color: #1d2129;
}

.test-case-remove {
  color: #f53f3f;
  font-size: 14px;
  cursor: pointer;
}

.test-case-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 16px;
}

/* 操作按钮 */
.form-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  margin-top: 32px;
}

/* 提示信息 */
.tip-text {
  font-size: 14px;
  line-height: 1.5;
  padding: 12px 16px;
  border-radius: 8px;
  text-align: center;

  &.success {
    background-color: #e8f5e8;
    color: #00b42a;
  }

  &.error {
    background-color: #fce4ec;
    color: #f53f3f;
  }
}

.sub-section-title {
  font-size: 16px;
  font-weight: 500;
  color: #1d2129;
  margin: 0;
}

.language-limits-section {
  margin-top: 24px;
  padding-top: 20px;
  border-top: 1px solid #e5e6eb;
}

.language-limits-section .section-header {
  margin-bottom: 16px;
}

/* 语言限制 */
.no-language-limits {
  padding: 20px;
  background-color: #f7f8fa;
  border-radius: 8px;
  text-align: center;
  color: #86909c;
  font-size: 14px;
  line-height: 1.5;
}

.language-limits-table {
  width: 100%;
  overflow-x: auto;

  table {
    width: 100%;
    border-collapse: collapse;

    th,
    td {
      padding: 12px;
      text-align: left;
      border-bottom: 1px solid #e5e6eb;
    }

    th {
      background-color: #f7f8fa;
      font-weight: 500;
      color: #4e5969;
      font-size: 14px;
    }

    td {
      vertical-align: middle;

      .form-item__select,
      .form-item__input {
        width: 100%;
      }
    }
  }
}

.remove-btn {
  color: #f53f3f;
  font-size: 14px;
  cursor: pointer;

  &:hover {
    text-decoration: underline;
  }
}
</style>
