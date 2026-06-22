<!-- src/components/Common/SuccessDialog.vue -->
<template>
  <div v-if="visible" class="success-overlay" @click="handleOverlayClick">
    <div class="success-container" @click.stop>
      <!-- 成功图标 -->
      <div class="success-icon">
        <svg
          width="60"
          height="60"
          viewBox="0 0 60 60"
          fill="none"
          xmlns="http://www.w3.org/2000/svg"
        >
          <circle cx="30" cy="30" r="30" fill="#00B42A" />
          <path
            d="M18 30L26 38L42 22"
            stroke="white"
            stroke-width="4"
            stroke-linecap="round"
            stroke-linejoin="round"
          />
        </svg>
      </div>

      <!-- 标题 -->
      <h2 class="success-title">{{ title }}</h2>

      <!-- 内容 -->
      <p class="success-message">{{ message }}</p>

      <!-- 按钮 -->
      <button class="success-button" @click="handleClose">
        {{ buttonText }}
      </button>
    </div>

    <!-- 彩带容器 -->
    <div class="confetti-container" ref="confettiContainer"></div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue'

const props = defineProps({
  // 是否可见
  visible: {
    type: Boolean,
    default: false,
  },
  // 标题
  title: {
    type: String,
    default: '成功！',
  },
  // 消息
  message: {
    type: String,
    default: '代码运行通过！',
  },
  // 按钮文本
  buttonText: {
    type: String,
    default: '确定',
  },
  // 彩带数量
  confettiCount: {
    type: Number,
    default: 100,
  },
  // 彩带类型
  confettiType: {
    type: String,
    default: 'mixed', // mixed, circles, squares, stars
    validator: (value) => {
      return ['mixed', 'circles', 'squares', 'stars'].includes(value)
    },
  },
  // 音效路径
  soundPath: {
    type: String,
    default: '',
  },
})

const emit = defineEmits(['close'])

const confettiContainer = ref(null)
const confettiPieces = ref([])
const audio = ref(null)

// 处理关闭
const handleClose = () => {
  cleanupConfetti()
  emit('close')
}

// 处理点击遮罩层
const handleOverlayClick = () => {
  cleanupConfetti()
  emit('close')
}

// 播放音效
const playSound = () => {
  if (props.soundPath) {
    audio.value = new Audio(props.soundPath)
    audio.value.play().catch(() => {
      // 无法播放音效
    })
  }
}

// 创建彩带
const createConfetti = () => {
  if (!confettiContainer.value) return

  const container = confettiContainer.value
  const containerRect = container.getBoundingClientRect()

  for (let i = 0; i < props.confettiCount; i++) {
    const confetti = document.createElement('div')

    // 设置彩带样式
    confetti.style.position = 'absolute'
    confetti.style.width = `${Math.random() * 10 + 5}px`
    confetti.style.height = `${Math.random() * 10 + 5}px`
    confetti.style.left = `${Math.random() * containerRect.width}px`
    confetti.style.top = `${containerRect.height}px`
    confetti.style.opacity = '1'
    confetti.style.transition = `all ${Math.random() * 3 + 2}s ease-in-out`

    // 设置颜色
    const colors = ['#165DFF', '#00B42A', '#FF7D00', '#F53F3F', '#722ED1', '#13C2C2']
    confetti.style.backgroundColor = colors[Math.floor(Math.random() * colors.length)]

    // 设置形状
    switch (props.confettiType) {
      case 'circles':
        confetti.style.borderRadius = '50%'
        break
      case 'squares':
        confetti.style.borderRadius = '0'
        break
      case 'stars':
        confetti.style.clipPath =
          'polygon(50% 0%, 61% 35%, 98% 35%, 68% 57%, 79% 91%, 50% 70%, 21% 91%, 32% 57%, 2% 35%, 39% 35%)'
        break
      default: // mixed
        if (Math.random() > 0.66) {
          confetti.style.borderRadius = '50%'
        } else if (Math.random() > 0.5) {
          confetti.style.clipPath =
            'polygon(50% 0%, 61% 35%, 98% 35%, 68% 57%, 79% 91%, 50% 70%, 21% 91%, 32% 57%, 2% 35%, 39% 35%)'
        }
    }

    // 添加到容器
    container.appendChild(confetti)
    confettiPieces.value.push(confetti)

    // 启动动画
    setTimeout(() => {
      confetti.style.transform = `translateY(${-containerRect.height - 50}px) rotate(${Math.random() * 360}deg)`
      confetti.style.opacity = '0'
    }, 10)
  }
}

// 清理彩带
const cleanupConfetti = () => {
  confettiPieces.value.forEach((confetti) => {
    if (confetti.parentNode) {
      confetti.parentNode.removeChild(confetti)
    }
  })
  confettiPieces.value = []

  // 停止音效
  if (audio.value) {
    audio.value.pause()
    audio.value = null
  }
}

// 监听可见性变化
watch(
  () => props.visible,
  (newValue) => {
    if (newValue) {
      setTimeout(() => {
        playSound()
        createConfetti()
      }, 100)
    } else {
      cleanupConfetti()
    }
  },
)

// 组件挂载时检查可见性
onMounted(() => {
  if (props.visible) {
    setTimeout(() => {
      playSound()
      createConfetti()
    }, 100)
  }
})

// 组件卸载时清理
onUnmounted(() => {
  cleanupConfetti()
})
</script>

<style scoped>
/* 遮罩层 */
.success-overlay {
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

/* 成功容器 */
.success-container {
  background-color: #ffffff;
  border-radius: 16px;
  box-shadow: 0 15px 35px rgba(0, 0, 0, 0.2);
  padding: 40px;
  text-align: center;
  max-width: 90%;
  width: 400px;
  position: relative;
  animation: slideIn 0.4s ease;
  z-index: 10;
}

/* 成功图标 */
.success-icon {
  margin-bottom: 24px;
  animation: pulse 1s ease-in-out;
}

/* 标题 */
.success-title {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin: 0 0 16px 0;
  animation: fadeInUp 0.5s ease;
}

/* 消息 */
.success-message {
  font-size: 16px;
  color: #4e5969;
  margin: 0 0 32px 0;
  line-height: 1.5;
  animation: fadeInUp 0.6s ease;
}

/* 按钮 */
.success-button {
  padding: 12px 32px;
  background-color: #00b42a;
  color: #ffffff;
  border: none;
  border-radius: 8px;
  font-size: 16px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s ease;
  animation: fadeInUp 0.7s ease;
}

.success-button:hover {
  background-color: #00a127;
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(0, 180, 42, 0.3);
}

/* 彩带容器 */
.confetti-container {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  overflow: hidden;
  pointer-events: none;
  z-index: 1;
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
    transform: translateY(-30px) scale(0.9);
  }
  to {
    opacity: 1;
    transform: translateY(0) scale(1);
  }
}

@keyframes pulse {
  0% {
    transform: scale(0.8);
    opacity: 0;
  }
  50% {
    transform: scale(1.1);
  }
  100% {
    transform: scale(1);
    opacity: 1;
  }
}

@keyframes fadeInUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

/* 响应式 */
@media (max-width: 768px) {
  .success-container {
    width: 90%;
    padding: 30px;
  }

  .success-title {
    font-size: 20px;
  }

  .success-message {
    font-size: 14px;
  }
}
</style>
