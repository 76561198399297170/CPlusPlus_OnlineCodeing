#ifndef _MODEL_ACCESS_H_
#define _MODEL_ACCESS_H_

#include <iostream>
#include <vector>
#include <utility>

#include "agent/agent_pool.h"
#include "utils/singleton.h"

class ModelAccess : public Singleton<ModelAccess>
{
	friend class Singleton<ModelAccess>;

public:

	struct ReturnMessage
	{
		bool isSuccess = false;
		std::string Messages = "";
	};

	//智能体补全题目内容
	ModelAccess::ReturnMessage agentAdditionalProblem(Problem& problem, const std::string& tool_id, std::string context = "");

	//智能体审查题目内容
	ModelAccess::ReturnMessage agentReviewProblem(Problem& problem, const std::string& tool_id, std::string context = "");

private:
	//私有构造
	ModelAccess();

    //ai回复添加到Problem中
    void addAssistantProblem(Problem& problem, nlohmann::json assistant, std::string tool_id);

private:
	nlohmann::json m_prompt;//工具提示词

};

#endif // !_MODEL_ACCESS_H_

