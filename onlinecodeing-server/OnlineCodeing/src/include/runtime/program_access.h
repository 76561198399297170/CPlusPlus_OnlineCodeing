#ifndef _PROGRAM_ACCESS_H_
#define _PROGRAM_ACCESS_H_

#include "utils/singleton.h"
#include "runtime/code_pool.h"

class ProgramAccess : public Singleton<ProgramAccess>
{
	friend class Singleton<ProgramAccess>;

public:
	class ProgramLimits
	{
	public:
		LangType lang = LangType::UNKNOW;	//语言类型
		int time_limit_ms = -1;				//时间限制
		int memory_limit_mb = -1;			//空间限制
	};

	class ProgramStatus
	{
	public:
		std::string status = "";
		std::string error_message = "";
		long long run_time_ms = 0;
		long long code_size_bytes = 0;
		std::vector<std::string> test_outputs;
	};

public:
	ProgramAccess::ProgramStatus runProgram(const ProgramAccess::ProgramLimits& limits, const std::string& code,
		const std::vector<std::vector<std::string>>& test_case_inputs, const std::vector<std::vector<std::string>>& test_case_outputs);


private:
	//私有构造
	ProgramAccess() = default;

	//禁用拷贝与移动
	ProgramAccess(const ProgramAccess&) = delete;
	ProgramAccess& operator=(const ProgramAccess&) = delete;
	ProgramAccess(ProgramAccess&&) = delete;
	ProgramAccess& operator=(ProgramAccess&&) = delete;

};

#endif // !_PROGRAM_ACCESS_H_
