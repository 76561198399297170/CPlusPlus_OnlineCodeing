#ifndef _UTILS_H_
#define _UTILS_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <limits>
#include <string>
#include <random>
#include <vector>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <fstream>
#include <iostream>

#include "utils/config.h"
#include "json.hpp"

const int SALT_LENGTH = 16;
const int TOKEN_LENGTH = 32;

extern const int TOKEN_VALID_TIME_SET;

inline std::u8string operator+(const char8_t* u8_prefix, const std::string& str)
{
	size_t prefix_len = std::strlen(reinterpret_cast<const char*>(u8_prefix));

	// GBK 到 UTF-8 转换
	int wide_len = MultiByteToWideChar(936, 0, str.c_str(), -1, nullptr, 0);
	wchar_t* wide_buf = new wchar_t[wide_len];
	MultiByteToWideChar(936, 0, str.c_str(), -1, wide_buf, wide_len);

	int utf8_len = WideCharToMultiByte(65001, 0, wide_buf, -1, nullptr, 0, nullptr, nullptr);
	char* utf8_buf = new char[utf8_len];
	WideCharToMultiByte(65001, 0, wide_buf, -1, utf8_buf, utf8_len, nullptr, nullptr);

	std::u8string result;
	result.reserve(prefix_len + utf8_len - 1);
	result.append(u8_prefix, prefix_len);
	result.append(reinterpret_cast<const char8_t*>(utf8_buf), utf8_len - 1);

	delete[] wide_buf;
	delete[] utf8_buf;

	return result;
}

//安全处理文件BOM，返回是否跳过了BOM
bool skipBOM(std::ifstream& file);

namespace sha256
{
	//生成盐值，随机16位字符串
	std::string generateSalt(int salt_length = SALT_LENGTH, int subhash = -2);

	//循环右移
	uint32_t rotateRight(uint32_t x, uint32_t n);

	//SHA256仿写核心压缩函数
	void sha256_transform(uint32_t state[8], const uint8_t block[64]);

	//仿写SHA256哈希
	std::vector<uint8_t> sha256(const std::string& input);

	//哈希加密密码，防止明文存储密码
	std::string hashPasswordBysha256(const std::string& password, const std::string& salt);
}

namespace hs256
{
	enum class ReturnType
	{
		TYPEERROR = 0,		 //格式错误
		VERIFYERROR = 1,	 //验证错误
		OVERDUEERROR = 2,	 //过期错误
		SUCCESS = 3			 //通过
	};

	//base64url编码
	std::string encodeBase64URL(const std::string& input);

	//base64url解码
	std::string decodeBase64URL(const std::string& input);

	//hmac-sha256签名
	std::vector<uint8_t> hmac_sha256(const std::string& data, const std::string& secret);

	//生成hs256签名的Token
	std::string generateToken(const std::string& secret, int64_t user_id, const std::string& role, int expire_seconds = TOKEN_VALID_TIME_SET);

	//验证hs256Token
	hs256::ReturnType verifyToken(const std::string& token, const std::string& secret, int64_t& out_user_id, std::string& out_role, int* time_left = nullptr);
}

namespace util
{
	//枚举输入类型
	enum class InputType;

	//未实现: 检查输入是否有效
	template<typename T>
	bool validateInput(T value, InputType type);

	//获取随机数，范围[min, max]
	int generateRandomInt(int min = 0, int max = LLONG_MAX);

	//分隔题目文本
	void splitQuestionContent(const std::string content, std::string& question_desc, std::string& input_format, std::string& output_format, std::string& sample_input);

	//gbk转utf8
	std::string gbk_to_utf8(const std::string& gbk_str);

	bool isNowAfter(const std::string& s);

	//UTF-8转GBK编码
	//std::string utf8_to_gbk(const std::string& utf8_str);
}

#endif // !_UTILS_H_
