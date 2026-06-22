<!-- src/components/Common/Notification.vue -->
<template>
  <div class="notification-container">
    <div class="notification-header">
      <h3>通知</h3>
      <div class="notification-actions">
        <button class="mark-all-read" @click="markAllRead">全部已读</button>
      </div>
    </div>
    <div class="notification-list">
      <div v-if="notifications.length === 0" class="empty-notifications">
        <div class="empty-icon">📭</div>
        <div class="empty-text">没有通知</div>
      </div>
      <div v-else>
        <div 
          v-for="notification in notifications" 
          :key="notification.id" 
          class="notification-item" 
          :class="{ unread: !notification.read }"
          @click="markAsRead(notification.id)"
        >
          <div class="notification-icon">{{ notification.icon }}</div>
          <div class="notification-content">
            <div class="notification-title">{{ notification.title }}</div>
            <div class="notification-message">{{ notification.message }}</div>
            <div class="notification-time">{{ notification.time }}</div>
          </div>
          <div class="notification-status" v-if="!notification.read">
            <div class="unread-dot"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'

// 响应式数据
const notifications = ref([
  {
    id: 1,
    icon: '👥',
    title: '组队邀请',
    message: '周九邀请你加入后端技术团队',
    time: '2小时前',
    read: false
  },
  {
    id: 2,
    icon: '🏆',
    title: '比赛开始',
    message: '2026年算法挑战赛将在明天开始',
    time: '4小时前',
    read: false
  },
  {
    id: 3,
    icon: '📝',
    title: '训练计划',
    message: '你的算法入门计划已完成 65%',
    time: '1天前',
    read: true
  },
  {
    id: 4,
    icon: '💬',
    title: '讨论回复',
    message: '李四回复了你的讨论：LeetCode 100题解题思路分享',
    time: '2天前',
    read: true
  }
])

// 方法
const markAsRead = (notificationId) => {
  const notification = notifications.value.find(n => n.id === notificationId)
  if (notification) {
    notification.read = true
  }
}

const markAllRead = () => {
  notifications.value.forEach(notification => {
    notification.read = true
  })
}
</script>

<style scoped>
.notification-container {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  overflow: hidden;
}

.notification-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 20px;
  border-bottom: 1px solid #e5e6eb;
  background-color: #f7f8fa;
}

.notification-header h3 {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 0;
}

.notification-actions {
  display: flex;
  gap: 8px;
}

.mark-all-read {
  background: none;
  border: none;
  font-size: 14px;
  color: #165dff;
  cursor: pointer;
  padding: 4px 8px;
  border-radius: 4px;
  transition: background-color 0.2s ease;
}

.mark-all-read:hover {
  background-color: #e8f0ff;
}

.notification-list {
  max-height: 300px;
  overflow-y: auto;
}

.empty-notifications {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 40px;
  text-align: center;
}

.empty-icon {
  font-size: 48px;
  margin-bottom: 16px;
}

.empty-text {
  font-size: 16px;
  color: #86909c;
}

.notification-item {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 16px 20px;
  border-bottom: 1px solid #e5e6eb;
  cursor: pointer;
  transition: background-color 0.2s ease;
}

.notification-item:hover {
  background-color: #f7f8fa;
}

.notification-item.unread {
  background-color: #f0f7ff;
}

.notification-icon {
  font-size: 20px;
  margin-top: 2px;
}

.notification-content {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.notification-title {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
}

.notification-message {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.4;
}

.notification-time {
  font-size: 12px;
  color: #86909c;
}

.notification-status {
  display: flex;
  align-items: flex-start;
  padding-top: 6px;
}

.unread-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background-color: #165dff;
}
</style>