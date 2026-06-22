import { ref, nextTick } from 'vue'

interface NotificationOptions {
  title: string
  message: string
  type?: 'success' | 'error' | 'warning' | 'info'
  duration?: number
}

interface Notification {
  id: number
  title: string
  message: string
  type: 'success' | 'error' | 'warning' | 'info'
  duration: number
  visible: boolean
}

export function useNotification() {
  const notifications = ref<Notification[]>([])
  let nextId = 1

  const show = (options: NotificationOptions) => {
    const id = nextId++
    const notification: Notification = {
      id,
      title: options.title,
      message: options.message,
      type: options.type || 'info',
      duration: options.duration || 3000,
      visible: true,
    }

    notifications.value.push(notification)

    // 自动关闭
    if (notification.duration > 0) {
      setTimeout(() => {
        remove(id)
      }, notification.duration)
    }

    return id
  }

  const remove = (id: number) => {
    const index = notifications.value.findIndex((n) => n.id === id)
    if (index !== -1) {
      notifications.value[index].visible = false
      // 等待动画完成后移除
      setTimeout(() => {
        notifications.value.splice(index, 1)
      }, 300)
    }
  }

  const success = (message: string, title: string = '成功') => {
    return show({ title, message, type: 'success' })
  }

  const error = (message: string, title: string = '错误') => {
    return show({ title, message, type: 'error' })
  }

  const warning = (message: string, title: string = '警告') => {
    return show({ title, message, type: 'warning' })
  }

  const info = (message: string, title: string = '信息') => {
    return show({ title, message, type: 'info' })
  }

  return {
    notifications,
    show,
    remove,
    success,
    error,
    warning,
    info,
  }
}
