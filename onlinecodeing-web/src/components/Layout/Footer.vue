<!-- src/components/Layout/Footer.vue -->
<template>
  <footer class="footer-container">
    <div class="container footer-content">
      <div class="copyright">
        © 2026 CodePractice 在线编程练习平台 - 版权所有
      </div>
      <div class="links">
        <a href="/about" class="footer-link">关于我们</a>
        <a href="/privacy" class="footer-link">隐私政策</a>
        <a href="/contact" class="footer-link">联系我们</a>
      </div>
    </div>
  </footer>
</template>

<script setup>
// 底部组件无业务逻辑
</script>

<style scoped>
/* 底部容器 */
.footer-container {
  height: 80px;
  background-color: #FFFFFF;
  border-top: 1px solid #E5E6EB;
  padding: 20px 0;
  margin-top: auto;
}

/* 底部内容 */
.footer-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 10px;

  .copyright {
    font-size: 12px;
    color: #4E5969;
  }

  .links {
    display: flex;
    gap: 20px;

    .footer-link {
      font-size: 12px;
      color: #4E5969;
      text-decoration: none;

      &:hover {
        color: #165DFF;
        text-decoration: underline;
      }
    }
  }
}
</style>