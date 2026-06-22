# 项目 API 请求规范文档

> OnlineJudge 在线判题系统 - 前端请求规范

## 目录

- [1. 概述](#1-概述)
  - [1.1 基本信息](#11-基本信息)
  - [1.2 请求格式](#12-请求格式)
  - [1.3 响应格式](#13-响应格式)
- [2. 认证相关](#2-认证相关)
- [3. 题目相关](#3-题目相关)
- [4. 提交相关](#4-提交相关)
- [5. 用户相关](#5-用户相关)
- [6. 竞赛相关](#6-竞赛相关)
- [7. 智能辅助](#7-智能辅助)

---

## 1. 概述

### 1.1 基本信息

**基础 URL**: `/api`

**认证方式**: Bearer Token (JWT)

所有需要认证的请求都必须在请求头中包含：

```
Authorization: Bearer <token>
```

### 1.2 请求格式

**请求头**:

```
Content-Type: application/json; charset=utf-8
Authorization: Bearer <token>  // 需要认证的接口
```

**请求体** (POST/PUT):

```json
{
  "field1": "value1",
  "field2": "value2"
}
```

### 1.3 响应格式

**成功响应**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

**错误响应**:

```json
{
  "code": -1,
  "msg": "错误描述",
  "data": {}
}
```

**状态码说明**:

| code | 说明                       |
| ---- | -------------------------- |
| 0    | 成功                       |
| -1   | 通用错误                   |
| -401 | Token无效/过期             |
| -403 | 无权限                     |
| -404 | 资源不存在                 |
| 401  | Token无效/过期（兼容旧版） |

---

## 2. 认证相关

### 2.1 用户登录

**接口**: `/api/login`
**方法**: POST
**功能**: 用户登录

**请求体**:

| 字段名   | 类型   | 必填 | 默认值 | 描述                        |
| -------- | ------ | ---- | ------ | --------------------------- |
| account  | string | 是   | -      | 账号                        |
| password | string | 是   | -      | 密码                        |
| role     | string | 是   | -      | 角色(admin/teacher/student) |

**请求示例**:

```json
{
  "account": "admin",
  "password": "password123",
  "role": "admin"
}
```

**响应结构**:

| 字段名   | 类型   | 描述                        |
| -------- | ------ | --------------------------- |
| id       | number | 用户ID                      |
| account  | string | 账号                        |
| nickname | string | 昵称                        |
| role     | string | 角色(admin/teacher/student) |
| token    | string | JWT认证令牌                 |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 1,
    "account": "admin",
    "nickname": "管理员",
    "role": "admin",
    "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
  }
}
```

### 2.2 用户注册

**接口**: `/api/register`
**方法**: POST
**功能**: 用户注册

**请求体**:

| 字段名        | 类型   | 必填 | 默认值 | 描述                                        |
| ------------- | ------ | ---- | ------ | ------------------------------------------- |
| account       | string | 是   | -      | 账号（必填）                                |
| password      | string | 是   | -      | 密码（必填）                                |
| nickname      | string | 是   | -      | 昵称（必填）                                |
| role          | string | 是   | -      | 角色（必填，admin/teacher/student）         |
| phone         | string | 是   | -      | 手机号（必填）                              |
| email         | string | 是   | -      | 邮箱（必填）                                |
| school        | string | 条件 | -      | 学校（学生/老师角色必填）                   |
| grade         | number | 条件 | -      | 年级（学生角色必填）                        |
| real_name     | string | 条件 | -      | 真实姓名（老师角色必填）                    |
| auditAdmin    | string | 条件 | -      | 审核管理员（老师注册时需指定，admin可忽略） |
| auditPassword | string | 条件 | -      | 审核密码（老师注册时需填写）                |

**请求示例**:

```json
{
  "account": "teacher1",
  "password": "password123",
  "nickname": "张老师",
  "role": "teacher",
  "phone": "13800138000",
  "email": "teacher@example.com",
  "school": "清华大学",
  "real_name": "张三",
  "auditAdmin": "admin",
  "auditPassword": "superpassword"
}
```

**响应结构**:

同登录接口

### 2.3 修改昵称

**接口**: `/api/user/nickname`
**方法**: PUT
**功能**: 修改当前用户的昵称

**请求体**:

| 字段名   | 类型   | 必填 | 默认值 | 描述   |
| -------- | ------ | ---- | ------ | ------ |
| nickname | string | 是   | -      | 新昵称 |

**请求示例**:

```json
{
  "nickname": "新昵称"
}
```

**响应结构**:

同登录接口

### 2.4 修改密码

**接口**: `/api/user/password`
**方法**: PUT
**功能**: 修改当前用户或其他用户的密码

**请求体**:

| 字段名      | 类型   | 必填 | 默认值 | 描述                     |
| ----------- | ------ | ---- | ------ | ------------------------ |
| oldPassword | string | 是   | -      | 原密码                   |
| newPassword | string | 是   | -      | 新密码                   |
| userId      | number | 否   | -      | 目标用户ID（管理员可填） |

**请求示例**:

```json
{
  "oldPassword": "oldpassword",
  "newPassword": "newpassword",
  "userId": 2
}
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

---

## 3. 题目相关

### 3.1 获取题目列表

**接口**: `/api/problems`
**方法**: GET
**功能**: 获取题目列表（支持分页）

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 10     | 每页数量 |

**请求示例**:

```http
GET /api/problems?page=1&pageSize=10
```

**响应结构**:

| 字段名   | 类型   | 描述     |
| -------- | ------ | -------- |
| list     | array  | 题目列表 |
| total    | number | 题目总数 |
| page     | number | 当前页码 |
| pageSize | number | 每页数量 |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "list": [
      {
        "id": 1,
        "title": "两数之和",
        "difficulty": 1,
        "tags": ["数组", "哈希表"],
        "completed": 0,
        "timeLimit": 1000,
        "memoryLimit": 256
      }
    ],
    "total": 100,
    "page": 1,
    "pageSize": 10
  }
}
```

**说明**: `completed` 字段为数字类型（0: 未开始, 1: 提交未通过, 2: 提交通过）

### 3.2 获取题目详情

**接口**: `/api/problems/:id`
**方法**: GET
**功能**: 获取单个题目的详细信息，支持传入题目组ID用于过滤未开赛的题目

**路径参数**:

| 字段名 | 类型   | 必填 | 描述   |
| ------ | ------ | ---- | ------ |
| id     | number | 是   | 题目ID |

**查询参数**:

| 字段名          | 类型    | 必填 | 描述                     |
| --------------- | ------- | ---- | ------------------------ |
| problem_set_id  | number  | 否   | 题目组ID，用于过滤未开赛题目 |

**请求示例**:

```http
GET /api/problems/1?problem_set_id=7
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 1,
    "title": "两数之和",
    "description": "给定一个整数数组 nums...",
    "inputFormat": "第一行包含一个整数 n...",
    "outputFormat": "输出一个整数表示...",
    "sample": "样例1...",
    "difficulty": 1,
    "tags": ["数组", "哈希表"],
    "timeLimit": 1000,
    "memoryLimit": 256,
    "completed": 0,
    "languageLimits": {
      "C++": { "timeLimit": 1000, "memoryLimit": 256 },
      "Java": { "timeLimit": 2000, "memoryLimit": 512 }
    }
  }
}
```

### 3.3 获取推荐题目

**接口**: `/api/problems/recommended`
**方法**: GET
**功能**: 获取推荐题目列表

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 10     | 每页数量 |

**请求示例**:

```http
GET /api/problems/recommended?page=1&pageSize=10
```

**响应结构**:

同获取题目列表接口

### 3.4 搜索题目

**接口**: `/api/problems/search`
**方法**: POST
**功能**: 统一搜索题目（支持关键词、标签、难度筛选）

**请求体**:

| 字段名     | 类型   | 必填 | 默认值 | 描述          |
| ---------- | ------ | ---- | ------ | ------------- |
| keyword    | string | 否   | -      | 搜索关键词    |
| tags       | array  | 否   | -      | 标签数组      |
| difficulty | number | 否   | -      | 难度等级(1-5) |
| page       | number | 否   | 1      | 页码          |
| pageSize   | number | 否   | 10     | 每页数量      |

**请求示例**:

```json
{
  "keyword": "两数之和",
  "tags": ["数组"],
  "difficulty": 1,
  "page": 1,
  "pageSize": 10
}
```

**响应结构**:

同获取题目列表接口

---

## 4. 提交相关

### 4.1 提交代码

**接口**: `/api/submissions`
**方法**: POST
**功能**: 提交代码进行判题

**请求体**:

| 字段名         | 类型   | 必填 | 默认值 | 描述                        |
| -------------- | ------ | ---- | ------ | --------------------------- |
| question_id    | number | 是   | -      | 题目ID                      |
| code           | string | 是   | -      | 提交的代码                  |
| language       | string | 是   | -      | 编程语言                    |
| problem_set_id | number | 否   | -      | 题目组ID（竞赛/训练时使用） |

**请求示例**:

```json
{
  "question_id": 1,
  "code": "function twoSum(nums, target) { ... }",
  "language": "javascript",
  "problem_set_id": 1
}
```

**响应结构**:

| 字段名        | 类型   | 描述         |
| ------------- | ------ | ------------ |
| id            | number | 提交记录ID   |
| question_id   | number | 题目ID       |
| userId        | number | 用户ID       |
| code          | string | 提交的代码   |
| language      | string | 编程语言     |
| status        | string | 判题状态     |
| score         | number | 得分         |
| executionTime | number | 执行时间(ms) |
| memoryUsed    | number | 内存使用(KB) |
| errorMessage  | string | 错误信息     |
| createdAt     | string | 创建时间     |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 1001,
    "question_id": 1,
    "userId": 1,
    "code": "function twoSum(nums, target) { ... }",
    "language": "javascript",
    "status": "AC",
    "score": 100,
    "executionTime": 100,
    "memoryUsed": 16,
    "errorMessage": "",
    "createdAt": "2026-03-16 12:00:00"
  }
}
```

**判题状态说明**:

| 状态 | 说明       |
| ---- | ---------- |
| AC   | 答案正确   |
| WA   | 答案错误   |
| CE   | 编译错误   |
| TLE  | 时间超限   |
| MLE  | 内存超限   |
| RE   | 运行时错误 |
| ERR  | 系统错误   |

**说明**: 该接口使用重试机制，最多重试3次

### 4.2 运行测试

**接口**: `/api/run-test`
**方法**: POST
**功能**: 运行单个测试用例（不保存提交记录）

**请求体**:

| 字段名          | 类型   | 必填 | 默认值 | 描述                        |
| --------------- | ------ | ---- | ------ | --------------------------- |
| question_id     | number | 是   | -      | 题目ID                      |
| code            | string | 是   | -      | 提交的代码                  |
| language        | string | 是   | -      | 编程语言                    |
| testCase        | object | 是   | -      | 测试用例                    |
| testCase.input  | string | 是   | -      | 测试输入                    |
| testCase.output | string | 是   | -      | 测试输出                    |
| problem_set_id  | number | 否   | -      | 题目组ID（竞赛/训练时使用） |

**请求示例**:

```json
{
  "question_id": 1,
  "code": "function twoSum(nums, target) { ... }",
  "language": "javascript",
  "testCase": {
    "input": "2\n1 2\n",
    "output": "3\n"
  },
  "problem_set_id": 1
}
```

**响应结构**:

| 字段名        | 类型   | 描述         |
| ------------- | ------ | ------------ |
| output        | string | 实际输出     |
| status        | string | 判题状态     |
| executionTime | number | 执行时间(ms) |
| memoryUsed    | number | 内存使用(KB) |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "output": "3\n",
    "status": "AC",
    "executionTime": 100,
    "memoryUsed": 16
  }
}
```

**说明**: 该接口使用重试机制，最多重试3次

### 4.3 获取用户提交记录

**接口**: `/api/submissions`
**方法**: GET
**功能**: 获取当前用户的代码提交记录（支持分页）

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述                                 |
| -------- | ------ | ---- | ------ | ------------------------------------ |
| page     | number | 否   | 1      | 页码                                 |
| pageSize | number | 否   | 10     | 每页数量                             |
| userId   | number | 否   | -      | 用户ID（可选，管理员可查看其他用户） |

**请求示例**:

```http
GET /api/submissions?page=1&pageSize=10
GET /api/submissions?page=1&pageSize=10&userId=2
```

**响应结构**:

| 字段名   | 类型   | 描述         |
| -------- | ------ | ------------ |
| list     | array  | 提交记录列表 |
| total    | number | 记录总数     |
| page     | number | 当前页码     |
| pageSize | number | 每页数量     |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "list": [
      {
        "id": 1001,
        "question_id": 1,
        "userId": 1,
        "code": "function twoSum(nums, target) { ... }",
        "language": "javascript",
        "status": "AC",
        "score": 100,
        "executionTime": 100,
        "memoryUsed": 16,
        "errorMessage": "",
        "createdAt": "2026-03-16 12:00:00"
      }
    ],
    "total": 50,
    "page": 1,
    "pageSize": 10
  }
}
```

### 4.4 获取题目组提交统计

**接口**: `/api/submissions/stats/:problemSetId`
**方法**: GET
**功能**: 获取指定题目组的全部提交记录，支持获取竞赛或训练记录

**路径参数**:

| 字段名       | 类型   | 必填 | 描述      |
| ------------ | ------ | ---- | --------- |
| problemSetId | number | 是   | 题目组ID  |

**请求示例**:

```http
GET /api/submissions/stats/7
```

**响应结构**:

| 字段名   | 类型   | 描述                        |
| -------- | ------ | --------------------------- |
| list     | array  | 提交记录列表（每个元素是JSON字符串） |
| total    | number | 记录总数                     |

list中每个JSON字符串解析后的对象包含以下字段：

| 字段名        | 类型   | 描述                         |
| ------------- | ------ | ---------------------------- |
| userId        | number | 用户ID                       |
| language      | string | 提交语言                     |
| status        | string | 提交状态                     |
| question_id   | number | 题目ID                       |
| score         | number | 分数                         |
| executionTime | number | 执行时间(ms)                 |
| memoryUsed    | number | 内存使用(KB)                 |
| createdAt     | string | 提交时间（格式：2026-04-12 23:02:17） |

**提交状态说明**:

| 状态字符串        | 简写   | 描述     |
| ----------------- | ------ | -------- |
| NOT_DEPLOYED      | ND     | 当时未部署 |
| COMPILE_ERROR     | CE     | 编译错误   |
| RUNTIME_ERROR     | RE     | 运行错误   |
| TIME_LIMIT_EXCEEDED | TLE   | 超时     |
| ACCEPTED          | AC     | 通过     |
| WRONG_ANSWER      | WA     | 答案错误   |
| UNKNOW            | -      | 未知     |

**响应示例**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "list": [
      "{\"userId\":1001,\"language\":\"C++\",\"status\":\"ACCEPTED\",\"question_id\":100009,\"score\":100,\"executionTime\":45,\"memoryUsed\":2048,\"createdAt\":\"2026-04-12 23:02:17\"}",
      "{\"userId\":1002,\"language\":\"Python3\",\"status\":\"COMPILE_ERROR\",\"question_id\":100009,\"score\":0,\"executionTime\":0,\"memoryUsed\":0,\"createdAt\":\"2026-04-12 22:45:30\"}"
    ],
    "total": 2
  }
}
```

---

## 5. 用户相关

### 5.1 获取用户信息

**接口**: `/api/user/info`
**方法**: GET
**功能**: 获取当前用户信息，若token有效时间不足设置时间`0.5`则生成新token

**请求示例**:

```http
GET /api/user/info
```

**响应结构**:

同登录接口

### 5.2 获取其他用户信息

**接口**: `/api/user/profile/:userId`
**方法**: GET
**功能**: 获取指定用户的信息

**路径参数**:

| 字段名 | 类型   | 必填 | 描述   |
| ------ | ------ | ---- | ------ |
| userId | number | 是   | 用户ID |

**请求示例**:

```http
GET /api/user/profile/2
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 2,
    "account": "user2",
    "nickname": "用户2",
    "role": "student",
    "school": "清华大学",
    "grade": 3,
    "userDesc": "这是用户描述",
    "submissionsCount": 50,
    "solvedCount": 30,
    "submissionStats": [
      { "language": "C++", "count": 30 },
      { "language": "Python", "count": 20 }
    ]
  }
}
```

### 5.3 获取用户列表

**接口**: `/api/users`
**方法**: GET
**功能**: 获取所有用户列表（仅管理员）

**请求示例**:

```http
GET /api/users
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": [
    {
      "id": 1,
      "account": "admin",
      "nickname": "管理员",
      "role": "admin"
    }
  ]
}
```

### 5.4 获取隐私设置

**接口**: `/api/user/privacy`
**方法**: GET
**功能**: 获取当前用户的隐私设置

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "showRole": true,
    "showSchool": true,
    "showGrade": true,
    "showSubmissionsCount": true,
    "showSolvedCount": true,
    "showSubmissionStats": true,
    "showUserRoleNo": false,
    "showRealName": false
  }
}
```

### 5.5 更新隐私设置

**接口**: `/api/user/privacy`
**方法**: PUT
**功能**: 更新当前用户的隐私设置

**请求体**:

| 字段名               | 类型 | 必填 | 描述             |
| -------------------- | ---- | ---- | ---------------- |
| showRole             | bool | 是   | 显示角色         |
| showSchool           | bool | 是   | 显示学校         |
| showGrade            | bool | 是   | 显示年级         |
| showSubmissionsCount | bool | 是   | 显示提交次数     |
| showSolvedCount      | bool | 是   | 显示解决题目数   |
| showSubmissionStats  | bool | 是   | 显示提交统计     |
| showUserRoleNo       | bool | 是   | 显示用户角色编号 |
| showRealName         | bool | 是   | 显示真实姓名     |

---

## 6. 竞赛相关

### 6.1 获取训练概览

**接口**: `/api/training/overview`
**方法**: GET
**功能**: 获取当前用户的训练概览

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "totalProblemSets": 10,
    "completedProblemSets": 3,
    "totalProblems": 100
  }
}
```

### 6.2 获取进行中的题目组

**接口**: `/api/training/active-sets`
**方法**: GET
**功能**: 获取当前用户进行中的题目组

**查询参数**:

| 字段名    | 类型   | 必填 | 默认值 | 描述                           |
| --------- | ------ | ---- | ------ | ------------------------------ |
| isContest | number | 否   | -      | 是否竞赛(1: 竞赛, 0或空: 训练) |

**请求示例**:

```http
GET /api/training/active-sets
GET /api/training/active-sets?isContest=1
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": [
    {
      "id": 1,
      "name": "算法入门",
      "description": "适合初学者的算法练习",
      "totalProblems": 10
    }
  ]
}
```

### 6.3 获取推荐题目组

**接口**: `/api/training/recommended-sets`
**方法**: GET
**功能**: 获取推荐的题目组列表

**查询参数**:

| 字段名    | 类型   | 必填 | 默认值 | 描述                           |
| --------- | ------ | ---- | ------ | ------------------------------ |
| page      | number | 否   | 1      | 页码                           |
| pageSize  | number | 否   | 10     | 每页数量                       |
| isContest | number | 否   | -      | 是否竞赛(1: 竞赛, 0或空: 训练) |

**请求示例**:

```http
GET /api/training/recommended-sets?page=1&pageSize=10
GET /api/training/recommended-sets?page=1&pageSize=10&isContest=1
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "list": [
      {
        "id": 1,
        "name": "算法入门",
        "description": "适合初学者的算法练习",
        "totalProblems": 10
      }
    ],
    "total": 20,
    "page": 1,
    "pageSize": 10
  }
}
```

### 6.4 获取训练历史

**接口**: `/api/training/history`
**方法**: GET
**功能**: 获取用户训练历史记录

**查询参数**:

| 字段名    | 类型   | 必填 | 默认值 | 描述                           |
| --------- | ------ | ---- | ------ | ------------------------------ |
| isContest | number | 否   | -      | 是否竞赛(1: 竞赛, 0或空: 训练) |

**请求示例**:

```http
GET /api/training/history
GET /api/training/history?isContest=1
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": [
    {
      "id": 1,
      "date": "2026-03-16",
      "problemSetId": 1,
      "problemSetName": "算法入门",
      "totalProblems": 10,
      "status": 2
    }
  ]
}
```

**status说明**:

| 值  | 说明   |
| --- | ------ |
| 0   | 未开始 |
| 1   | 进行中 |
| 2   | 已完成 |

### 6.5 获取题目组详情

**接口**: `/api/training/set/:id`
**方法**: GET
**功能**: 获取题目组详情

**路径参数**:

| 字段名 | 类型   | 必填 | 描述     |
| ------ | ------ | ---- | -------- |
| id     | number | 是   | 题目组ID |

**查询参数**:

| 字段名    | 类型   | 必填 | 默认值 | 描述                           |
| --------- | ------ | ---- | ------ | ------------------------------ |
| isContest | number | 否   | -      | 是否竞赛(1: 竞赛, 0或空: 训练) |

**请求示例**:

```http
GET /api/training/set/1
GET /api/training/set/1?isContest=1
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 1,
    "name": "算法入门",
    "description": "适合初学者的算法练习",
    "startTime": "2026-03-01 00:00:00",
    "endTime": "2026-03-31 23:59:59",
    "createTime": "2026-02-28 12:00:00",
    "creatorId": 1,
    "problems": [
      {
        "id": 1,
        "title": "两数之和",
        "difficulty": 1,
        "tags": ["数组"],
        "completed": 0
      }
    ]
  }
}
```

**说明**:

- `completed` 字段为数字类型（0: 未开始, 1: 未通过, 2: 已通过）
- 当 `isContest=1` 时，后端会检查竞赛时间是否在正确时间段内

### 6.6 创建题目组

**接口**: `/api/training/create`
**方法**: POST
**功能**: 创建一个新的题目组

**请求体**:

| 字段名      | 类型   | 必填 | 描述                           |
| ----------- | ------ | ---- | ------------------------------ |
| name        | string | 是   | 题目组名称                     |
| description | string | 否   | 题目组描述                     |
| startTime   | string | 否   | 开始时间 (YYYY-MM-DD HH:mm:ss) |
| endTime     | string | 否   | 结束时间 (YYYY-MM-DD HH:mm:ss) |
| problemIds  | array  | 是   | 题目ID数组                     |
| isContest   | number | 否   | 是否竞赛(1: 竞赛, 0或空: 训练) |

**请求示例**:

```json
{
  "name": "算法入门",
  "description": "适合初学者的算法练习",
  "startTime": "2026-03-01 00:00:00",
  "endTime": "2026-03-31 23:59:59",
  "problemIds": [1, 2, 3, 4, 5],
  "isContest": 1
}
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

### 6.7 删除题目组

**接口**: `/api/training/:id`
**方法**: DELETE
**功能**: 删除一个题目组（仅管理员和创建者可删除）

**路径参数**:

| 字段名 | 类型   | 必填 | 描述     |
| ------ | ------ | ---- | -------- |
| id     | number | 是   | 题目组ID |

**请求示例**:

```http
DELETE /api/training/1
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

---

## 7. 智能辅助

### 7.1 AI辅助功能

**接口**: `/api/ai/help`
**方法**: POST
**功能**: 提供AI辅助功能（题目解析、代码审查等）

**请求体**:

| 字段名     | 类型   | 必填 | 默认值 | 描述                |
| ---------- | ------ | ---- | ------ | ------------------- |
| mode       | string | 是   | -      | 模式(create/review) |
| functionId | string | 是   | -      | 功能ID              |
| context    | string | 是   | -      | 上下文信息          |
| question   | object | 是   | -      | 问题对象            |
| toolId     | string | 否   | -      | 工具ID              |

**请求示例**:

```json
{
  "mode": "create",
  "functionId": "problem_analysis",
  "context": "用户正在创建一个新题目",
  "question": {
    "title": "两数之和",
    "description": "给定一个整数数组...",
    "difficulty": 1
  }
}
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "content": "这是AI生成的分析内容..."
  }
}
```

**超时设置**:

- create 模式: 60秒
- review 模式: 120秒

---

## 8. 审核相关

### 8.1 获取待审核题目列表

**接口**: `/api/problems/review`
**方法**: GET
**功能**: 获取待审核的题目列表（仅管理员/老师）

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 10     | 每页数量 |

**请求示例**:

```http
GET /api/problems/review?page=1&pageSize=10
```

**响应结构**:

同获取题目列表接口

### 8.2 审核通过题目

**接口**: `/api/problems/review/:id/approve`
**方法**: POST
**功能**: 审核通过一个题目

**路径参数**:

| 字段名 | 类型   | 必填 | 描述   |
| ------ | ------ | ---- | ------ |
| id     | number | 是   | 题目ID |

**请求示例**:

```http
POST /api/problems/review/1/approve
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

### 8.3 审核拒绝题目

**接口**: `/api/problems/review/:id/reject`
**方法**: POST
**功能**: 审核拒绝一个题目

**路径参数**:

| 字段名 | 类型   | 必填 | 描述   |
| ------ | ------ | ---- | ------ |
| id     | number | 是   | 题目ID |

**请求示例**:

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

### 8.4 根据状态获取题目列表

**接口**: `/api/problems/all`
**方法**: GET
**功能**: 获取所有题目（管理员专用，可按状态筛选）

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述                                       |
| -------- | ------ | ---- | ------ | ------------------------------------------ |
| status   | string | 否   | -      | 状态筛选(PENDING/APPROVED/REJECTED/HIDDEN) |
| page     | number | 否   | 1      | 页码                                       |
| pageSize | number | 否   | 10     | 每页数量                                   |

**请求示例**:

```http
GET /api/problems/all?status=ALL&page=1&pageSize=10
GET /api/problems/all?status=PENDING&page=1&pageSize=10
```

**响应结构**:

同获取题目列表接口

### 8.5 获取待审核题目列表

**接口**: `/api/problems/review`
**方法**: GET
**功能**: 获取待审核的题目列表

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 100    | 每页数量 |

**请求示例**:

```http
GET /api/problems/review?page=1&pageSize=100
```

**响应结构**:

同获取题目列表接口

### 8.6 获取已通过题目列表

**接口**: `/api/problems`
**方法**: GET
**功能**: 获取已通过的题目列表

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 100    | 每页数量 |

**请求示例**:

```http
GET /api/problems?page=1&pageSize=100
```

**响应结构**:

同获取题目列表接口

### 8.7 获取已驳回题目列表

**接口**: `/api/problems/rejected`
**方法**: GET
**功能**: 获取已驳回的题目列表

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 100    | 每页数量 |

**请求示例**:

```http
GET /api/problems/rejected?page=1&pageSize=100
```

**响应结构**:

同获取题目列表接口

### 8.8 获取隐藏题目列表

**接口**: `/api/problems/hidden`
**方法**: GET
**功能**: 获取已隐藏的题目列表

**查询参数**:

| 字段名   | 类型   | 必填 | 默认值 | 描述     |
| -------- | ------ | ---- | ------ | -------- |
| page     | number | 否   | 1      | 页码     |
| pageSize | number | 否   | 100    | 每页数量 |

**请求示例**:

```http
GET /api/problems/hidden?page=1&pageSize=100
```

**响应结构**:

同获取题目列表接口

### 8.9 隐藏题目

**接口**: `/api/problems/:id/hide`
**方法**: POST
**功能**: 隐藏一个题目（仅管理员）

**路径参数**:

| 字段名 | 类型   | 必填 | 描述   |
| ------ | ------ | ---- | ------ |
| id     | number | 是   | 题目ID |

**请求示例**:

```http
POST /api/problems/1/hide
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {}
}
```

---

## 9. 统计相关

### 9.1 获取平台统计

**接口**: `/api/stats`
**方法**: GET
**功能**: 获取平台统计数据

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "totalProblems": 1000,
    "supportedLanguages": 7,
    "totalSubmissions": 50000,
    "judgeAccuracy": 99.5
  }
}
```

---

## 10. 创建题目

### 10.1 创建新题目

**接口**: `/api/problems/create`
**方法**: POST
**功能**: 创建一个新题目

**请求体**:

| 字段名                     | 类型   | 必填 | 默认值 | 描述               |
| -------------------------- | ------ | ---- | ------ | ------------------ |
| title                      | string | 是   | -      | 题目标题           |
| difficulty                 | number | 是   | -      | 难度等级(1-5)      |
| tags                       | array  | 是   | -      | 标签数组           |
| timeLimit                  | number | 是   | -      | 时间限制(ms)       |
| memoryLimit                | number | 是   | -      | 内存限制(MB)       |
| description                | string | 是   | -      | 题目描述           |
| inputFormat                | string | 是   | -      | 输入格式           |
| outputFormat               | string | 是   | -      | 输出格式           |
| sample                     | string | 是   | -      | 样例               |
| testCases                  | array  | 是   | -      | 测试用例数组       |
| testCases.input            | string | 是   | -      | 测试输入           |
| testCases.output           | string | 是   | -      | 测试输出           |
| languageLimits             | array  | 是   | -      | 语言限制数组       |
| languageLimits.language    | string | 是   | -      | 编程语言           |
| languageLimits.timeLimit   | number | 是   | -      | 该语言时间限制(ms) |
| languageLimits.memoryLimit | number | 是   | -      | 该语言内存限制(MB) |

**请求示例**:

```json
{
  "title": "两数之和",
  "difficulty": 1,
  "tags": ["数组", "哈希表"],
  "timeLimit": 1000,
  "memoryLimit": 256,
  "description": "给定一个整数数组 nums...",
  "inputFormat": "第一行包含一个整数 n...",
  "outputFormat": "输出一个整数表示...",
  "sample": "样例1...",
  "testCases": [
    { "input": "2\n1 2\n", "output": "3\n" },
    { "input": "5\n1 2 3 4 5\n", "output": "15\n" }
  ],
  "languageLimits": [
    { "language": "C++", "timeLimit": 1000, "memoryLimit": 256 },
    { "language": "Java", "timeLimit": 2000, "memoryLimit": 512 }
  ]
}
```

**响应结构**:

```json
{
  "code": 0,
  "msg": "success",
  "data": {
    "id": 100
  }
}
```
