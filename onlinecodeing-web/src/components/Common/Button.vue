<!-- src/components/Common/Button.vue -->
<template>
  <button
    class="common-btn"
    :class="[
      `common-btn--${type}`,
      `common-btn--${size}`,
      { 'common-btn--disabled': disabled }
    ]"
    @click="handleClick"
    :disabled="disabled"
  >
    <slot />
  </button>
</template>

<script setup>


// 定义Props
const props = defineProps({
  // 按钮类型
  type: {
    type: String,
    default: 'primary',
    validator: (val) => ['primary', 'success', 'warning', 'danger', 'default'].includes(val)
  },
  // 按钮尺寸
  size: {
    type: String,
    default: 'medium',
    validator: (val) => ['small', 'medium', 'large'].includes(val)
  },
  // 是否禁用
  disabled: {
    type: Boolean,
    default: false
  }
})

// 定义事件
const emit = defineEmits(['click'])

// 处理点击事件
const handleClick = () => {
  if (!props.disabled) {
    emit('click')
  }
}
</script>

<style scoped>
/* 通用按钮基础样式 */
.common-btn {
  border: none;
  border-radius: 6px;
  cursor: pointer;
  font-family: "Inter", "PingFang SC", "Microsoft YaHei", sans-serif;
  font-weight: 500;
  transition: all 0.2s ease;
  display: inline-flex;
  align-items: center;
  justify-content: center;
}

/* 禁用状态 */
.common-btn--disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

/* 尺寸样式 */
.common-btn--small {
  height: 32px;
  padding: 0 12px;
  font-size: 12px;
}

.common-btn--medium {
  height: 40px;
  padding: 0 20px;
  font-size: 14px;
}

.common-btn--large {
  height: 48px;
  padding: 0 24px;
  font-size: 16px;
}

/* 类型样式 */
.common-btn--primary {
  background-color: #165DFF;
  color: #FFFFFF;
}

.common-btn--primary:not(.common-btn--disabled):hover {
  background-color: #0E42C5;
}

.common-btn--success {
  background-color: #00B42A;
  color: #FFFFFF;
}

.common-btn--success:not(.common-btn--disabled):hover {
  background-color: #00A124;
}

.common-btn--warning {
  background-color: #FF7D00;
  color: #FFFFFF;
}

.common-btn--warning:not(.common-btn--disabled):hover {
  background-color: #E57000;
}

.common-btn--danger {
  background-color: #F53F3F;
  color: #FFFFFF;
}

.common-btn--danger:not(.common-btn--disabled):hover {
  background-color: #D03434;
}

.common-btn--default {
  background-color: #FFFFFF;
  color: #1D2129;
  border: 1px solid #E5E6EB;
}

.common-btn--default:not(.common-btn--disabled):hover {
  border-color: #165DFF;
  color: #165DFF;
}
</style>