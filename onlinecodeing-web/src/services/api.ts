// src/services/api.ts
import axios from 'axios'
import type { AxiosInstance, AxiosRequestConfig, AxiosResponse } from 'axios'
import { decodeU8String } from '@/utils/encoding'

// API响应类型
export interface ApiResponse<T = any> {
  code: number
  data: T
  msg: string
}

// 用户信息类型
export interface UserInfo {
  id: number
  account: string
  nickname: string
  role: string
  token: string
}

// 题目类型
export interface Problem {
  id: number
  title: string
  description: string
  inputFormat: string
  outputFormat: string
  sample: string
  difficulty: number
  tags: string[]
  timeLimit: number
  memoryLimit: number
  completed: number // 0: 未开始, 1: 提交未通过, 2: 提交通过
  languageLimits: {
    [language: string]: {
      timeLimit: number
      memoryLimit: number
    }
  }
}

// 提交结果类型
export interface SubmissionResult {
  id: number
  question_id: number
  userId: number
  code: string
  language: string
  status: string
  score: number
  executionTime: number
  memoryUsed: number
  errorMessage: string
  createdAt: string
}

// 测试用例类型
export interface TestCase {
  input: string
  output: string
}

// 分页响应类型
export interface PaginatedResponse<T> {
  list: T[]
  total: number
  page: number
  pageSize: number
}

// 题目组（训练计划）类型
export interface ProblemSet {
  id: number
  name: string
  description: string
  totalProblems: number
}

// 训练概览类型
export interface TrainingOverview {
  totalProblemSets: number
  completedProblemSets: number
  totalProblems: number
}

// 通用响应处理函数
const processResponse = <T>(response: AxiosResponse<ApiResponse<T>>): ApiResponse<T> => {
  return response.data
}

// API服务类
class ApiService {
  private axios: AxiosInstance

  constructor() {
    this.axios = axios.create({
      baseURL: '',
      headers: {
        'Content-Type': 'application/json; charset=utf-8',
      },
      responseType: 'json',
    })

    this.setupInterceptors()
  }

  // 设置拦截器
  private setupInterceptors(): void {
    // 请求拦截器
    this.axios.interceptors.request.use(
      (config) => {
        const token = this.getToken()
        config.headers.Authorization = `Bearer ${token}`
        // 确保所有请求都使用 UTF-8 编码
        config.headers['Content-Type'] = 'application/json; charset=utf-8'
        return config
      },
      (error) => {
        return Promise.reject(error)
      },
    )

    // 响应拦截器
    this.axios.interceptors.response.use(
      (response) => {
        // 处理字节数组转换为字符串
        if (response.data) {
          response.data = this.processByteArray(response.data)
        }
        return response
      },
      (error) => {
        return Promise.reject(error)
      },
    )
  }

  // 获取token
  private getToken(): string {
    let token = localStorage.getItem('token')
    if (!token) {
      token = 'mock-token'
      localStorage.setItem('token', token)
    }
    return token
  }

  // 处理字节数组
  private processByteArray(data: any): any {
    return decodeU8String(data)
  }

  // 登录
  async login(account: string, password: string, role: string): Promise<ApiResponse<UserInfo>> {
    const response = await this.axios.post<ApiResponse<UserInfo>>('/api/login', {
      account,
      password,
      role: role.toLowerCase(),
    })
    return processResponse(response)
  }

  // 注册
  async register(
    account: string,
    password: string,
    nickname: string,
    role: string,
  ): Promise<ApiResponse<UserInfo>> {
    const response = await this.axios.post<ApiResponse<UserInfo>>('/api/register', {
      account,
      password,
      nickname,
      role: role.toLowerCase(),
    })
    return processResponse(response)
  }

  // 获取题目列表
  async getProblems(
    page: number = 1,
    pageSize: number = 10,
  ): Promise<ApiResponse<PaginatedResponse<Problem>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<Problem>>>(
      '/api/problems',
      {
        params: { page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 获取题目详情
  async getProblem(
    id: number | string,
    problemSetId?: number | string,
  ): Promise<ApiResponse<Problem>> {
    const params: any = {}
    if (problemSetId) {
      params.problem_set_id = problemSetId
    }
    const response = await this.axios.get<ApiResponse<Problem>>(`/api/problems/${id}`, { params })
    return processResponse(response)
  }

  // 提交代码（带重试机制）
  async submitCode(
    problemId: number | string,
    code: string,
    language: string,
    problemSetId?: string | number,
    retries: number = 3,
  ): Promise<ApiResponse<SubmissionResult>> {
    const data: any = {
      question_id: problemId,
      code,
      language,
    }
    if (problemSetId) {
      data.problem_set_id = problemSetId
    }
    console.log('API submitCode request:', data)

    let lastResponse
    for (let attempt = 1; attempt <= retries; attempt++) {
      const response = await this.axios.post<ApiResponse<SubmissionResult>>(
        '/api/submissions',
        data,
      )
      const processedResponse = processResponse(response)
      lastResponse = processedResponse

      if (
        processedResponse.code === 0 ||
        (processedResponse.msg !== 'retry' && !processedResponse.msg.includes('重试'))
      ) {
        return processedResponse
      }

      if (attempt < retries) {
        console.log(`submitCode attempt ${attempt} failed with 'retry', retrying...`)
        await new Promise((resolve) => setTimeout(resolve, 1000))
      }
    }

    return lastResponse
  }

  // 运行测试（带重试机制）
  async runTest(
    problemId: number | string,
    code: string,
    language: string,
    testCase: TestCase,
    problemSetId?: string | number,
    retries: number = 3,
  ): Promise<
    ApiResponse<{
      output: string
      status: string
      executionTime: number
      memoryUsed: number
    }>
  > {
    const data: any = {
      question_id: problemId,
      code,
      language,
      testCase,
    }
    if (problemSetId) {
      data.problem_set_id = problemSetId
    }

    let lastResponse
    for (let attempt = 1; attempt <= retries; attempt++) {
      const response = await this.axios.post<ApiResponse<any>>('/api/run-test', data)
      const processedResponse = processResponse(response)
      lastResponse = processedResponse

      if (
        processedResponse.code === 0 ||
        (processedResponse.msg !== 'retry' && !processedResponse.msg.includes('重试'))
      ) {
        return processedResponse
      }

      if (attempt < retries) {
        console.log(`runTest attempt ${attempt} failed with 'retry', retrying...`)
        await new Promise((resolve) => setTimeout(resolve, 1000))
      }
    }

    return lastResponse
  }

  // 获取用户提交记录
  async getUserSubmissions(
    page: number = 1,
    pageSize: number = 10,
    userId?: number,
  ): Promise<ApiResponse<PaginatedResponse<SubmissionResult>>> {
    const params: any = { page, pageSize }
    if (userId !== undefined) {
      params.userId = userId
    }
    const response = await this.axios.get<ApiResponse<PaginatedResponse<SubmissionResult>>>(
      '/api/submissions',
      {
        params,
      },
    )
    return processResponse(response)
  }

  // 获取题目组提交统计
  async getSubmissionStats(problemSetId: number): Promise<
    ApiResponse<
      Array<{
        userId: number
        question_id: number
        language: string
        status: number
        score: number
        executionTime: number
        memoryUsed: number
        submittedAt: string
      }>
    >
  > {
    const response = await this.axios.get<ApiResponse<any>>(
      `/api/submissions/stats/${problemSetId}`,
    )

    // 适配后端返回的格式 { list: [json字符串, ...], total: N }
    const result = processResponse(response)
    if (result.code === 0 && result.data && Array.isArray(result.data.list)) {
      result.data = result.data.list.map((jsonStr: string) => {
        const obj = JSON.parse(jsonStr)
        // 转换状态字符串为数字
        const statusMap: Record<string, number> = {
          NOT_DEPLOYED: -1,
          ND: -1,
          COMPILE_ERROR: 0,
          CE: 0,
          RUNTIME_ERROR: 1,
          RE: 1,
          TIME_LIMIT_EXCEEDED: 2,
          TLE: 2,
          ACCEPTED: 3,
          AC: 3,
          WRONG_ANSWER: 4,
          WA: 4,
          UNKNOW: -1,
        }
        return {
          userId: obj.userId,
          question_id: obj.question_id,
          language: obj.language,
          status: statusMap[obj.status] ?? obj.status,
          score: obj.score,
          executionTime: obj.executionTime,
          memoryUsed: obj.memoryUsed,
          submittedAt: obj.createdAt,
        }
      })
    }
    return result
  }

  // 获取用户信息
  async getUserInfo(): Promise<ApiResponse<UserInfo>> {
    const response = await this.axios.get<ApiResponse<UserInfo>>('/api/user/info')
    return processResponse(response)
  }

  // 修改昵称
  async updateNickname(nickname: string): Promise<ApiResponse<any>> {
    const response = await this.axios.put<ApiResponse<any>>('/api/user/nickname', {
      nickname,
    })
    return processResponse(response)
  }

  // 修改密码
  async changePassword(
    oldPassword: string,
    newPassword: string,
    userId?: number,
  ): Promise<ApiResponse<any>> {
    const response = await this.axios.put<ApiResponse<any>>('/api/user/password', {
      oldPassword,
      newPassword,
      userId,
    })
    return processResponse(response)
  }

  // 设置邮箱
  async setEmail(email: string): Promise<ApiResponse<any>> {
    const response = await this.axios.put<ApiResponse<any>>('/api/user/email', {
      email,
    })
    return processResponse(response)
  }

  // 获取隐私设置
  async getPrivacySettings(): Promise<
    ApiResponse<{
      showRole: boolean
      showSchool: boolean
      showGrade: boolean
      showSubmissionsCount: boolean
      showSolvedCount: boolean
      showSubmissionStats: boolean
      showUserRoleNo: boolean
      showRealName: boolean
    }>
  > {
    const response = await this.axios.get<ApiResponse<any>>('/api/user/privacy')
    return processResponse(response)
  }

  // 更新隐私设置
  async updatePrivacySettings(settings: {
    showRole: boolean
    showSchool: boolean
    showGrade: boolean
    showSubmissionsCount: boolean
    showSolvedCount: boolean
    showSubmissionStats: boolean
    showUserRoleNo: boolean
    showRealName: boolean
  }): Promise<ApiResponse<any>> {
    const response = await this.axios.put<ApiResponse<any>>('/api/user/privacy', settings)
    return processResponse(response)
  }

  // 获取其他用户信息
  async getUserProfile(userId: number | string): Promise<
    ApiResponse<{
      id: number
      account: string
      nickname: string
      role: string
      school?: string
      grade?: number
      realName?: string
      userRoleNo?: string
      userDesc?: string
      submissionsCount: number
      solvedCount: number
      submissionStats: {
        language: string
        count: number
      }[]
    }>
  > {
    const response = await this.axios.get<ApiResponse<any>>(`/api/user/profile/${userId}`)
    return processResponse(response)
  }

  // 获取用户列表（仅管理员）
  async getUsers(): Promise<
    ApiResponse<
      Array<{
        id: number
        account: string
        nickname: string
        role: string
      }>
    >
  > {
    const response = await this.axios.get<ApiResponse<any>>('/api/users')
    return processResponse(response)
  }

  // AI 辅助功能
  async getAIHelp(params: {
    mode: string
    functionId: string
    context: string
    question: any
    toolId?: string
  }): Promise<ApiResponse<{ content: string }>> {
    // 为每个工具生成专属的工具ID，方便后端进行分析工作需求
    const toolId = params.toolId || `${params.mode}-${params.functionId}`

    // 根据模式设置不同的超时时间
    // create模式：60秒，review模式：120秒（审核需要更长时间）
    const timeout = params.mode === 'review' ? 120000 : 60000

    const response = await this.axios.post<ApiResponse<{ content: string }>>(
      '/api/ai/help',
      {
        mode: params.mode,
        functionId: params.functionId,
        context: params.context,
        question: params.question,
        toolId: toolId,
      },
      {
        timeout: timeout,
      },
    )
    return processResponse(response)
  }

  // 获取待审核题目列表
  async getPendingProblems(
    page: number = 1,
    pageSize: number = 10,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>(
      '/api/problems/review',
      {
        params: { page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 审核通过题目
  async approveProblem(id: number): Promise<ApiResponse<any>> {
    const response = await this.axios.post<ApiResponse<any>>(`/api/problems/review/${id}/approve`)
    return processResponse(response)
  }

  // 审核拒绝题目
  async rejectProblem(id: number): Promise<ApiResponse<any>> {
    const response = await this.axios.post<ApiResponse<any>>(`/api/problems/review/${id}/reject`)
    return processResponse(response)
  }

  // 根据状态获取题目列表
  async getProblemsByStatus(
    status: string,
    page: number = 1,
    pageSize: number = 10,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>(
      '/api/problems/all',
      {
        params: { status, page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 隐藏题目
  async hideProblem(id: number): Promise<ApiResponse<any>> {
    const response = await this.axios.post<ApiResponse<any>>(`/api/problems/${id}/hide`)
    return processResponse(response)
  }

  // 获取待审核题目列表
  async getPendingProblemsList(
    page: number = 1,
    pageSize: number = 100,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>(
      '/api/problems/review',
      {
        params: { page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 获取已通过的题目列表
  async getApprovedProblemsList(
    page: number = 1,
    pageSize: number = 100,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>('/api/problems', {
      params: { page, pageSize },
    })
    return processResponse(response)
  }

  // 获取已驳回的题目列表
  async getRejectedProblemsList(
    page: number = 1,
    pageSize: number = 100,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>(
      '/api/problems/rejected',
      {
        params: { page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 获取隐藏的题目列表
  async getHiddenProblemsList(
    page: number = 1,
    pageSize: number = 100,
  ): Promise<ApiResponse<PaginatedResponse<any>>> {
    const response = await this.axios.get<ApiResponse<PaginatedResponse<any>>>(
      '/api/problems/hidden',
      {
        params: { page, pageSize },
      },
    )
    return processResponse(response)
  }

  // 获取平台统计数据
  async getStats(): Promise<
    ApiResponse<{
      totalProblems: number
      supportedLanguages: number
      totalSubmissions: number
      judgeAccuracy: number
    }>
  > {
    const response = await this.axios.get<ApiResponse<any>>('/api/stats')
    return processResponse(response)
  }

  // 创建题目
  async createProblem(problemData: {
    title: string
    difficulty: number
    tags: string[]
    timeLimit: number
    memoryLimit: number
    description: string
    inputFormat: string
    outputFormat: string
    sample: string
    testCases: { input: string; output: string }[]
    languageLimits: { language: string; timeLimit: number; memoryLimit: number }[]
  }): Promise<ApiResponse<any>> {
    const response = await this.axios.post<ApiResponse<any>>('/api/problems/create', problemData, {
      headers: {
        'Content-Type': 'application/json; charset=utf-8',
      },
    })
    return processResponse(response)
  }

  // 搜索题目（统一搜索）
  async searchProblems(params: {
    keyword?: string
    tags?: string[]
    difficulty?: number
    page?: number
    pageSize?: number
    includeHidden?: boolean
  }): Promise<ApiResponse<PaginatedResponse<Problem>>> {
    // 设置默认值：默认不搜索隐藏题目
    const requestParams = {
      includeHidden: false,
      ...params,
    }
    const response = await this.axios.post<ApiResponse<PaginatedResponse<Problem>>>(
      '/api/problems/search',
      requestParams,
    )
    return processResponse(response)
  }

  // 训练相关接口

  // 获取训练概览
  async getTrainingOverview(): Promise<ApiResponse<TrainingOverview>> {
    const response = await this.axios.get<ApiResponse<TrainingOverview>>('/api/training/overview')
    return processResponse(response)
  }

  // 获取进行中的训练计划（题目组）
  async getActiveProblemSets(isContest?: number): Promise<ApiResponse<Array<ProblemSet>>> {
    const response = await this.axios.get<ApiResponse<Array<ProblemSet>>>(
      '/api/training/active-sets',
      { params: { isContest } },
    )
    return processResponse(response)
  }

  // 获取推荐训练计划（题目组）
  async getRecommendedProblemSets(
    page: number = 1,
    pageSize: number = 10,
    isContest?: number,
  ): Promise<
    ApiResponse<
      PaginatedResponse<{
        id: number
        name: string
        description: string
        totalProblems: number
      }>
    >
  > {
    const response = await this.axios.get<ApiResponse<any>>('/api/training/recommended-sets', {
      params: { page, pageSize, isContest },
    })
    return processResponse(response)
  }

  // 获取训练历史
  async getTrainingHistory(isContest?: number): Promise<
    ApiResponse<
      Array<{
        id: number
        date: string
        problemSetId: number
        problemSetName: string
        totalProblems: number
        status: number
      }>
    >
  > {
    const response = await this.axios.get<ApiResponse<any>>('/api/training/history', {
      params: { isContest },
    })
    return processResponse(response)
  }

  // 获取题目组详情
  async getProblemSetDetail(
    setId: number,
    isContest?: number,
  ): Promise<
    ApiResponse<{
      id: number
      name: string
      description: string
      startTime: string
      endTime: string
      createTime: string
      creatorId: number
      problems: Array<{
        id: number
        title: string
        difficulty: number
        tags: string[]
        completed: number
      }>
    }>
  > {
    const params: any = {}
    if (isContest !== undefined) {
      params.isContest = isContest
    }
    const response = await this.axios.get<ApiResponse<any>>(`/api/training/set/${setId}`, {
      params,
    })
    return processResponse(response)
  }

  // 创建训练计划（题目组）
  async createProblemSet(setData: {
    name: string
    description: string
    startTime?: string
    endTime?: string
    problemIds: number[]
    isContest?: number
  }): Promise<ApiResponse<any>> {
    const response = await this.axios.post<ApiResponse<any>>('/api/training/create', setData)
    return processResponse(response)
  }

  // 删除题目组
  async deleteProblemSet(setId: number): Promise<ApiResponse<any>> {
    const response = await this.axios.delete<ApiResponse<any>>(`/api/training/${setId}`)
    return processResponse(response)
  }
}

export const apiService = new ApiService()
