#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include <string>
#include <vector>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <thread>
#include <algorithm>

enum class LangType
{
	C,				//C
	CPP,			//C++
	PYTHON3,
	PYTHON2,
	JAVA,
	PYPY3,
	GO,
	JAVASCRIPT,
	UNKNOW			//未知类型&计数标记
};

enum class CodeResult
{
	CODE_AC,        //全部通过
	CODE_WA,        //答案错误
	CODE_CE,        //编译错误
	CODE_TLE,       //运行超时
	CODE_ERROR,     //运行错误
	UNKNOW			//未知原因
};

//字符串描述语言类型转自定义枚举语言类型
LangType codeStringToLang(std::string lang);

//自定义枚举语言类型转字符串描述语言类型
std::string codeLangToString(LangType lang);

//代码结果描述拆分结果
CodeResult codeStringFindCodeResult(std::string str, std::string& error_msg);

//整形描述代码结果转自定义枚举代码结果
CodeResult codeIntToCodeResult(int i);

//字符串描述结果转整形
int codeStringToInt(std::string result);

//自定义枚举代码结果转字符串
std::string codeResultToString(CodeResult type);

//整形难度描述转字符串描述难度
std::string codeIntToStringDifficulty(int difficulty);

class CodeExecutor
{
public:
	//构造与析构
	CodeExecutor();
	~CodeExecutor();

	//代码执行
	std::string evaluateCode(LangType lang, const std::string& code,
		const std::vector<std::vector<std::string>>& test_case_inputs,
		const std::vector<std::vector<std::string>>& test_case_outputs);

	//运行时间限制
	void setTimeLimit(int ms) { this->m_time_limit_ms = ms; }
	//编译时间限制
	void setCompileTimeLimit(int ms) { this->m_compile_time_limit_ms = ms; }
	//内存大小限制
	void setMemoryLimit(size_t mb) { this->m_memory_limit_mb = mb; }
	//编译文件临时根目录
	void setTempDir(std::string dir) { this->m_temp_dir = dir; }

	//获取运行时间
	long long getRunTimeMs() const { return m_run_time_ms; }
	//获取代码大小
	long long getCodeSizeBytes() const { return m_code_size_bytes; }
	//获取测试用例输出
	const std::vector<std::string>& getTestOutputs() const { return m_test_outputs; }

private:
	//创建临时文件夹
	std::string createTempDir();
	//代码安全处理
	void sanitizeCode(std::string& code, LangType lang);
	//生成代码文件及命令
	std::string generateCodeCommands(LangType lang, const std::string& code, const std::filesystem::path& temp_dir, std::filesystem::path& code_file, std::string& run_cmd, std::string& compile_cmd);
	//运行编译工作
	void runCompile(LangType lang, const std::string& compile_cmd);
	//安全关闭句柄
	void closeHandle(HANDLE& h);
	//设置句柄非继承
	void setHandleNonInheritable(HANDLE& h);
	//读取管道数据
	std::string readPipeNonBlocking(HANDLE hRead, int timeout_ms = 1000);
	//运行代码程序
	std::string runTests(LangType lang, const std::string& run_cmd,
		HANDLE hJob, const std::filesystem::path& temp_dir,
		const std::vector<std::vector<std::string>>& test_case_input,
		const std::vector<std::vector<std::string>>& test_case_output);
	//清理临时文件夹
	void cleanupTempDir(const std::filesystem::path& path);

private:
	int m_time_limit_ms;			//运行时间限制
	int m_compile_time_limit_ms;	//编译时间限制
	size_t m_memory_limit_mb;		//文件大小限制

	std::string m_temp_dir;							//临时文件夹根目录
	long long m_run_time_ms;						//运行时间(毫秒)
	long long m_code_size_bytes;					//代码大小(字节)
	std::vector<std::string> m_test_outputs;		//测试用例输出

};

#endif // !_EXECUTOR_H_
