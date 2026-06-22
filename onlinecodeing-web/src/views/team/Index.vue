<!-- src/views/team/Index.vue -->
<template>
  <MainLayout>
    <div class="team-page container">
      <div class="page-header">
        <h1>组队管理</h1>
        <Button type="primary" @click="createTeam"> + 创建队伍 </Button>
      </div>

      <!-- 我的队伍 -->
      <div class="my-teams-section">
        <h2>我的队伍</h2>
        <div v-if="myTeams.length === 0" class="empty-state">
          <div class="empty-icon">👥</div>
          <div class="empty-text">你还没有加入任何队伍</div>
          <Button type="primary" @click="createTeam">创建队伍</Button>
        </div>
        <div v-else class="teams-grid">
          <div v-for="team in myTeams" :key="team.id" class="team-card">
            <div class="team-header">
              <div class="team-icon">{{ team.name.charAt(0) }}</div>
              <div class="team-info">
                <h3>{{ team.name }}</h3>
                <p>{{ team.description }}</p>
              </div>
            </div>
            <div class="team-members">
              <div class="members-title">成员 ({{ team.members.length }}/{{ team.maxMembers }})</div>
              <div class="members-list">
                <div v-for="member in team.members" :key="member.id" class="member-item">
                  <div class="member-avatar">{{ member.name.charAt(0) }}</div>
                  <div class="member-info">
                    <div class="member-name">{{ member.name }}</div>
                    <div class="member-role" :class="member.role">{{ member.roleText }}</div>
                  </div>
                </div>
              </div>
            </div>
            <div class="team-actions">
              <Button type="primary" size="small" @click="viewTeam(team.id)">查看详情</Button>
              <Button type="default" size="small" @click="leaveTeam(team.id)">退出队伍</Button>
            </div>
          </div>
        </div>
      </div>

      <!-- 组队请求 -->
      <div class="requests-section">
        <h2>组队请求</h2>
        <div v-if="requests.length === 0" class="empty-state">
          <div class="empty-icon">📨</div>
          <div class="empty-text">没有组队请求</div>
        </div>
        <div v-else class="requests-list">
          <div v-for="request in requests" :key="request.id" class="request-card">
            <div class="request-header">
              <div class="user-info">
                <div class="user-avatar">{{ request.user.name.charAt(0) }}</div>
                <div class="user-details">
                  <div class="user-name">{{ request.user.name }}</div>
                  <div class="request-time">{{ request.time }}</div>
                </div>
              </div>
              <div class="request-team">
                <span class="team-name">{{ request.team.name }}</span>
              </div>
            </div>
            <div class="request-message">
              {{ request.message }}
            </div>
            <div class="request-actions">
              <Button type="primary" size="small" @click="acceptRequest(request.id)">接受</Button>
              <Button type="default" size="small" @click="rejectRequest(request.id)">拒绝</Button>
            </div>
          </div>
        </div>
      </div>

      <!-- 推荐队伍 -->
      <div class="recommended-teams-section">
        <h2>推荐队伍</h2>
        <div class="teams-grid">
          <div v-for="team in recommendedTeams" :key="team.id" class="team-card recommended">
            <div class="team-badge">{{ team.badge }}</div>
            <div class="team-header">
              <div class="team-icon">{{ team.name.charAt(0) }}</div>
              <div class="team-info">
                <h3>{{ team.name }}</h3>
                <p>{{ team.description }}</p>
              </div>
            </div>
            <div class="team-members">
              <div class="members-title">成员 ({{ team.members.length }}/{{ team.maxMembers }})</div>
              <div class="members-list">
                <div v-for="member in team.members.slice(0, 3)" :key="member.id" class="member-item">
                  <div class="member-avatar">{{ member.name.charAt(0) }}</div>
                  <div class="member-name">{{ member.name }}</div>
                </div>
                <div v-if="team.members.length > 3" class="member-item more">
                  <div class="member-avatar">+{{ team.members.length - 3 }}</div>
                </div>
              </div>
            </div>
            <div class="team-tags">
              <span v-for="tag in team.tags" :key="tag" class="team-tag">
                {{ tag }}
              </span>
            </div>
            <Button type="primary" size="small" @click="joinTeam(team.id)">申请加入</Button>
          </div>
        </div>
      </div>
    </div>
  </MainLayout>
</template>

<script setup>
import { ref } from 'vue'
import MainLayout from '@/components/Layout/MainLayout.vue'
import Button from '@/components/Common/Button.vue'

// 响应式数据
const myTeams = ref([
  {
    id: 1,
    name: '算法精英队',
    description: '专注于算法竞赛的队伍，目标是在各类比赛中取得好成绩',
    members: [
      { id: 1, name: '张三', role: 'captain', roleText: '队长' },
      { id: 2, name: '李四', role: 'member', roleText: '队员' },
      { id: 3, name: '王五', role: 'member', roleText: '队员' }
    ],
    maxMembers: 5
  },
  {
    id: 2,
    name: '前端开发小组',
    description: '前端开发技术交流与学习小组',
    members: [
      { id: 1, name: '张三', role: 'member', roleText: '队员' },
      { id: 4, name: '赵六', role: 'captain', roleText: '队长' },
      { id: 5, name: '钱七', role: 'member', roleText: '队员' },
      { id: 6, name: '孙八', role: 'member', roleText: '队员' }
    ],
    maxMembers: 6
  }
])

const requests = ref([
  {
    id: 1,
    user: { id: 7, name: '周九' },
    team: { id: 3, name: '后端技术团队' },
    message: '你好，我们正在组建一个后端技术团队，希望你能加入我们！',
    time: '2小时前'
  },
  {
    id: 2,
    user: { id: 8, name: '吴十' },
    team: { id: 4, name: '全栈开发小组' },
    message: '邀请你加入我们的全栈开发小组，一起学习和项目实战',
    time: '1天前'
  }
])

const recommendedTeams = ref([
  {
    id: 3,
    name: '后端技术团队',
    description: '后端开发技术交流与项目实践团队',
    members: [
      { id: 7, name: '周九', role: 'captain', roleText: '队长' },
      { id: 9, name: '郑十一', role: 'member', roleText: '队员' },
      { id: 10, name: '王十二', role: 'member', roleText: '队员' }
    ],
    maxMembers: 5,
    tags: ['后端', 'Java', 'Spring Boot'],
    badge: '热门'
  },
  {
    id: 4,
    name: '全栈开发小组',
    description: '全栈开发技术学习与实践小组',
    members: [
      { id: 8, name: '吴十', role: 'captain', roleText: '队长' },
      { id: 11, name: '陈十三', role: 'member', roleText: '队员' }
    ],
    maxMembers: 6,
    tags: ['全栈', '前端', '后端'],
    badge: '推荐'
  },
  {
    id: 5,
    name: 'AI研究小组',
    description: '人工智能技术研究与应用小组',
    members: [
      { id: 12, name: '林十四', role: 'captain', roleText: '队长' },
      { id: 13, name: '黄十五', role: 'member', roleText: '队员' },
      { id: 14, name: '刘十六', role: 'member', roleText: '队员' },
      { id: 15, name: '赵十七', role: 'member', roleText: '队员' }
    ],
    maxMembers: 8,
    tags: ['AI', '机器学习', '深度学习'],
    badge: '新组'
  }
])

// 方法
const createTeam = () => {
  // 创建队伍逻辑
}

const viewTeam = (teamId) => {
  // 查看队伍详情逻辑
}

const leaveTeam = (teamId) => {
  // 退出队伍逻辑
}

const acceptRequest = (requestId) => {
  // 接受组队请求逻辑
}

const rejectRequest = (requestId) => {
  // 拒绝组队请求逻辑
}

const joinTeam = (teamId) => {
  // 申请加入队伍逻辑
}
</script>

<style scoped>
.team-page {
  padding: 40px 0;
  display: flex;
  flex-direction: column;
  gap: 30px;
}

.page-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.page-header h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 0;
}

/* 我的队伍 */
.my-teams-section,
.requests-section,
.recommended-teams-section {
  background-color: #ffffff;
  border-radius: 8px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  padding: 24px;
}

.my-teams-section h2,
.requests-section h2,
.recommended-teams-section h2 {
  font-size: 18px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 20px;
}

/* 空状态 */
.empty-state {
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
  margin-bottom: 24px;
}

/* 队伍网格 */
.teams-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
}

/* 队伍卡片 */
.team-card {
  padding: 20px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  transition: all 0.2s ease;
  position: relative;
}

.team-card:hover {
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.team-card.recommended {
  border-color: #165dff;
}

.team-badge {
  position: absolute;
  top: -8px;
  right: 20px;
  background-color: #165dff;
  color: #ffffff;
  padding: 4px 12px;
  border-radius: 12px;
  font-size: 12px;
  font-weight: 500;
}

.team-header {
  display: flex;
  align-items: flex-start;
  gap: 16px;
  margin-bottom: 16px;
}

.team-icon {
  width: 50px;
  height: 50px;
  border-radius: 50%;
  background-color: #165dff;
  color: #ffffff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 20px;
  font-weight: 600;
}

.team-info {
  flex: 1;
}

.team-info h3 {
  font-size: 16px;
  font-weight: 600;
  color: #1d2129;
  margin-bottom: 8px;
}

.team-info p {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.5;
  margin-bottom: 0;
}

/* 队伍成员 */
.team-members {
  margin-bottom: 16px;
}

.members-title {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
  margin-bottom: 12px;
}

.members-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.member-item {
  display: flex;
  align-items: center;
  gap: 12px;
}

.member-item.more {
  opacity: 0.6;
}

.member-avatar {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background-color: #e5e6eb;
  color: #4e5969;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  font-weight: 500;
}

.member-info {
  flex: 1;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.member-name {
  font-size: 14px;
  color: #1d2129;
}

.member-role {
  padding: 2px 8px;
  border-radius: 10px;
  font-size: 12px;
  font-weight: 500;
}

.member-role.captain {
  background-color: #e8f0ff;
  color: #165dff;
}

.member-role.member {
  background-color: #f7f8fa;
  color: #4e5969;
}

/* 队伍标签 */
.team-tags {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-bottom: 16px;
}

.team-tag {
  padding: 4px 12px;
  background-color: #f7f8fa;
  color: #4e5969;
  border-radius: 12px;
  font-size: 12px;
}

/* 队伍操作 */
.team-actions {
  display: flex;
  gap: 8px;
  justify-content: flex-end;
}

/* 请求列表 */
.requests-list {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.request-card {
  padding: 20px;
  border: 1px solid #e5e6eb;
  border-radius: 8px;
  transition: all 0.2s ease;
}

.request-card:hover {
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
}

.request-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 12px;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 12px;
}

.user-avatar {
  width: 40px;
  height: 40px;
  border-radius: 50%;
  background-color: #165dff;
  color: #ffffff;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 16px;
  font-weight: 500;
}

.user-details {
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.user-name {
  font-size: 14px;
  font-weight: 500;
  color: #1d2129;
}

.request-time {
  font-size: 12px;
  color: #86909c;
}

.request-team {
  padding: 4px 12px;
  background-color: #e8f0ff;
  color: #165dff;
  border-radius: 12px;
  font-size: 12px;
}

.request-message {
  font-size: 14px;
  color: #4e5969;
  line-height: 1.5;
  margin-bottom: 16px;
}

.request-actions {
  display: flex;
  gap: 8px;
  justify-content: flex-end;
}
</style>