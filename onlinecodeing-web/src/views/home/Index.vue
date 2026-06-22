<!-- src/views/home/Index.vue -->
<template>
  <MainLayout>
    <div class="index-page container">
      <h1>讨论</h1>
      <!-- 内容留空 -->
    </div>
  </MainLayout>
</template>

<script setup>
import MainLayout from '@/components/Layout/MainLayout.vue'
</script>

<style scoped>
.index-page {
  padding: 40px 0;
}

.index-page h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1D2129;
  margin-bottom: 30px;
}
</style>