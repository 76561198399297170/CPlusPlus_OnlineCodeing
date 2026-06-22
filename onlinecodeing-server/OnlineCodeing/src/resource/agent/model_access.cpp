#include "agent/model_access.h"

#include <algorithm>
#include <fstream>

#include "utils/security.h"

extern const std::string PROMPTS_FILE_PATH;
extern const bool IS_STRUCTURED_OUTPUT;

ModelAccess::ReturnMessage ModelAccess::agentAdditionalProblem(Problem& problem, const std::string& tool_id, std::string context)
{
	int is_empty = 0;
	if (problem.title.empty()) is_empty++;
	if (problem.difficulty == -1) is_empty++;
	if (problem.tags.empty()) is_empty++;
	if (problem.problem_content.empty()) is_empty++;
	if (problem.input_content.empty()) is_empty++;
	if (problem.output_content.empty()) is_empty++;
	if (problem.test_case.empty()) is_empty++;

	if (is_empty == 7)
	{
		std::cerr << "[warn]ModelAccess::agentAdditionalProblem: base problem is empty." << std::endl;
	}

	std::unique_ptr<PooledAgent> pooled_agent = AgentPool::getInstance()->getExecutor();
	if (!pooled_agent)
	{
		std::cerr << "[warn]ModelAccess::agentAdditionalProblem: get connection from pool failed." << std::endl;
		return
		{
			.isSuccess = false,
			.Messages = "system: get connection from pool failed."
		};
	}

	std::string problem_str = problemToString(problem);
	std::string result;
	for (int i = 0; i < problem_str.size(); i++)
	{
		if (problem_str[i] == '\\' && i + 1 < problem_str.size() && problem_str[i + 1] == 't')
		{
			i++;
			continue;
		}
		result += problem_str[i];
	}
	problem_str = std::move(result);
	nlohmann::json problem_json = nlohmann::json::parse(problem_str);

	Agent* executor = pooled_agent->get();

	executor->clearHistory(context);
	std::cout << "[info]ModelAccess::agentAdditionalProblem: use tool create, id " << tool_id << std::endl;
	nlohmann::json assistant = executor->chatCompletionsByJson(problem_json, this->m_prompt["create"][tool_id]);

	this->addAssistantProblem(problem, assistant, tool_id);

	bool isv = false;
	std::string msg;
	if (assistant.contains("success") && !assistant["success"].empty())
		isv = assistant["success"].get<bool>();
	if (assistant.contains("message") && !assistant["message"].empty())
		msg = assistant["message"].get<std::string>();

	return
	{
		.isSuccess = isv,
		.Messages = msg
	};
}

ModelAccess::ReturnMessage ModelAccess::agentReviewProblem(Problem& problem, const std::string& tool_id, std::string context)
{
	int is_empty = 0;
	if (problem.title.empty()) is_empty++;
	if (problem.difficulty == -1) is_empty++;
	if (problem.tags.empty()) is_empty++;
	if (problem.problem_content.empty()) is_empty++;
	if (problem.input_content.empty()) is_empty++;
	if (problem.output_content.empty()) is_empty++;
	if (problem.test_case.empty()) is_empty++;

	if (is_empty == 7)
	{
		std::cerr << "[error]ModelAccess::agentReviewProblem: base problem is empty." << std::endl;
		return
		{
			.isSuccess = false,
			.Messages = "system: question is null."
		};
	}

	std::unique_ptr<PooledAgent> pooled_agent = AgentPool::getInstance()->getExecutor();
	if (!pooled_agent)
	{
		std::cerr << "[warn]ModelAccess::agentReviewProblem: get connection from pool failed." << std::endl;
		return
		{
			.isSuccess = false,
			.Messages = "system: get connection from pool failed."
		};
	}

	std::string problem_str = problemToString(problem);
	std::string result;
	for (int i = 0; i < problem_str.size(); i++)
	{
		if (problem_str[i] == '\\' && i + 1 < problem_str.size() && problem_str[i + 1] == 't')
		{
			i++;
			continue;
		}
		result += problem_str[i];
	}
	problem_str = std::move(result);
	nlohmann::json problem_json = nlohmann::json::parse(problem_str);

	Agent* executor = pooled_agent->get();

	executor->clearHistory(context);
	std::cout << "[info]ModelAccess::agentReviewProblem: use tool review, id " << tool_id << std::endl;
	nlohmann::json assistant = executor->chatCompletionsByJson(problem_json, this->m_prompt["review"][tool_id]);

	bool isv = false;
	std::string msg;
	if (assistant.contains("success") && assistant["success"].is_boolean())
		isv = assistant["success"].get<bool>();
	if (assistant.contains("message") && !assistant["message"].empty())
		msg = assistant["message"].get<std::string>();

	return
	{
		.isSuccess = isv,
		.Messages = msg
	};
}

ModelAccess::ModelAccess()
{
	std::ifstream ifs(PROMPTS_FILE_PATH, std::ios::binary);
	skipBOM(ifs);
	this->m_prompt = nlohmann::json::parse(ifs);
	ifs.close();

	return;
}

void ModelAccess::addAssistantProblem(Problem& problem, nlohmann::json assistant,std::string tool_id)
{
	if (IS_STRUCTURED_OUTPUT)
	{
		nlohmann::json content = assistant;
		std::cout << "[info]ModelAccess::addAssistantProblem: content: " << content.dump() << std::endl;

		if (content.contains("title") && !content["title"].empty())
			problem.title = content["title"];

		if (content.contains("difficulty") && content["difficulty"].is_number_integer())
			problem.difficulty = content["difficulty"];

		if (content.contains("problem_content") && !content["problem_content"].empty())
			problem.problem_content = content["problem_content"];

		if (content.contains("input_content") && !content["input_content"].empty())
			problem.input_content = content["input_content"];

		if (content.contains("output_content") && !content["output_content"].empty())
			problem.output_content = content["output_content"];
		
		if (content.contains("tags") && content["tags"].is_array())
			problem.tags = content["tags"].get<std::vector<std::string>>();

		std::vector<std::pair<std::string, std::string>> test_case;
		if (content.contains("test_cases") && content["test_cases"].is_array())
		{
			for (const auto& t : content["test_cases"])
			{
				if (t.contains("input") && t.contains("output"))
				{
					std::string input = t["input"].get<std::string>();
					std::string output = t["output"].get<std::string>();

					if (!input.empty() && !output.empty())
					{
						test_case.emplace_back(input, output);
					}
				}
			}
			problem.test_case = test_case;
		}
	}
	else
	{
		//不使用结构化输出，需要手动进行解析内容，建议使用结构化否则可能出现预期外的错误，且标签变更需要同步此处
		//所以对于后续更新此处暂不更新维护
		std::string content = assistant;
		std::cout << "[info]ModelAccess::addAssistantProblem: content: " << content << std::endl;

		if (tool_id == "create-generate-title")
		{
			problem.title = content;
		}
		else if (tool_id == "create-generate-description")
		{
			problem.problem_content = content;
		}
		else if (tool_id == "create-generate-test-cases")
		{
			std::vector<std::pair<std::string, std::string>> test_case;
			nlohmann::json tsc_json = nlohmann::json::parse(content);

			for (const nlohmann::json& t : content)
			{
				test_case.emplace_back(t["input"].get<std::string>(), t["output"].get<std::string>());
			}
			problem.test_case = test_case;
		}
		else if (tool_id == "create-optimize-difficulty")
		{
			problem.difficulty = std::stoi(content);
		}
		else if (tool_id == "create-add-tags")
		{
			std::vector<std::string> tags;
			std::string::iterator bg = content.begin();
			std::string::iterator ed = ++(content.begin());
			while (ed != content.end())
			{
				if (*(ed) == ',' || *(ed) == '\n' || *(ed) == ';' || *(ed) == ':')
				{
					tags.emplace_back(content.substr(bg - content.begin(), ed - bg));
					bg = ed + 1;
				}
				ed++;
			}
			tags.emplace_back(content.substr(bg - content.begin(), ed - bg));

			problem.tags = tags;
		}
	}
}
