//注释隐藏以便于修改example中的main进行启动
#define _FLAG_TEST_ true

#if _FLAG_TEST_

#include <iostream>
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>

#include "httplib.h"

#include "database/connection_pool.h"
#include "runtime/code_pool.h"
#include "runtime/language_config.h"
#include "agent/agent_pool.h"
#include "routes/handlers.h"

#include "agent/model_access.h"
#include "runtime/program_access.h"
#include "utils/security.h"

//是否添加了GCC为编译语言
#define _ADD_GCC_ true;

void myResponseSetHeader(const httplib::Request& req, httplib::Response& res, int methods_type)
{
	res.set_header("Access-Control-Allow-Origin", "*");
	switch (methods_type)
	{
	case 0:
		res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
		break;
	case 1:
		res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
		break;
	case 2:
		res.set_header("Access-Control-Allow-Methods", "PUT, OPTIONS");
		break;
	case 3:
		res.set_header("Access-Control-Allow-Methods", "DELETE, OPTIONS");
	}
	res.set_header("Access-Control-Allow-Headers", "Content-Type");
	res.set_header("Content-Type", "application/json; charset=utf-8");
	res.status = 200;
}

int main()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	try
	{
		// 初始化语言管理器
		LanguageManager::getInstance()->init();
		std::cout << "LanguageManager init success." << std::endl;

		ConnectionPool::getInstance()->init(
			{
				.host = "127.0.0.1",
				.port = "3306",
				.username = "root",
				.password = "Yy220311",
				.dbname = "ocdb"
			});
		std::cout << "ConnectionPool init success." << std::endl;

		CodePool::getInstance()->init(
			{
				.min_size = 5,
				.max_size = 20,
				.timeout_ms = 12500,
				.compile_limit_ms = 3000
			});
		std::cout << "CodePool init success." << std::endl;

		AgentPool::getInstance()->init(
			{
				.ai_host = "172.23.207.64",
				.ai_port = 8000,
				.min_size = 5,
				.max_size = 20,
				.timeout_ms = 12500
			});
		std::cout << "AgentPool init success." << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "main function init failed, " << e.what() << std::endl;
		return -1;
	}

	// GCC预热
#if _ADD_GCC_
	std::cout << "[info]main: Warming up GCC compiler..." << std::endl;

	//预热GCC
	std::string warmup_code = "#include <iostream>\nint main() { return 0; }";
	std::vector<std::vector<std::string>> test_inputs = { {""} };
	std::vector<std::vector<std::string>> test_outputs = { {""} };

	ProgramAccess::ProgramLimits limits;
	limits.lang = LangType::CPP;
	limits.time_limit_ms = 1000;
	limits.memory_limit_mb = 64;

	ProgramAccess::ProgramStatus status = ProgramAccess::getInstance()->runProgram(limits, warmup_code, test_inputs, test_outputs);

	//无论编译是否成功，GCC也完成调用达到预热目的
	if (status.status == "AC" || status.status == "CE")
	{
		std::cout << "[info]main: GCC warmup completed." << std::endl;
	}
	else
	{
		std::cerr << "[warn]main: GCC warmup failed: " << status.error_message << std::endl;
	}

#endif

	httplib::Server svr;
	svr.set_logger([](const auto& req, const auto& res)
		{
			std::cout << "[http] " << req.method << " " << req.path
				<< " | Status: " << res.status
				<< " | Remote: " << req.remote_addr << std::endl;
		});

	//注册提交请求
	svr.Post("/register", handleRegister);
	svr.Options("/register", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//登录提交请求
	svr.Post("/login", handleLogin);
	svr.Options("/login", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//代码结果提交请求
	svr.Post("/submissions", handleSubmitCode);
	svr.Options("/submissions", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//代码测试请求
	svr.Post("/run-test", handleRunTese);
	svr.Options("/run-test", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//创建题目请求
	svr.Post("/problems/create", handleCreateProblem);
	svr.Options("/problems/create", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//审核题目通过
	svr.Post(R"(/problems/review/(\d+)/approve)", handleProblemApprove);
	svr.Options(R"(/problems/review/(\d+)/approve)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//审核题目驳回
	svr.Post(R"(/problems/review/(\d+)/reject)", handleProblemReject);
	svr.Options(R"(/problems/review/(\d+)/reject)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//修改题目隐藏
	svr.Post(R"(/problems/(\d+)/hide)", handleProblemHide);
	svr.Options(R"(/problems/(\d+)/hide)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//统一搜索题目
	svr.Post(R"(/problems/search)", handleSearchProblem);
	svr.Options(R"(/problems/search)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//获取 AI 帮助
	svr.Post(R"(/ai/help)", handleAgentHelp);
	svr.Options(R"(/ai/help)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});
	//创建题组
	svr.Post(R"(/training/create)", handleCreateDiscussionGroup);
	svr.Options(R"(/training/create)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 0);
		});


	//题目列表获取请求
	svr.Get(R"(/problems)", handleGetProblemList);
	svr.Options(R"(/problems)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//隐藏题目列表获取请求
	svr.Get(R"(/problems/hidden)", handleGetHideProblemList);
	svr.Options(R"(/problems/hidden)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//题目获取请求
	svr.Get(R"(/problems/(\d+))", handleGetProblem);
	svr.Options(R"(/problems/(\d+))", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//平台统计数据获取请求
	svr.Get(R"(/stats)", handleGetQuestionStats);
	svr.Options(R"(/stats)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//未审核题目列表获取请求
	svr.Get(R"(/problems/review)", handleGetAuditedList);
	svr.Options(R"(/problems/review)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//驳回题目列表获取请求
	svr.Get(R"(/problems/rejected)", handleGetRejectedList);
	svr.Options(R"(/problems/rejected)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//用户提交记录列表获取请求
	svr.Get(R"(/submissions)", handleGetSubmissionList);
	svr.Options(R"(/submissions)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取所有用户列表（仅管理员
	svr.Get(R"(/users)", handleGetUserList);
	svr.Options(R"(/users)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取指定用户信息
	svr.Get(R"(/user/profile/(\d+))", handleGetUser);
	svr.Options(R"(/user/profile/(\d+))", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取当前用户信息
	svr.Get(R"(/user/info)", handleGetUserSelf);
	svr.Options(R"(/user/info)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取当前用户隐私设置
	svr.Get(R"(/user/privacy)", handleGetProvacy);
	svr.Options(R"(/user/privacy)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取题目训练概览
	svr.Get(R"(/training/overview)", handleGetTrainingStats);
	svr.Options(R"(/training/overview)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取用户正在进行中的题目组
	svr.Get(R"(/training/active-sets)", handleGetTrainingActive);
	svr.Options(R"(/training/active-sets)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取题目组列表
	svr.Get(R"(/training/recommended-sets)", handleGetQuestionGroup);
	svr.Options(R"(/training/recommended-sets)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取历史题目组
	svr.Get(R"(/training/history)", handleGetGroupHistory);
	svr.Options(R"(/training/history)", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取题组详情
	svr.Get(R"(/training/set/(\d+))", handleGetGroupInfo);
	svr.Options(R"(/training/set/(\d+))", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});
	//获取竞赛提交记录
	svr.Get(R"(/submissions/stats/(\d+))", handleGetStats);
	svr.Options(R"(/submissions/stats/(\d+))", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 1);
		});


	//更新用户昵称
	svr.Put("/user/nickname", handleSetNickname);
	svr.Options("/user/nickname", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 2);
		});
	//更新用户密码
	svr.Put("/user/password", handleSetPassword);
	svr.Options("/user/password", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 2);
		});
	//更新用户邮箱
	svr.Put("/user/email", handleSetEmail);
	svr.Options("/user/email", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 2);
		});
	//更新隐私设置数据
	svr.Put("/user/privacy", handleSetProvacy);
	svr.Options("/user/privacy", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 2);
		});


	//删除题目组
	svr.Delete(R"(/training/(\d+))", handleDeleteDiscussion);
	svr.Options(R"(/training/(\d+))", [](const httplib::Request& req, httplib::Response& res)
		{
			myResponseSetHeader(req, res, 3);
		});

	std::cout << "server start, listen: http://0.0.0.0:8080/" << std::endl;
	std::cout << "====================================================================================================" << std::endl;
	svr.listen("0.0.0.0", 8080);

	return 0;
}

#endif
