// 支持的编程语言配置
// 可以由管理员或开发者进行修改

export interface LanguageConfig {
  name: string // 语言名称
  value: string // 语言值（用于API调用）
  extension: string // 文件扩展名
  visible: boolean // 是否在界面中显示
}

export const supportedLanguages: LanguageConfig[] = [
  { name: 'C', value: 'C', extension: '.c', visible: true },
  { name: 'C++', value: 'C++', extension: '.cpp', visible: true },
  { name: 'Java', value: 'Java', extension: '.java', visible: true },
  { name: 'Python 3', value: 'Python3', extension: '.py', visible: true },
  { name: 'JavaScript', value: 'JavaScript', extension: '.js', visible: true },
  { name: 'Go', value: 'Go', extension: '.go', visible: true },
  { name: 'Rust', value: 'Rust', extension: '.rs', visible: false },
  { name: 'PHP', value: 'PHP', extension: '.php', visible: false },
  { name: 'Ruby', value: 'Ruby', extension: '.rb', visible: false },
  { name: 'Swift', value: 'Swift', extension: '.swift', visible: false },
  { name: 'Kotlin', value: 'Kotlin', extension: '.kt', visible: false },
  { name: 'C#', value: 'C#', extension: '.cs', visible: false },
  { name: 'TypeScript', value: 'TypeScript', extension: '.ts', visible: true },
  { name: 'PHP', value: 'PHP', extension: '.php', visible: false },
  { name: 'Scala', value: 'Scala', extension: '.scala', visible: false },
]

// 获取可见的语言列表
export const getVisibleLanguages = (): LanguageConfig[] => {
  return supportedLanguages.filter((lang) => lang.visible)
}

// 获取语言值列表（用于下拉选择）
export const getLanguageValues = (): string[] => {
  return getVisibleLanguages().map((lang) => lang.value)
}

// 根据语言值获取语言配置
export const getLanguageByValue = (value: string): LanguageConfig | undefined => {
  return supportedLanguages.find((lang) => lang.value === value)
}
