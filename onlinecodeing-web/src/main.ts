// src/main.ts
import { createApp } from 'vue'
import App from './App.vue'
import router from './router/index'
import axios from 'axios'
import { decodeU8String } from '@/utils/encoding'
// 导入全局样式
import '@/assets/styles/common.scss'

// axios 配置（逻辑不变，补充 TS 类型）
axios.defaults.withCredentials = false
axios.defaults.headers.common['Content-Type'] = 'application/json;charset=utf-8'
axios.defaults.timeout = 10000
axios.defaults.baseURL = '/api'

// 响应拦截器（补充 TS 类型）
axios.interceptors.response.use(
  (response) => {
    if (response.data && typeof response.data === 'object') {
      response.data = decodeU8String(response.data)
    }
    return response
  },
  (error) => {
    const errorInfo = {
      code: -1,
      msg: '未知错误',
    } as { code: number; msg: string; response?: any }

    if (error.response) {
      errorInfo.code = error.response.status
      errorInfo.msg = error.response.data?.msg || `请求失败（${error.response.status}）`
      errorInfo.response = error.response
    } else if (error.request) {
      errorInfo.msg = '网络错误：无法连接到后端服务，请检查服务是否启动'
    } else {
      errorInfo.msg = '请求配置错误：' + error.message
    }

    return Promise.reject(errorInfo)
  },
)

// 创建应用（补充 TS 类型）
const app = createApp(App)
// 扩展全局属性的 TS 类型
declare module 'vue' {
  interface ComponentCustomProperties {
    $axios: typeof axios
  }
}
app.config.globalProperties.$axios = axios

app.use(router).mount('#app')
