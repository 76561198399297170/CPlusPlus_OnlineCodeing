// src/utils/encoding.ts

export function decodeU8String(data: any): any {
  if (Array.isArray(data)) {
    // 空数组直接返回，不进行处理
    if (data.length === 0) {
      return data
    }
    const isByteArray = data.every(
      (item: any) => typeof item === 'number' && item >= 0 && item <= 255,
    )
    if (isByteArray) {
      let str = new TextDecoder('utf-8').decode(new Uint8Array(data))
      str = str.replace(/\0/g, '').trim()
      return str
    } else {
      return data.map((item: any) => decodeU8String(item))
    }
  } else if (typeof data === 'object' && data !== null) {
    const processedData: Record<string, any> = {}
    for (const key in data) {
      if (Object.prototype.hasOwnProperty.call(data, key)) {
        processedData[key] = decodeU8String(data[key])
      }
    }
    return processedData
  }
  return data
}

export function isU8String(data: any): boolean {
  if (!Array.isArray(data)) return false
  if (data.length === 0) return false
  return data.every((item: any) => typeof item === 'number' && item >= 0 && item <= 255)
}
