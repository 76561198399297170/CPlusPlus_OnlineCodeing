// src/services/cache.ts
import type { Problem } from './api'
import { decodeU8String } from '@/utils/encoding'

interface EditorCache {
  code: string
  language: string
  timestamp: number
}

// 缓存服务类
class CacheService {
  private storage: Storage
  private readonly PROBLEM_CACHE_KEY = 'code_practice_problems'
  private readonly EDITOR_CACHE_KEY = 'code_practice_editor'
  private readonly CACHE_EXPIRY = 24 * 60 * 60 * 1000 // 24小时缓存
  private readonly EDITOR_CACHE_EXPIRY = 7 * 24 * 60 * 60 * 1000 // 编辑器内容保存7天

  constructor() {
    this.storage = localStorage
  }

  // ========== 题目缓存 ==========

  // 缓存题目数据
  setProblem(id: number | string, problem: Problem): void {
    try {
      const cache = this.getProblemsCache()
      cache[String(id)] = {
        data: decodeU8String(problem),
        timestamp: Date.now(),
      }
      this.storage.setItem(this.PROBLEM_CACHE_KEY, JSON.stringify(cache))
    } catch (error) {
      // 缓存题目失败
    }
  }

  // 获取缓存的题目数据
  getProblem(id: number | string): Problem | null {
    try {
      const cache = this.getProblemsCache()
      const cachedItem = cache[String(id)]

      if (cachedItem) {
        // 检查缓存是否过期
        if (Date.now() - cachedItem.timestamp < this.CACHE_EXPIRY) {
          return cachedItem.data
        } else {
          // 缓存过期，删除
          delete cache[String(id)]
          this.storage.setItem(this.PROBLEM_CACHE_KEY, JSON.stringify(cache))
          return null
        }
      }
      return null
    } catch (error) {
      return null
    }
  }

  // 清除指定题目的缓存
  clearProblem(id: number | string): void {
    try {
      const cache = this.getProblemsCache()
      delete cache[String(id)]
      this.storage.setItem(this.PROBLEM_CACHE_KEY, JSON.stringify(cache))
    } catch (error) {
      // 清除题目缓存失败
    }
  }

  // 清除所有题目缓存
  clearAllProblems(): void {
    try {
      this.storage.removeItem(this.PROBLEM_CACHE_KEY)
    } catch (error) {
      // 清除所有题目缓存失败
    }
  }

  // 获取所有缓存的题目
  private getProblemsCache(): Record<string, { data: Problem; timestamp: number }> {
    try {
      const cache = this.storage.getItem(this.PROBLEM_CACHE_KEY)
      return cache ? JSON.parse(cache) : {}
    } catch (error) {
      return {}
    }
  }

  // ========== 编辑器内容缓存 ==========

  // 生成编辑器缓存的 key
  private getEditorCacheKey(problemId: number, problemSetId?: string | number | null): string {
    if (problemSetId) {
      return `problem_${problemId}_set_${problemSetId}`
    }
    return `problem_${problemId}`
  }

  // 缓存编辑器内容
  setEditorContent(
    problemId: number,
    problemSetId: string | number | null | undefined,
    code: string,
    language: string,
  ): void {
    try {
      const cache = this.getEditorCache()
      const key = this.getEditorCacheKey(problemId, problemSetId)
      cache[key] = {
        code,
        language,
        timestamp: Date.now(),
      }
      this.storage.setItem(this.EDITOR_CACHE_KEY, JSON.stringify(cache))
    } catch (error) {
      // 缓存编辑器内容失败
    }
  }

  // 获取缓存的编辑器内容
  getEditorContent(
    problemId: number,
    problemSetId: string | number | null | undefined,
  ): { code: string; language: string } | null {
    try {
      const cache = this.getEditorCache()
      const key = this.getEditorCacheKey(problemId, problemSetId)
      const cachedItem = cache[key]

      if (cachedItem) {
        // 检查缓存是否过期
        if (Date.now() - cachedItem.timestamp < this.EDITOR_CACHE_EXPIRY) {
          return {
            code: cachedItem.code,
            language: cachedItem.language,
          }
        } else {
          // 缓存过期，删除
          delete cache[key]
          this.storage.setItem(this.EDITOR_CACHE_KEY, JSON.stringify(cache))
          return null
        }
      }
      return null
    } catch (error) {
      return null
    }
  }

  // 清除指定题目的编辑器缓存
  clearEditorContent(problemId: number, problemSetId: string | number | null | undefined): void {
    try {
      const cache = this.getEditorCache()
      const key = this.getEditorCacheKey(problemId, problemSetId)
      delete cache[key]
      this.storage.setItem(this.EDITOR_CACHE_KEY, JSON.stringify(cache))
    } catch (error) {
      // 清除编辑器缓存失败
    }
  }

  // 清除所有过期的编辑器缓存
  clearExpiredEditorCache(): void {
    try {
      const cache = this.getEditorCache()
      const now = Date.now()
      let hasChanges = false
      for (const key in cache) {
        if (now - cache[key].timestamp >= this.EDITOR_CACHE_EXPIRY) {
          delete cache[key]
          hasChanges = true
        }
      }
      if (hasChanges) {
        this.storage.setItem(this.EDITOR_CACHE_KEY, JSON.stringify(cache))
      }
    } catch (error) {
      // 清理过期缓存失败
    }
  }

  // 清除所有编辑器缓存
  clearAllEditorCache(): void {
    try {
      this.storage.removeItem(this.EDITOR_CACHE_KEY)
    } catch (error) {
      // 清除所有编辑器缓存失败
    }
  }

  // 获取编辑器缓存
  private getEditorCache(): Record<string, EditorCache> {
    try {
      const cache = this.storage.getItem(this.EDITOR_CACHE_KEY)
      return cache ? JSON.parse(cache) : {}
    } catch (error) {
      return {}
    }
  }
}

export const cacheService = new CacheService()
