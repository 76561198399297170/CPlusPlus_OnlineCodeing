#ifndef _LANGUAGE_CONFIG_H_
#define _LANGUAGE_CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include "runtime/executor.h"

//语言配置类
class LanguageConfig
{
public:
    //默认构造函数
    LanguageConfig() : m_type(LangType::UNKNOW), m_name(""), m_file_extension(""),
        m_compile_cmd(""), m_run_cmd(""), m_need_compile(false)
    {}

    //构造函数
    LanguageConfig(LangType type, const std::string& name, const std::string& file_extension,
        const std::string& compile_cmd, const std::string& run_cmd, bool need_compile);

    //获取语言类型
    LangType getType() const
    {
        return this->m_type;
    }

    //获取语言名称
    std::string getName() const
    {
        return this->m_name;
    }

    //获取文件扩展名
    std::string getFileExtension() const
    {
        return this->m_file_extension;
    }

    //获取编译命令
    std::string getCompileCmd() const
    {
        return this->m_compile_cmd;
    }

    //获取运行命令
    std::string getRunCmd() const
    {
        return this->m_run_cmd;
    }

    //是否需要编译
    bool needCompile() const
    {
        return this->m_need_compile;
    }

    //添加危险函数
    void addDangerousFunction(const std::string& func)
    {
        this->m_dangerous_functions.push_back(func);
    }

    //添加危险模块/头文件
    void addDangerousModule(const std::string& module)
    {
        this->m_dangerous_modules.push_back(module);
    }

    //获取危险函数列表
    const std::vector<std::string>& getDangerousFunctions() const
    {
        return this->m_dangerous_functions;
    }

    //获取危险模块列表
    const std::vector<std::string>& getDangerousModules() const
    {
        return this->m_dangerous_modules;
    }

private:
    LangType m_type;               //语言类型
    std::string m_name;            //语言名称
    std::string m_file_extension;  //文件扩展名
    std::string m_compile_cmd;     //编译命令
    std::string m_run_cmd;         //运行命令
    bool m_need_compile;           //是否需要编译

    //安全相关配置
    std::vector<std::string> m_dangerous_functions;  //危险函数列表
    std::vector<std::string> m_dangerous_modules;    //危险模块/头文件列表
};

//语言管理器类
class LanguageManager
{
public:
    //获取单例实例
    static LanguageManager* getInstance();

    //初始化语言配置
    void init();

    //添加语言配置
    void addLanguage(const LanguageConfig& config);

    //根据语言类型获取配置
    const LanguageConfig* getLanguage(LangType type) const;

    //根据字符串获取语言类型
    LangType getLangType(const std::string& lang_str) const;

    //根据语言类型获取字符串
    std::string getLangString(LangType type) const;

    //检查语言是否支持
    bool isLanguageSupported(LangType type) const;

private:
    //构造函数
    LanguageManager();

    //语言配置映射
    std::map<LangType, LanguageConfig> m_language_configs;

    //字符串到语言类型的映射
    std::map<std::string, LangType> m_string_to_lang;

    //语言类型到字符串的映射
    std::map<LangType, std::string> m_lang_to_string;
};

#endif // !_LANGUAGE_CONFIG_H_
