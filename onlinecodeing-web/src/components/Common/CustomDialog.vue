<!-- src/components/Common/CustomDialog.vue -->
<template>
  <div v-show="visible" class="dialog-overlay" @click="handleOverlayClick">
    <div class="dialog-container" :class="dialogClass" @click.stop>
      <!-- 关闭按钮 -->
      <button v-if="showClose" class="dialog-close" @click="handleClose">
        &times;
      </button>
      
      <!-- 标题 -->
      <div v-if="title" class="dialog-title">
        {{ title }}
      </div>
      
      <!-- 内容 -->
      <div class="dialog-content">
        <slot></slot>
      </div>
      
      <!-- 按钮区域 -->
      <div v-if="buttons.length > 0" class="dialog-buttons">
        <button 
          v-for="(button, index) in buttons" 
          :key="index"
          class="dialog-button"
          :class="button.type"
          @click="button.callback"
        >
          {{ button.text }}
        </button>
      </div>
    </div>
  </div>
</template>

<script setup>

const props = defineProps({
  // 是否可见
  visible: {
    type: Boolean,
    default: false
  },
  // 标题
  title: {
    type: String,
    default: ''
  },
  // 是否显示关闭按钮
  showClose: {
    type: Boolean,
    default: true
  },
  // 弹窗类型
  type: {
    type: String,
    default: 'default', // default, success, error, warning, info
    validator: (value) => {
      return ['default', 'success', 'error', 'warning', 'info'].includes(value)
    }
  },
  // 按钮配置
  buttons: {
    type: Array,
    default: () => []
  }
})

const emit = defineEmits(['close'])

// 计算弹窗类名
const dialogClass = {
  'dialog-default': props.type === 'default',
  'dialog-success': props.type === 'success',
  'dialog-error': props.type === 'error',
  'dialog-warning': props.type === 'warning',
  'dialog-info': props.type === 'info'
}

// 处理关闭
const handleClose = () => {
  emit('close')
}

// 处理点击遮罩层
const handleOverlayClick = () => {
  emit('close')
}
</script>

<style scoped>
/* 遮罩层 */
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
  z-index: 9999;
  animation: fadeIn 0.3s ease;
}

/* 弹窗容器 */
.dialog-container {
  background-color: #FFFFFF;
  border-radius: 12px;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
  max-width: 90%;
  width: 500px;
  max-height: 80vh;
  overflow-y: auto;
  position: relative;
  animation: slideIn 0.3s ease;
}

/* 弹窗类型样式 */
.dialog-success {
  border-top: 4px solid #00B42A;
}

.dialog-error {
  border-top: 4px solid #F53F3F;
}

.dialog-warning {
  border-top: 4px solid #FF7D00;
}

.dialog-info {
  border-top: 4px solid #165DFF;
}

/* 关闭按钮 */
.dialog-close {
  position: absolute;
  top: 12px;
  right: 12px;
  width: 32px;
  height: 32px;
  border: none;
  background: none;
  font-size: 24px;
  line-height: 1;
  cursor: pointer;
  color: #86909C;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s ease;
}

.dialog-close:hover {
  background-color: #F7F8FA;
  color: #1D2129;
}

/* 标题 */
.dialog-title {
  font-size: 18px;
  font-weight: 600;
  color: #1D2129;
  padding: 20px 24px 16px;
  border-bottom: 1px solid #E5E6EB;
}

/* 内容 */
.dialog-content {
  padding: 24px;
  color: #4E5969;
  line-height: 1.6;
}

/* 按钮区域 */
.dialog-buttons {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  padding: 0 24px 24px;
  border-top: 1px solid #E5E6EB;
  padding-top: 16px;
}

/* 按钮样式 */
.dialog-button {
  padding: 8px 20px;
  border: 1px solid #E5E6EB;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.dialog-button:hover {
  transform: translateY(-1px);
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

/* 按钮类型 */
.dialog-button.primary {
  background-color: #165DFF;
  color: #FFFFFF;
  border-color: #165DFF;
}

.dialog-button.primary:hover {
  background-color: #0E42C5;
  border-color: #0E42C5;
}

.dialog-button.secondary {
  background-color: #FFFFFF;
  color: #4E5969;
}

.dialog-button.secondary:hover {
  background-color: #F7F8FA;
}

.dialog-button.success {
  background-color: #00B42A;
  color: #FFFFFF;
  border-color: #00B42A;
}

.dialog-button.success:hover {
  background-color: #00A127;
  border-color: #00A127;
}

.dialog-button.error {
  background-color: #F53F3F;
  color: #FFFFFF;
  border-color: #F53F3F;
}

.dialog-button.error:hover {
  background-color: #E03636;
  border-color: #E03636;
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

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateY(-20px) scale(0.95);
  }
  to {
    opacity: 1;
    transform: translateY(0) scale(1);
  }
}

/* 响应式 */
@media (max-width: 768px) {
  .dialog-container {
    width: 90%;
    margin: 20px;
  }
}
</style>