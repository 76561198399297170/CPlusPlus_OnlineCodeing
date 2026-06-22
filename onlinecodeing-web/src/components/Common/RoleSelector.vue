<!-- src/components/Common/RoleSelector.vue -->
<template>
  <div class="role-selector">
    <div class="role-label">用户角色：</div>
    <div class="role-buttons">
      <button 
        v-for="role in roleList" 
        :key="role.value"
        class="role-btn"
        :class="{ active: selectedRole === role.value }"
        @click="handleRoleChange(role.value)"
      >
        {{ role.label }}
      </button>
    </div>
  </div>
</template>

<script setup>
import { ref, watch } from 'vue'

// 定义Props
const props = defineProps({
  // 当前选中的角色
  modelValue: {
    type: String,
    required: true,
    default: 'ADMIN'
  },
  // 角色列表（可自定义）
  roleList: {
    type: Array,
    default: () => [
      { label: '管理员', value: 'ADMIN' },
      { label: '学生', value: 'STUDENT' },
      { label: '教师', value: 'TEACHER' }
    ]
  }
})

// 定义事件
const emit = defineEmits(['update:modelValue'])

// 响应式变量
const selectedRole = ref(props.modelValue)

// 监听modelValue变化
watch(() => props.modelValue, (newValue) => {
  selectedRole.value = newValue
})

// 处理角色切换
const handleRoleChange = (value) => {
  selectedRole.value = value
  emit('update:modelValue', value)
}
</script>

<style scoped>
/* 角色选择容器 */
.role-selector {
  display: flex;
  flex-direction: column;
  gap: 10px;
  margin-bottom: 20px;

  .role-label {
    font-weight: 500;
    color: #1D2129;
    font-size: 14px;
  }

  .role-buttons {
    display: flex;
    gap: 12px;
    flex-wrap: wrap;
  }

  .role-btn {
    padding: 8px 20px;
    border: 1px solid #E5E6EB;
    border-radius: 6px;
    background-color: #FFFFFF;
    color: #1D2129;
    font-size: 14px;
    cursor: pointer;
    transition: all 0.2s ease;

    &:hover {
      border-color: #4080FF;
      color: #4080FF;
    }

    &.active {
      background-color: #165DFF;
      color: #FFFFFF;
      border-color: #165DFF;

      &:hover {
        background-color: #0E42C5;
        border-color: #0E42C5;
      }
    }
  }
}
</style>