#include "agent/agent.h"

#include "httplib.h"
#include "json.hpp"

#include "utils/security.h"

Agent::Agent()
{
	this->m_client = std::make_unique<httplib::Client>(AI_HOST, AI_PORT);
	this->m_client->set_read_timeout(AI_TIMEOUT);
	this->m_client->set_write_timeout(AI_TIMEOUT);
	this->m_client->set_connection_timeout(AI_TIMEOUT);

	nlohmann::json system_msg;
	system_msg["role"] = "system";
	system_msg["content"] = "Please solve the questions I am about to raise from the perspective of a professional programmer.";//"Please reply in UTF8 and use English punctuation.";
	this->m_messages.emplace_back(system_msg);
}

Agent::Agent(std::string ai_host, int ai_port)
{
	this->AI_HOST = ai_host;
	this->AI_PORT = ai_port;

	this->m_client = std::make_unique<httplib::Client>(AI_HOST, AI_PORT);
	this->m_client->set_read_timeout(AI_TIMEOUT);
	this->m_client->set_write_timeout(AI_TIMEOUT);
	this->m_client->set_connection_timeout(AI_TIMEOUT);

	nlohmann::json system_msg;
	system_msg["role"] = "system";
	system_msg["content"] = "Please use UTF-8 Chinese. Please solve the questions I am about to raise from the perspective of a professional programmer.";//"Please reply in UTF8 and use English punctuation.";
	this->m_messages.emplace_back(system_msg);

}

Agent::~Agent()
{}

void Agent::clearHistory(std::string addcontext)
{
	this->m_messages.clear();

	nlohmann::json system_msg;
	system_msg["role"] = "system";
	system_msg["content"] = addcontext + ". Please use UTF-8 Chinese. Please solve the questions I am about to raise from the perspective of a professional programmer.";
	this->m_messages.emplace_back(system_msg);
}

std::string problemToString(const Problem& question)
{
	nlohmann::json question_json;

	question_json["title"] = question.title;
	question_json["difficulty"] = question.difficulty;
	question_json["tags"] = question.tags;
	question_json["problem_content"] = question.problem_content;
	question_json["input_content"] = question.input_content;
	question_json["output_content"] = question.output_content;
	question_json["test_case"] = question.test_case;

	return question_json.dump();
}

Problem problemJsonToProblem(nlohmann::json question)
{
	std::string title = "";
	int difficulty = 1;
	std::vector<std::string> tags = {};
	std::string problem_content = "";
	std::string input_content = "";
	std::string output_content = "";
	std::vector<std::pair<std::string, std::string>> test_case = {};
	if (question.contains("title")) title = question["title"];
	if (question.contains("difficulty")) difficulty = question["difficulty"];
	if (question.contains("tags")) tags = question["tags"];
	if (question.contains("problem_content")) problem_content = question["problem_content"];
	if (question.contains("input_content")) input_content = question["input_content"];
	if (question.contains("output_content")) output_content = question["output_content"];
	if (question.contains("test_case")) test_case = question["test_case"];

	return Problem({
			.title = title,
			.difficulty = difficulty,
			.tags = tags,
			.problem_content = problem_content,
			.input_content = input_content,
			.output_content = output_content,
			.test_case = test_case
		});
}

nlohmann::json Agent::chatCompletionsByJson(nlohmann::json content, std::string format)
{
	std::string prompt = format;
	std::string json_str = content.dump();
	std::string final_content = prompt + "\n" + json_str;

	nlohmann::json user_msg;
	user_msg["role"] = "user";
	user_msg["content"] = final_content;
	this->m_messages.emplace_back(user_msg);

	nlohmann::json root;
	root["model"] = this->m_model;
	root["messages"] = this->m_messages;
	root["temperature"] = 0.1;
	root["max_tokens"] = 5120;
	root["stream"] = false;

	std::cout << "[info]Agent::chatCompletions: content: \n"
		<< format << ": \n" << content.dump() << "\n" << std::endl;

	try
	{
		std::string post_data = root.dump();
		std::cout << "[info]Agent::chatCompletions: post_data: \n" << post_data << std::endl;

		httplib::Result res = this->m_client->Post(AI_PATH, post_data, "application/json");
		std::cout << "[info]Agent::chatCompletions: res body: \n" << (res ? res->body : "nullptr") << std::endl;

		if (res && res->status == 200)
		{
			nlohmann::json response_json = nlohmann::json::parse(res->body);
			std::string assistant_content = response_json["choices"][0]["message"]["content"];

			nlohmann::json assistant_msg;
			assistant_msg["role"] = "assistant";
			assistant_msg["content"] = assistant_content;
			this->m_messages.push_back(assistant_msg);

			return nlohmann::json::parse(assistant_content);
		}
		else
		{
			std::cerr << "[warn]Agent::chatCompletion: " << (res ? std::to_string(res->status) : "no response") << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "[error]Agent::chatCompletions: " << e.what() << std::endl;
	}

	return nlohmann::json();
}
