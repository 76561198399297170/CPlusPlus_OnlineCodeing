DROP DATABASE IF EXISTS ocdb;
CREATE DATABASE ocdb CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE ocdb;

CREATE TABLE `user` (
    `user_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '账号编号',
    `account` VARCHAR(25) NOT NULL UNIQUE COMMENT '登录账号',
    `phone` VARCHAR(15) UNIQUE COMMENT '手机号',
    `email` VARCHAR(254) UNIQUE COMMENT '邮箱',
    `password` VARCHAR(64) NOT NULL COMMENT '密码',
    `salt` VARCHAR(17) NOT NULL COMMENT '密码加密盐值',
    `nickname` VARCHAR(15) NOT NULL COMMENT '昵称',
    `user_desc` VARCHAR(50) COMMENT '个性签名',
    `role` TINYINT NOT NULL COMMENT '用户角色',
    `info_display_setting` INT DEFAULT 0 COMMENT '按位存储对外显示设置',
    `create_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '账号创建时间'
) COMMENT '用户信息表';

CREATE TABLE `student` (
    `student_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '学生编号',
    `user_id` INT NOT NULL UNIQUE COMMENT '关联用户表ID',
    `school` VARCHAR(50) NOT NULL COMMENT '学校名称',
    `grade` INT NOT NULL COMMENT '年级',
    `student_no` VARCHAR(20) COMMENT '学号',
    `real_name` VARCHAR(20) NOT NULL COMMENT '真实姓名',
    `training_monitor` TEXT COMMENT '训练监控信息',
    FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE
) COMMENT '学生信息表';

CREATE TABLE `teacher` (
    `teacher_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '教师编号',
    `user_id` INT NOT NULL UNIQUE COMMENT '关联用户表ID',
    `school` VARCHAR(50) NOT NULL COMMENT '学校名称',
    `real_name` VARCHAR(20) NOT NULL COMMENT '真实姓名',
    `training_monitor` TEXT COMMENT '训练监控信息',
    FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE
) COMMENT '教师信息表';

CREATE TABLE `admin` (
    `admin_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '管理员编号',
    `user_id` INT NOT NULL UNIQUE COMMENT '关联用户表ID',
    FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE
) COMMENT '管理员信息表';

-- CREATE TABLE `study_group` (
--     `study_group_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '小组编号',
--     `group_name` VARCHAR(50) NOT NULL COMMENT '小组名称',
--     `group_desc` TEXT COMMENT '小组描述',
--     `create_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间'
-- ) COMMENT '学习小组表';

-- CREATE TABLE `student_study_group` (
--     `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '关联表编号',
--     `student_id` INT NOT NULL COMMENT '关联学生表ID',
--     `study_group_id` INT NOT NULL COMMENT '关联学习小组表ID',
--     UNIQUE KEY `uk_student_group` (`student_id`, `study_group_id`),
--     FOREIGN KEY (`student_id`) REFERENCES `student`(`student_id`) ON DELETE CASCADE,
--     FOREIGN KEY (`study_group_id`) REFERENCES `study_group`(`study_group_id`) ON DELETE CASCADE
-- ) COMMENT '学生-学习小组关联表';

-- CREATE TABLE `teacher_study_group` (
--     `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '关联表编号',
--     `teacher_id` INT NOT NULL COMMENT '关联教师表ID',
--     `study_group_id` INT NOT NULL COMMENT '关联学习小组表ID',
--     UNIQUE KEY `uk_teacher_group` (`teacher_id`, `study_group_id`),
--     FOREIGN KEY (`teacher_id`) REFERENCES `teacher`(`teacher_id`) ON DELETE CASCADE,
--     FOREIGN KEY (`study_group_id`) REFERENCES `study_group`(`study_group_id`) ON DELETE CASCADE
-- ) COMMENT '教师-学习小组关联表';

CREATE TABLE `question` ( 
    `question_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '题目编号', 
    `question_name` TEXT NOT NULL COMMENT '题目名称', 
    `create_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间', 
    `difficulty` TINYINT DEFAULT 1 COMMENT '题目难度(1-5)',
    `tags` JSON DEFAULT NULL COMMENT '题目标签',
    `is_audited` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '是否通过审核：0-等待审核，1-通过，2-未通过，3-隐藏'
) COMMENT '题目信息表';

CREATE TABLE `user_submission` (
  `submission_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '提交编号',
  `user_id` INT NOT NULL COMMENT '用户ID',
  `question_id` INT NOT NULL COMMENT '题目ID',
  `code` TEXT NOT NULL COMMENT '提交的代码',
  `language` VARCHAR(10) NOT NULL COMMENT '编程语言',
  `status` TINYINT NOT NULL COMMENT '提交状态：-1-当时未部署，0-编译错误，1-运行错误，2-超时，3-通过，4-答案错误',
  `score` INT DEFAULT 0 COMMENT '得分',
  `execution_time` INT DEFAULT 0 COMMENT '执行时间(ms)',
  `memory_used` INT DEFAULT 0 COMMENT '内存使用(KB)',
  `submit_at` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '提交时间',
  `is_audited` INT DEFAULT 0 NOT NULL COMMENT '0-普通提交，1-竞赛提交',
  FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE,
  FOREIGN KEY (`question_id`) REFERENCES `question`(`question_id`) ON DELETE CASCADE
) COMMENT='用户代码提交记录';

CREATE TABLE `user_question_status` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '记录编号',
  `user_id` INT NOT NULL COMMENT '用户ID',
  `question_id` INT NOT NULL COMMENT '题目ID',
  `status` TINYINT NOT NULL COMMENT '完成状态：0-未开始，1-未通过，2-已通过',
  `best_score` INT DEFAULT 0 COMMENT '最高得分',
  `completed_at` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '完成时间',
  UNIQUE KEY `uk_user_question` (`user_id`, `question_id`),
  FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE,
  FOREIGN KEY (`question_id`) REFERENCES `question`(`question_id`) ON DELETE CASCADE
) COMMENT='用户题目完成状态';

CREATE TABLE `discussion_group` (
  `group_id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '题目组ID',
  `group_name` VARCHAR(100) NOT NULL COMMENT '题目组名称',
  `group_desc` TEXT COMMENT '题目组描述',
  `start_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '开始答题时间',
  `end_time` DATETIME DEFAULT '9999-12-31 23:59:59' COMMENT '结束答题时间（默认无限期）',
  `create_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `creator_id` INT COMMENT '创建者用户ID',
  `is_competition` TINYINT NOT NULL DEFAULT 0 COMMENT '标记类型：0-训练，1-竞赛'
) COMMENT '题目组表';

CREATE TABLE `group_question` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '关联ID',
  `group_id` INT NOT NULL COMMENT '题目组ID',
  `question_id` INT NOT NULL COMMENT '题目ID',
  `order_num` INT DEFAULT 0 COMMENT '题目顺序',
  UNIQUE KEY `uk_group_question` (`group_id`, `question_id`),
  FOREIGN KEY (`group_id`) REFERENCES `discussion_group`(`group_id`) ON DELETE CASCADE,
  FOREIGN KEY (`question_id`) REFERENCES `question`(`question_id`) ON DELETE CASCADE
) COMMENT '题目组-题目关联表';

CREATE TABLE `user_group_status` (
  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '记录ID',
  `user_id` INT NOT NULL COMMENT '用户ID',
  `group_id` INT NOT NULL COMMENT '题目组ID',
  `status` TINYINT DEFAULT 0 COMMENT '状态：0-未开始，1-进行中，2-已完成',
  `start_time` DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '用户开始时间',
  `update_time` DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '记录更新时间',
  FOREIGN KEY (`user_id`) REFERENCES `user`(`user_id`) ON DELETE CASCADE,
  FOREIGN KEY (`group_id`) REFERENCES `discussion_group`(`group_id`) ON DELETE CASCADE
) COMMENT '用户-题目组状态表';

--CREATE TABLE `study_group_discussion` (
--  `id` INT PRIMARY KEY AUTO_INCREMENT COMMENT '关联ID',
--  `study_group_id` INT NOT NULL COMMENT '学习小组ID',
--  `discussion_group_id` INT NOT NULL COMMENT '题目组ID',
--  UNIQUE KEY `uk_study_discussion` (`study_group_id`, `discussion_group_id`),
--  FOREIGN KEY (`study_group_id`) REFERENCES `study_group`(`study_group_id`) ON DELETE CASCADE,
--  FOREIGN KEY (`discussion_group_id`) REFERENCES `discussion_group`(`group_id`) ON DELETE CASCADE
--) COMMENT '小组-题目组关联表';

SELECT '数据库创建完成，所有表创建成功！' AS '执行结果';

-- password: 123456    sha256: 8e4bb87e6fa3040477e5b8a9c69c98d0152b8c51e5a77a5a95267a9a790ea94d    salt: OStInz4eXFzd0zK5
-- password: 123456    sha256: 3a6058f92b334e652ff63250ef6afd8c71d461ab4329554cc2a35e24b215361f    salt: hcl4gVQU8YnHoEU1
-- password: 123123    sha256: 27f136a64a96bcb9cb9d119c0d6b808b80466c659aee1e0c77a92ef1e430377d    salt: vh7owoS5kKQNhUSK




SET GLOBAL event_scheduler = ON;

CREATE INDEX idx_user_submission_submit_at ON user_submission(submit_at);
CREATE EVENT `cleanup_user_submission`
ON SCHEDULE EVERY 1 HOUR
DO
BEGIN
    DECLARE delete_count INT;
    SELECT COUNT(*) - 1000000 INTO delete_count FROM user_submission;
    
    IF delete_count > 0 THEN
        DELETE FROM user_submission
        WHERE submission_id IN (
            SELECT submission_id FROM (
                SELECT submission_id FROM user_submission
                ORDER BY submit_at ASC
                LIMIT delete_count
            ) AS old_records
        );
    END IF;
END;

SELECT '自动清理事件创建完成，最多保留1000000条user_submission记录' AS '执行结果';
