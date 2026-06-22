#include "runtime/language_config.h"
#include "json.hpp"
#include <fstream>

//LanguageConfig 构造函数
LanguageConfig::LanguageConfig(LangType type, const std::string& name, const std::string& file_extension,
    const std::string& compile_cmd, const std::string& run_cmd, bool need_compile)
    : m_type(type), m_name(name), m_file_extension(file_extension),
    m_compile_cmd(compile_cmd), m_run_cmd(run_cmd), m_need_compile(need_compile)
{}

//LanguageManager 单例实例
static LanguageManager* s_instance = nullptr;

//从字符串获取 LangType
LangType stringToLangType(const std::string& type_str)
{
    if (type_str == "C")
    {
        return LangType::C;
    }
    if (type_str == "CPP")
    {
        return LangType::CPP;
    }
    if (type_str == "PYTHON3")
    {
        return LangType::PYTHON3;
    }
    if (type_str == "PYTHON2")
    {
        return LangType::PYTHON2;
    }
    if (type_str == "JAVA")
    {
        return LangType::JAVA;
    }
    if (type_str == "PYPY3")
    {
        return LangType::PYPY3;
    }
    if (type_str == "GO")
    {
        return LangType::GO;
    }
    if (type_str == "JAVASCRIPT")
    {
        return LangType::JAVASCRIPT;
    }
    return LangType::UNKNOW;
}

//LanguageManager 构造函数
LanguageManager::LanguageManager()
{
    //初始化语言配置
    this->init();
}

//获取 LanguageManager 单例实例
LanguageManager* LanguageManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new LanguageManager();
    }
    return s_instance;
}

//初始化语言配置
void LanguageManager::init()
{
    //配置文件路径
    std::string config_path = "res/languages/languages.json";

    try
    {
        //读取配置文件
        std::ifstream config_file(config_path);
        if (!config_file.is_open())
        {
            std::cerr << "[error]LanguageManager::init: Failed to open config file: " << config_path << std::endl;
            return;
        }

        //解析 JSON
        nlohmann::json config;
        config_file >> config;
        config_file.close();

        //加载语言配置
        if (config.contains("languages") && config["languages"].is_array())
        {
            for (size_t i = 0; i < config["languages"].size(); i++)
            {
                const nlohmann::json& lang_config = config["languages"][i];
                //提取语言配置
                std::string type_str = lang_config["type"];
                std::string name = lang_config["name"];
                std::string file_extension = lang_config["file_extension"];
                std::string compile_cmd = lang_config["compile_cmd"];
                std::string run_cmd = lang_config["run_cmd"];
                bool need_compile = lang_config["need_compile"];

                //检查是否启用
                bool enabled = true;
                if (lang_config.contains("enabled"))
                {
                    enabled = lang_config["enabled"];
                }

                //如果未启用，跳过
                if (!enabled)
                {
                    std::cout << "[info]LanguageManager::init: Skipping disabled language: " << name << std::endl;
                    continue;
                }

                //转换语言类型
                LangType type = stringToLangType(type_str);
                if (type == LangType::UNKNOW)
                {
                    std::cerr << "[warn]LanguageManager::init: Unknown language type: " << type_str << std::endl;
                    continue;
                }

                //创建语言配置对象
                LanguageConfig lang(type, name, file_extension, compile_cmd, run_cmd, need_compile);

                //添加危险函数
                if (lang_config.contains("dangerous_functions") && lang_config["dangerous_functions"].is_array())
                {
                    for (size_t j = 0; j < lang_config["dangerous_functions"].size(); j++)
                    {
                        const std::string& func = lang_config["dangerous_functions"][j];
                        lang.addDangerousFunction(func);
                    }
                }

                //添加危险模块
                if (lang_config.contains("dangerous_modules") && lang_config["dangerous_modules"].is_array())
                {
                    for (size_t j = 0; j < lang_config["dangerous_modules"].size(); j++)
                    {
                        const std::string& module = lang_config["dangerous_modules"][j];
                        lang.addDangerousModule(module);
                    }
                }

                //添加语言配置
                this->addLanguage(lang);

                //添加语言映射
                this->m_string_to_lang[name] = type;
                //添加可能的别名
                if (name == "c++")
                {
                    this->m_string_to_lang["cpp"] = type;
                }
                else if (name == "python3")
                {
                    this->m_string_to_lang["python"] = type;
                }

                //添加语言类型到字符串的映射
                this->m_lang_to_string[type] = name;
            }

            std::cout << "[info]LanguageManager::init: Loaded " << this->m_language_configs.size() << " languages from config file" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]LanguageManager::init: Failed to load language config: " << e.what() << std::endl;
    }
}

//添加语言配置
void LanguageManager::addLanguage(const LanguageConfig& config)
{
    this->m_language_configs[config.getType()] = config;
}

//根据语言类型获取配置
const LanguageConfig* LanguageManager::getLanguage(LangType type) const
{
    std::map<LangType, LanguageConfig>::const_iterator it = this->m_language_configs.find(type);
    if (it != this->m_language_configs.end())
    {
        return &(it->second);
    }
    return nullptr;
}

//根据字符串获取语言类型
LangType LanguageManager::getLangType(const std::string& lang_str) const
{
    std::string lower_str = lang_str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);

    std::map<std::string, LangType>::const_iterator it = this->m_string_to_lang.find(lower_str);
    if (it != this->m_string_to_lang.end())
    {
        return it->second;
    }
    return LangType::UNKNOW;
}

//根据语言类型获取字符串
std::string LanguageManager::getLangString(LangType type) const
{
    std::map<LangType, std::string>::const_iterator it = this->m_lang_to_string.find(type);
    if (it != this->m_lang_to_string.end())
    {
        return it->second;
    }
    return "unknow";
}

//检查语言是否支持
bool LanguageManager::isLanguageSupported(LangType type) const
{
    return this->m_language_configs.find(type) != this->m_language_configs.end();
}
