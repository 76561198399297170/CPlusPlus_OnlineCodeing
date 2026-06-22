<template>
  <div v-if="visible" class="notification-overlay">
    <div class="notification-box" :class="notificationClass">
      <div class="notification-icon">{{ icon }}</div>
      <div class="notification-content">
        <h3 class="notification-title">{{ title }}</h3>
        <p class="notification-message">{{ message }}</p>
      </div>
      <button class="notification-close" @click="close">×</button>
    </div>
  </div>
</template>

<script setup>
import { computed, onMounted, onUnmounted } from 'vue'

const props = defineProps({
  visible: {
    type: Boolean,
    default: false,
  },
  title: {
    type: String,
    default: '',
  },
  message: {
    type: String,
    default: '',
  },
  type: {
    type: String,
    default: 'info', // success, error, warning, info
    validator: (value) => {
      return ['success', 'error', 'warning', 'info'].includes(value)
    },
  },
  duration: {
    type: Number,
    default: 3000,
  },
})

const emit = defineEmits(['close'])

const notificationClass = computed(() => {
  return `notification-${props.type}`
})

const icon = computed(() => {
  const icons = {
    success: '✓',
    error: '✗',
    warning: '⚠',
    info: 'i',
  }
  return icons[props.type] || 'i'
})

const close = () => {
  emit('close')
}

let timer = null

onMounted(() => {
  if (props.duration > 0) {
    timer = setTimeout(() => {
      close()
    }, props.duration)
  }
})

onUnmounted(() => {
  if (timer) {
    clearTimeout(timer)
  }
})
</script>

<style scoped>
.notification-overlay {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 9999;
  animation: fadeIn 0.3s ease;
}

.notification-box {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.15);
  padding: 16px 20px;
  max-width: 320px;
  min-width: 280px;
  display: flex;
  align-items: flex-start;
  gap: 12px;
  position: relative;
  animation: slideIn 0.3s ease;
}

.notification-icon {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 16px;
  font-weight: bold;
  flex-shrink: 0;
  margin-top: 2px;
}

.notification-content {
  flex: 1;
  min-width: 0;
}

.notification-title {
  font-size: 14px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 4px 0;
}

.notification-message {
  font-size: 14px;
  color: #4e5969;
  margin: 0;
  line-height: 1.4;
  word-break: break-word;
}

.notification-close {
  position: absolute;
  top: 8px;
  right: 12px;
  width: 20px;
  height: 20px;
  border: none;
  background: none;
  font-size: 16px;
  line-height: 1;
  cursor: pointer;
  color: #86909c;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s ease;
}

.notification-close:hover {
  background-color: #f7f8fa;
  color: #1d2129;
}

/* 类型样式 */
.notification-success .notification-icon {
  background-color: #e8f5e9;
  color: #00b42a;
}

.notification-error .notification-icon {
  background-color: #fce8e6;
  color: #f53f3f;
}

.notification-warning .notification-icon {
  background-color: #fff3e0;
  color: #ff7d00;
}

.notification-info .notification-icon {
  background-color: #e3f2fd;
  color: #165dff;
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
    transform: translateX(100%);
  }
  to {
    opacity: 1;
    transform: translateX(0);
  }
}

/* 响应式 */
@media (max-width: 768px) {
  .notification-overlay {
    top: 10px;
    right: 10px;
    left: 10px;
  }

  .notification-box {
    max-width: 100%;
    min-width: auto;
  }
}
</style>
