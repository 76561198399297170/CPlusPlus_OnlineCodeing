
&lt;!-- src/components/Common/DateTimePicker.vue --&gt;
&lt;template&gt;
  &lt;div class="date-time-picker"&gt;
    &lt;div class="picker-input" @click="togglePicker"&gt;
      &lt;input
        type="text"
        :value="displayValue"
        readonly
        placeholder="请选择日期时间"
      /&gt;
      &lt;span class="calendar-icon"&gt;📅&lt;/span&gt;
    &lt;/div&gt;

    &lt;div v-if="showPicker" class="picker-overlay" @click="closePicker"&gt;
      &lt;div class="picker-content" @click.stop&gt;
        &lt;div class="picker-header"&gt;
          &lt;h3&gt;选择日期时间&lt;/h3&gt;
          &lt;button class="close-btn" @click="closePicker"&gt;×&lt;/button&gt;
        &lt;/div&gt;

        &lt;div class="picker-body"&gt;
          &lt;!-- 日历部分 --&gt;
          &lt;div class="calendar-section"&gt;
            &lt;div class="calendar-header"&gt;
              &lt;button class="nav-btn" @click="prevMonth"&gt;‹&lt;/button&gt;
              &lt;span class="month-year"&gt;{{ currentMonthYear }}&lt;/span&gt;
              &lt;button class="nav-btn" @click="nextMonth"&gt;›&lt;/button&gt;
            &lt;/div&gt;
            &lt;div class="calendar-weekdays"&gt;
              &lt;span v-for="day in ['日', '一', '二', '三', '四', '五', '六']" :key="day"&gt;{{ day }}&lt;/span&gt;
            &lt;/div&gt;
            &lt;div class="calendar-days"&gt;
              &lt;span
                v-for="(day, index) in calendarDays"
                :key="index"
                class="day"
                :class="{
                  'other-month': !day.isCurrentMonth,
                  'today': day.isToday,
                  'selected': day.isSelected
                }"
                @click="selectDate(day.date)"
              &gt;
                {{ day.day }}
              &lt;/span&gt;
            &lt;/div&gt;
          &lt;/div&gt;

          &lt;!-- 时间滚轮部分 --&gt;
          &lt;div class="time-section"&gt;
            &lt;div class="time-label"&gt;时&lt;/div&gt;
            &lt;div class="wheel-container" ref="hourWheelRef"&gt;
              &lt;div class="wheel-scroll"&gt;
                &lt;div
                  v-for="h in 24"
                  :key="h - 1"
                  class="wheel-item"
                  :class="{ 'selected': selectedHour === h - 1 }"
                  @click="selectHour(h - 1)"
                &gt;
                  {{ String(h - 1).padStart(2, '0') }}
                &lt;/div&gt;
              &lt;/div&gt;
              &lt;div class="wheel-indicator"&gt;&lt;/div&gt;
            &lt;/div&gt;
            &lt;div class="time-separator"&gt;:&lt;/div&gt;
            &lt;div class="time-label"&gt;分&lt;/div&gt;
            &lt;div class="wheel-container" ref="minuteWheelRef"&gt;
              &lt;div class="wheel-scroll"&gt;
                &lt;div
                  v-for="m in 60"
                  :key="m - 1"
                  class="wheel-item"
                  :class="{ 'selected': selectedMinute === m - 1 }"
                  @click="selectMinute(m - 1)"
                &gt;
                  {{ String(m - 1).padStart(2, '0') }}
                &lt;/div&gt;
              &lt;/div&gt;
              &lt;div class="wheel-indicator"&gt;&lt;/div&gt;
            &lt;/div&gt;
          &lt;/div&gt;
        &lt;/div&gt;

        &lt;div class="picker-footer"&gt;
          &lt;button class="btn btn-cancel" @click="closePicker"&gt;取消&lt;/button&gt;
          &lt;button class="btn btn-confirm" @click="confirm"&gt;确定&lt;/button&gt;
        &lt;/div&gt;
      &lt;/div&gt;
    &lt;/div&gt;
  &lt;/div&gt;
&lt;/template&gt;

&lt;script setup&gt;
import { ref, computed, onMounted, watch } from 'vue'

const props = defineProps({
  modelValue: {
    type: String,
    default: ''
  }
})

const emit = defineEmits(['update:modelValue'])

const showPicker = ref(false)
const selectedDate = ref(new Date())
const selectedHour = ref(0)
const selectedMinute = ref(0)
const currentMonth = ref(new Date())
const hourWheelRef = ref(null)
const minuteWheelRef = ref(null)

const displayValue = computed(() =&gt; {
  if (!props.modelValue) return ''
  const date = new Date(props.modelValue)
  if (isNaN(date.getTime())) return ''
  return formatDateTime(date)
})

const calendarDays = computed(() =&gt; {
  const days = []
  const year = currentMonth.value.getFullYear()
  const month = currentMonth.value.getMonth()
  
  const firstDay = new Date(year, month, 1)
  const lastDay = new Date(year, month + 1, 0)
  const prevLastDay = new Date(year, month, 0)
  
  const firstDayOfWeek = firstDay.getDay()
  const lastDateOfMonth = lastDay.getDate()
  const prevLastDate = prevLastDay.getDate()
  
  for (let i = firstDayOfWeek - 1; i &gt;= 0; i--) {
    days.push({
      day: prevLastDate - i,
      date: new Date(year, month - 1, prevLastDate - i),
      isCurrentMonth: false,
      isToday: false,
      isSelected: false
    })
  }
  
  const today = new Date()
  const todayStr = formatDate(today)
  const selectedStr = formatDate(selectedDate.value)
  
  for (let i = 1; i &lt;= lastDateOfMonth; i++) {
    const date = new Date(year, month, i)
    days.push({
      day: i,
      date: date,
      isCurrentMonth: true,
      isToday: formatDate(date) === todayStr,
      isSelected: formatDate(date) === selectedStr
    })
  }
  
  const remainingDays = 42 - days.length
  for (let i = 1; i &lt;= remainingDays; i++) {
    days.push({
      day: i,
      date: new Date(year, month + 1, i),
      isCurrentMonth: false,
      isToday: false,
      isSelected: false
    })
  }
  
  return days
})

const currentMonthYear = computed(() =&gt; {
  const year = currentMonth.value.getFullYear()
  const month = currentMonth.value.getMonth() + 1
  return `${year}年${month}月`
})

const formatDate = (date) =&gt; {
  const year = date.getFullYear()
  const month = String(date.getMonth() + 1).padStart(2, '0')
  const day = String(date.getDate()).padStart(2, '0')
  return `${year}-${month}-${day}`
}

const formatDateTime = (date) =&gt; {
  const dateStr = formatDate(date)
  const hours = String(date.getHours()).padStart(2, '0')
  const minutes = String(date.getMinutes()).padStart(2, '0')
  const seconds = String(date.getSeconds()).padStart(2, '0')
  return `${dateStr} ${hours}:${minutes}:${seconds}`
}

const togglePicker = () =&gt; {
  showPicker.value = !showPicker.value
  if (showPicker.value) {
    initPicker()
  }
}

const closePicker = () =&gt; {
  showPicker.value = false
}

const initPicker = () =&gt; {
  if (props.modelValue) {
    const date = new Date(props.modelValue)
    if (!isNaN(date.getTime())) {
      selectedDate.value = new Date(date.getFullYear(), date.getMonth(), date.getDate())
      selectedHour.value = date.getHours()
      selectedMinute.value = date.getMinutes()
      currentMonth.value = new Date(date.getFullYear(), date.getMonth())
      return
    }
  }
  
  const now = new Date()
  selectedDate.value = new Date(now.getFullYear(), now.getMonth(), now.getDate())
  selectedHour.value = now.getHours()
  selectedMinute.value = now.getMinutes()
  currentMonth.value = new Date(now.getFullYear(), now.getMonth())
}

const prevMonth = () =&gt; {
  currentMonth.value = new Date(currentMonth.value.getFullYear(), currentMonth.value.getMonth() - 1)
}

const nextMonth = () =&gt; {
  currentMonth.value = new Date(currentMonth.value.getFullYear(), currentMonth.value.getMonth() + 1)
}

const selectDate = (date) =&gt; {
  selectedDate.value = new Date(date.getFullYear(), date.getMonth(), date.getDate())
  currentMonth.value = new Date(date.getFullYear(), date.getMonth())
}

const selectHour = (hour) =&gt; {
  selectedHour.value = hour
}

const selectMinute = (minute) =&gt; {
  selectedMinute.value = minute
}

const confirm = () =&gt; {
  const date = new Date(
    selectedDate.value.getFullYear(),
    selectedDate.value.getMonth(),
    selectedDate.value.getDate(),
    selectedHour.value,
    selectedMinute.value,
    0
  )
  emit('update:modelValue', formatDateTime(date))
  closePicker()
}

onMounted(() =&gt; {
  if (props.modelValue) {
    initPicker()
  }
})

watch(() =&gt; props.modelValue, (newVal) =&gt; {
  if (newVal) {
    initPicker()
  }
})
&lt;/script&gt;

&lt;style scoped&gt;
.date-time-picker {
  position: relative;
  width: 100%;
}

.picker-input {
  position: relative;
  display: flex;
  align-items: center;
}

.picker-input input {
  width: 100%;
  padding: 10px 40px 10px 12px;
  border: 1px solid #ddd;
  border-radius: 6px;
  font-size: 14px;
  cursor: pointer;
  background: white;
}

.picker-input input:hover {
  border-color: #999;
}

.calendar-icon {
  position: absolute;
  right: 12px;
  font-size: 18px;
  pointer-events: none;
}

.picker-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.picker-content {
  background: white;
  border-radius: 12px;
  width: 500px;
  max-height: 90vh;
  overflow: hidden;
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.2);
}

.picker-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 20px;
  border-bottom: 1px solid #eee;
}

.picker-header h3 {
  margin: 0;
  font-size: 18px;
  font-weight: 600;
}

.close-btn {
  background: none;
  border: none;
  font-size: 28px;
  cursor: pointer;
  color: #666;
  padding: 0;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 50%;
}

.close-btn:hover {
  background: #f5f5f5;
  color: #333;
}

.picker-body {
  display: flex;
  padding: 20px;
  gap: 20px;
}

.calendar-section {
  flex: 1;
}

.calendar-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 12px;
}

.nav-btn {
  background: none;
  border: 1px solid #ddd;
  border-radius: 6px;
  width: 36px;
  height: 36px;
  font-size: 20px;
  cursor: pointer;
  color: #666;
}

.nav-btn:hover {
  background: #f5f5f5;
  border-color: #999;
  color: #333;
}

.month-year {
  font-size: 16px;
  font-weight: 600;
}

.calendar-weekdays {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  gap: 4px;
  margin-bottom: 8px;
}

.calendar-weekdays span {
  text-align: center;
  font-size: 12px;
  font-weight: 600;
  color: #999;
  padding: 8px 0;
}

.calendar-days {
  display: grid;
  grid-template-columns: repeat(7, 1fr);
  gap: 4px;
}

.day {
  text-align: center;
  padding: 10px 0;
  font-size: 14px;
  border-radius: 6px;
  cursor: pointer;
  color: #333;
}

.day:hover {
  background: #f0f0f0;
}

.day.other-month {
  color: #ccc;
}

.day.today {
  background: #e3f2fd;
  color: #1976d2;
  font-weight: 600;
}

.day.selected {
  background: #1976d2;
  color: white;
  font-weight: 600;
}

.day.selected:hover {
  background: #1565c0;
}

.time-section {
  width: 180px;
  display: flex;
  align-items: center;
  gap: 8px;
}

.time-label {
  font-size: 12px;
  font-weight: 600;
  color: #666;
  margin-bottom: 4px;
  align-self: flex-start;
}

.wheel-container {
  flex: 1;
  height: 200px;
  position: relative;
  border: 1px solid #ddd;
  border-radius: 8px;
  overflow: hidden;
  background: #fafafa;
}

.wheel-scroll {
  height: 100%;
  overflow-y: auto;
  scroll-snap-type: y mandatory;
  scrollbar-width: none;
  -ms-overflow-style: none;
}

.wheel-scroll::-webkit-scrollbar {
  display: none;
}

.wheel-item {
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  color: #666;
  scroll-snap-align: center;
  cursor: pointer;
}

.wheel-item:hover {
  color: #333;
}

.wheel-item.selected {
  color: #1976d2;
  font-weight: 700;
  font-size: 20px;
}

.wheel-indicator {
  position: absolute;
  top: 50%;
  left: 0;
  right: 0;
  height: 40px;
  transform: translateY(-50%);
  pointer-events: none;
  border-top: 2px solid #1976d2;
  border-bottom: 2px solid #1976d2;
  background: rgba(25, 118, 210, 0.05);
}

.time-separator {
  font-size: 24px;
  font-weight: 700;
  color: #666;
  margin-top: 10px;
}

.picker-footer {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  padding: 16px 20px;
  border-top: 1px solid #eee;
}

.btn {
  padding: 10px 24px;
  border-radius: 6px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  border: none;
}

.btn-cancel {
  background: #f5f5f5;
  color: #666;
}

.btn-cancel:hover {
  background: #e0e0e0;
}

.btn-confirm {
  background: #1976d2;
  color: white;
}

.btn-confirm:hover {
  background: #1565c0;
}
&lt;/style&gt;
