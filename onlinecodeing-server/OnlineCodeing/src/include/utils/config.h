#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>

//测试提交代码存储位置
const std::string SECURE_SECRET_KEY = "my_temp_secret_key";
const std::string PROBLEM_PATH = "C:\\Users\\Administrator\\Desktop\\OnlineCodeing\\onlinecodeing-server\\OnlineCodeing\\res\\problems";
const std::string TESTPOINTS_PATH = "C:\\Users\\Administrator\\Desktop\\OnlineCodeing\\onlinecodeing-server\\OnlineCodeing\\res\\testpoints";

//ai 工具提示词
const std::string PROMPTS_FILE_PATH = "C:\\Users\\Administrator\\Desktop\\OnlineCodeing\\onlinecodeing-server\\OnlineCodeing\\src\\resource\\agent\\prompts.json";
const bool IS_STRUCTURED_OUTPUT = true;
/*
//AI 结构化输出JSON提示内容
{
  "type": "object",
  "required": [
    "title",
    "difficulty",
    "tags",
    "problem_content",
    "input_content",
    "output_content",
    "test_cases",
    "success",
    "message"
  ],
  "properties": {
    "title": {
      "description": "The title of the programming algorithm question",
      "type": "string"
    },
    "difficulty": {
      "description": "Difficulty level of the programming question",
      "type": "integer",
      "minimum": 1,
      "maximum": 5
    },
    "tags": {
      "description": "List of tags for the algorithm question",
      "type": "array",
      "items": {
        "type": "string"
      }
    },
    "problem_content": {
      "description": "Detailed content and description of the problem",
      "type": "string"
    },
    "input_content": {
      "description": "Input format and requirements of the problem",
      "type": "string"
    },
    "output_content": {
      "description": "Output format and requirements of the problem",
      "type": "string"
    },
    "test_cases": {
      "description": "Test cases containing input and output pairs",
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "input": {
            "type": "string"
          },
          "output": {
            "type": "string"
          }
        },
        "required": [
          "input",
          "output"
        ]
      }
    },
    "success": {
      "type": "boolean",
      "description": "True for check passed, False for failed"
    },
    "message": {
      "type": "string",
      "description": "Review result, conclusion or suggestions"
    }
  }
}
*/

//Token相关设置
const int TOKEN_VALID_TIME_SET = 7200;						//Token设定有效时长（喵）
const double TOKEN_REGENERATE_PERCENT = 0.5f;				//Token重新生成阈值（目前仅应用在/user/info请求中进行重生成判断）

#endif // !_CONFIG_H_
