#ifndef _AGENT_H_
#define _AGENT_H_

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "json.hpp"

namespace httplib
{
    class Client;
}

struct Problem
{
	std::string title;												//题目
	int difficulty = -1;											//难度
	std::vector<std::string> tags;									//题目标签
	std::string problem_content;									//问题描述
	std::string input_content;										//输入描述
	std::string output_content;										//输出描述
	std::vector<std::pair<std::string, std::string>> test_case;		//测试用例

};

//问题文件转json格式再dump
std::string problemToString(const Problem& question);

Problem problemJsonToProblem(nlohmann::json question);

class Agent
{
public:
	//构造与析构
	Agent();
	Agent(std::string ai_host, int ai_port);
	~Agent();

	//重置清除历史记录并设定初始对话
	void clearHistory(std::string addcontext = "");

	//聊天补全 向模型发送聊天历史以预测下一个助手响应
	std::string chatCompletionsByString(std::string content);

	//聊天补全 向模型发送聊天历史以预测下一个助手响应
	nlohmann::json chatCompletionsByJson(nlohmann::json content, std::string format = "");

private:
	const char* AI_PATH = "/v1/chat/completions";
	const int AI_TIMEOUT = 25;

	std::string AI_HOST = "172.23.207.64";
	int AI_PORT = 8000;

	std::unique_ptr<httplib::Client> m_client;

private:
	std::vector<nlohmann::json> m_messages;//历史对话记录
	std::string m_model = "liquid/lfm2.5-1.2b";//模型

};

#endif // !_AGENT_H_
