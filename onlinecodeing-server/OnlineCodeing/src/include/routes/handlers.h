#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "httplib.h"
#include "json.hpp"

//====================
// POST请求
//====================
//用户注册
void handleRegister(const httplib::Request& req, httplib::Response& res);

//用户登录
void handleLogin(const httplib::Request& req, httplib::Response& res);

//答题测试
void handleRunTese(const httplib::Request& req, httplib::Response& res);

//答题提交
void handleSubmitCode(const httplib::Request& req, httplib::Response& res);

//创建题目
void handleCreateProblem(const httplib::Request& req, httplib::Response& res);

//题目审核通过
void handleProblemApprove(const httplib::Request& req, httplib::Response& res);

//题目审核驳回
void handleProblemReject(const httplib::Request& req, httplib::Response& res);

//题目审核驳回
void handleProblemHide(const httplib::Request& req, httplib::Response& res);

//统一搜索题目
void handleSearchProblem(const httplib::Request& req, httplib::Response& res);

//获取 AI 帮助
void handleAgentHelp(const httplib::Request& req, httplib::Response& res);

//创建题组
void handleCreateDiscussionGroup(const httplib::Request& req, httplib::Response& res);

//====================
// GET请求
//====================
//请求题目列表
void handleGetProblemList(const httplib::Request& req, httplib::Response& res);

//请求隐藏题目列表
void handleGetHideProblemList(const httplib::Request& req, httplib::Response& res);

//请求题目
void handleGetProblem(const httplib::Request& req, httplib::Response& res);

//请求后端统计
void handleGetQuestionStats(const httplib::Request& req, httplib::Response& res);

//请求未审核题目
void handleGetAuditedList(const httplib::Request& req, httplib::Response& res);

//请求驳回题目
void handleGetRejectedList(const httplib::Request& req, httplib::Response& res);

//请求所有提交记录
void handleGetSubmissionList(const httplib::Request& req, httplib::Response& res);

//获取用户列表
void handleGetUserList(const httplib::Request& req, httplib::Response& res);

//获取指定用户
void handleGetUser(const httplib::Request& req, httplib::Response& res);

//获取当前用户
void handleGetUserSelf(const httplib::Request& req, httplib::Response& res);

//用户获取隐私设置
void handleGetProvacy(const httplib::Request& req, httplib::Response& res);

//获取题目训练概览
void handleGetTrainingStats(const httplib::Request& req, httplib::Response& res);

//获取进行中的题目组
void handleGetTrainingActive(const httplib::Request& req, httplib::Response& res);

//获取题目组
void handleGetQuestionGroup(const httplib::Request& req, httplib::Response& res);

//获取历史题目组
void handleGetGroupHistory(const httplib::Request& req, httplib::Response& res);

//获取题组内容
void handleGetGroupInfo(const httplib::Request& req, httplib::Response& res);

//获取竞赛提交记录
void handleGetStats(const httplib::Request& req, httplib::Response& res);

//====================
// PUT请求
//====================
//用户更新昵称
void handleSetNickname(const httplib::Request& req, httplib::Response& res);

//用户更新密码
void handleSetPassword(const httplib::Request& req, httplib::Response& res);

//用户更新密码
void handleSetEmail(const httplib::Request& req, httplib::Response& res);

//用户更新隐私设置
void handleSetProvacy(const httplib::Request& req, httplib::Response& res);

//====================
// DELETE请求
//====================
//删除题目组/竞赛题组
void handleDeleteDiscussion(const httplib::Request& req, httplib::Response& res);

#endif // !_HANDLERS_H_