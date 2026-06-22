<!-- src/components/Layout/MainLayout.vue -->
<template>
  <div class="main-layout">
    <!-- 顶部导航 -->
    <Header />
    
    <!-- 主内容区 -->
    <main class="main-content" :style="{ paddingTop: headerPadding }">
      <slot />
    </main>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import Header from './Header.vue'

// 计算头部内边距（适配固定头部）
const headerPadding = computed(() => {
  return `80px`
})
</script>

<style scoped>
/* 布局容器 */
.main-layout {
  display: flex;
  flex-direction: column;
  min-height: 100vh;
}

/* 主内容区 */
.main-content {
  flex: 1;
  width: 100%;
}
</style>