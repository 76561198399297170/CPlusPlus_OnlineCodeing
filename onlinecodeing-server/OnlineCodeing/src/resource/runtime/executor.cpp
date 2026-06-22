#include "runtime/executor.h"
#include "runtime/language_config.h"
#include "utils/security.h"

LangType codeStringToLang(std::string lang)
{
    return LanguageManager::getInstance()->getLangType(lang);
}

std::string codeLangToString(LangType lang)
{
    return LanguageManager::getInstance()->getLangString(lang);
}

CodeResult codeStringFindCodeResult(std::string str, std::string& error_msg)
{
    CodeResult status;
    if (str.find("AC") == 0)
    {
        status = CodeResult::CODE_AC;
    }
    else if (str.find("WA") == 0)
    {
        status = CodeResult::CODE_WA;
        error_msg = str.length() > 3 ? str.substr(3) : "";
    }
    else if (str.find("CE") == 0)
    {
        status = CodeResult::CODE_CE;
        error_msg = str.length() > 3 ? str.substr(3) : "";
    }
    else if (str.find("TLE") == 0)
    {
        status = CodeResult::CODE_TLE;
        error_msg = str.length() > 3 ? str.substr(3) : "";
    }
    else
    {
        status = CodeResult::UNKNOW;
        error_msg = str;
    }
    return status;
}

CodeResult codeIntToCodeResult(int i)
{
    switch (i)
    {
    case 0:
        return CodeResult::CODE_CE;
    case 1:
        return CodeResult::CODE_ERROR;
    case 2:
        return CodeResult::CODE_TLE;
    case 3:
        return CodeResult::CODE_AC;
    case 4:
        return CodeResult::CODE_WA;
    default:
        break;
    }

    return CodeResult::UNKNOW;
}

int codeStringToInt(std::string result)
{
    //根据数据库中 user_submission 用户提交记录表描述进行转换
    if (result == "AC") return 3;
    if (result == "WA") return 4;
    if (result == "CE") return 0;
    if (result == "TLE") return 2;
    if (result == "ERROR") return 1;
    return -1;
}

std::string codeResultToString(CodeResult type)
{
    switch (type)
    {
    case CodeResult::CODE_AC:
        return "AC";
    case CodeResult::CODE_WA:
        return "WA";
    case CodeResult::CODE_CE:
        return "CE";
    case CodeResult::CODE_TLE:
        return "TLE";
    case CodeResult::CODE_ERROR:
        return "ERROR";
    }
    return "UNKNOW";
}

std::string codeIntToStringDifficulty(int difficulty)
{
    switch (difficulty)
    {
    case 1:
        return "入门";
    case 2:
        return "基础";
    case 3:
        return "中等";
    case 4:
        return "进阶";
    case 5:
        return "困难";
    }
    return "未定";
}

CodeExecutor::CodeExecutor() :
    m_time_limit_ms(1000),
    m_compile_time_limit_ms(5000),
    m_memory_limit_mb(128),
    m_run_time_ms(0),
    m_code_size_bytes(0),
    m_test_outputs()
{}

CodeExecutor::~CodeExecutor()
{}

void CodeExecutor::sanitizeCode(std::string& code, LangType lang)
{
    // 获取语言配置
    const LanguageConfig* config = LanguageManager::getInstance()->getLanguage(lang);
    if (!config) {
        // 语言未配置，跳过处理
        return;
    }

    // 处理 C/C++ 语言
    if (lang == LangType::C || lang == LangType::CPP) {
        // 移除危险的头文件包含
        const auto& dangerous_modules = config->getDangerousModules();
        for (const std::string& module : dangerous_modules) {
            size_t pos = code.find(module);
            if (pos != std::string::npos) {
                //找到包含该头行移除
                size_t line_start = code.rfind('\n', pos);
                if (line_start == std::string::npos) {
                    line_start = 0;
                }
                size_t line_end = code.find('\n', pos + module.size());
                if (line_end == std::string::npos) {
                    line_end = code.size();
                }
                else line_end += 1;
                code.erase(line_start, line_end - line_start);
            }
        }

        // 移除危险的函数调用
        const auto& dangerous_functions = config->getDangerousFunctions();
        for (const std::string& func : dangerous_functions) {
            size_t pos = 0;
            while ((pos = code.find(func, pos)) != std::string::npos) {
                //函数调用范围替换为空
                size_t start = pos;
                int brace_count = 0;
                bool in_function = false;

                for (size_t i = pos; i < code.size(); i++) {
                    if (code[i] == '(') {
                        brace_count++;
                        in_function = true;
                    }
                    else if (code[i] == ')') {
                        brace_count--;
                        if (in_function && brace_count == 0) {
                            //替换整个函数调用为0
                            code.replace(start, i - start + 1, "0");
                            break;
                        }
                    }
                }
                pos = start + 1;
            }
        }

        // 移除文件操作相关的代码
        std::vector<std::string> file_operations = {
            "fstream", "ifstream", "ofstream", "fopen", "fclose", "fread", "fwrite"
        };

        for (const std::string& op : file_operations) {
            size_t pos = 0;
            while ((pos = code.find(op, pos)) != std::string::npos) {
                //简单处理相关代码注释
                size_t line_start = code.rfind('\n', pos);
                if (line_start == std::string::npos) {
                    line_start = 0;
                }
                size_t line_end = code.find('\n', pos + op.size());
                if (line_end == std::string::npos) {
                    line_end = code.size();
                }
                else {
                    line_end += 1;
                }

                //替换为注释
                std::string line = code.substr(line_start, line_end - line_start);
                code.replace(line_start, line_end - line_start, "// [Security] Removed file operation: " + line);
                pos = line_start + 1;
            }
        }
    }
    // 处理 Python3 语言
    else if (lang == LangType::PYTHON3) {
        // 移除危险模块导入
        const auto& dangerous_modules = config->getDangerousModules();
        for (const std::string& module : dangerous_modules) {
            size_t pos = 0;
            while ((pos = code.find("import " + module, pos)) != std::string::npos) {
                size_t line_start = code.rfind('\n', pos);
                if (line_start == std::string::npos) {
                    line_start = 0;
                }
                size_t line_end = code.find('\n', pos);
                if (line_end == std::string::npos) {
                    line_end = code.size();
                }
                else line_end += 1;
                code.replace(line_start, line_end - line_start, "# [Security] Removed dangerous module import\n");
                pos = line_start + 1;
            }

            // 处理from ... import ... 形式
            pos = 0;
            while ((pos = code.find("from " + module + " import", pos)) != std::string::npos) {
                size_t line_start = code.rfind('\n', pos);
                if (line_start == std::string::npos) {
                    line_start = 0;
                }
                size_t line_end = code.find('\n', pos);
                if (line_end == std::string::npos) {
                    line_end = code.size();
                }
                else {
                    line_end += 1;
                }
                code.replace(line_start, line_end - line_start, "# [Security] Removed dangerous module import\n");
                pos = line_start + 1;
            }
        }

        // 移除危险函数调用
        const auto& dangerous_functions = config->getDangerousFunctions();
        for (const std::string& func : dangerous_functions) {
            size_t pos = 0;
            while ((pos = code.find(func, pos)) != std::string::npos) {
                //找到函数调用范围替换
                size_t start = pos;
                int brace_count = 0;
                bool in_function = false;

                for (size_t i = pos; i < code.size(); i++) {
                    if (code[i] == '(') {
                        brace_count++;
                        in_function = true;
                    }
                    else if (code[i] == ')') {
                        brace_count--;
                        if (in_function && brace_count == 0) {
                            //替换为0
                            code.replace(start, i - start + 1, "0");
                            break;
                        }
                    }
                }
                pos = start + 1;
            }
        }
    }
    // 其他语言暂未部署，所以暂不处理
}

std::string CodeExecutor::createTempDir()
{
    std::string temp_dir_name;
    std::filesystem::path temp_dir;
    bool created = false;

    //尝试10次使用时间戳与随机5位数方法创建文件夹确保不会重名
    for (int i = 0; i < 10; i++)
    {
        long long time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int random_num = util::generateRandomInt(10000, 99999);

        temp_dir_name = "code_even_" + std::to_string(time_stamp) + "_" + std::to_string(random_num);
        temp_dir = std::filesystem::path(this->m_temp_dir) / temp_dir_name;

        try
        {
            if (std::filesystem::create_directories(temp_dir))
            {
                created = true;
                break;
            }
        }
        catch (const std::exception& e)
        {
            //短暂休眠后再次重试
            std::cerr << "[warn]CodeExecutor::createTempDir: create temp dir " << i + 1 << " failed: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    if (!created)
    {
        throw std::runtime_error("[error]CodeExecutor::createTempDir: failed to create temp directory after multiple attempts");
    }

    std::cout << "[debug]CodeExecutor::createTempDir: created temp dir: " << temp_dir.string() << std::endl;
    return temp_dir.string();
}

std::string CodeExecutor::generateCodeCommands(LangType lang, const std::string& code, const std::filesystem::path& temp_dir, std::filesystem::path& code_file, std::string& run_cmd, std::string& compile_cmd)
{
    // 获取语言配置
    const LanguageConfig* config = LanguageManager::getInstance()->getLanguage(lang);
    if (!config) {
        return codeResultToString(CodeResult::CODE_ERROR) + ": Unsupported language";
    }

    // 生成代码文件路径
    std::string file_extension = config->getFileExtension();
    code_file = temp_dir / ("main." + file_extension);

    // 生成编译命令
    if (config->needCompile()) {
        std::string compile_template = config->getCompileCmd();
        std::string output_file = (temp_dir / "main.exe").string();
        
        // 替换命令中的占位符
        size_t pos = compile_template.find("{input}");
        if (pos != std::string::npos) {
            compile_template.replace(pos, 7, code_file.string());
        }
        pos = compile_template.find("{output}");
        if (pos != std::string::npos) {
            compile_template.replace(pos, 8, output_file);
        }
        
        // 为 C/C++ 编译命令添加 UTF-8 编码支持
        if (lang == LangType::C || lang == LangType::CPP) {
            compile_template += " -finput-charset=UTF-8 -fexec-charset=UTF-8";
        }
        
        compile_cmd = compile_template;
    } else {
        compile_cmd = "";
    }

    // 生成运行命令
    std::string run_template = config->getRunCmd();
    size_t pos = run_template.find("{input}");
    if (pos != std::string::npos) {
        run_template.replace(pos, 7, code_file.string());
    }
    pos = run_template.find("{output}");
    if (pos != std::string::npos) {
        std::string output_file = (temp_dir / "main.exe").string();
        run_template.replace(pos, 8, output_file);
    }
    run_cmd = run_template;

    // 写入代码文件
    std::ofstream code_stream(code_file, std::ios::binary);
    if (!code_stream.is_open())
    {
        throw std::runtime_error("failed to write code file: " + code_file.string());
    }
    
    // 只给 C/C++ 添加 UTF-8 BOM，其他语言不加！
    if (lang == LangType::C || lang == LangType::CPP) {
        const char utf8_bom[] = { (char)0xEF, (char)0xBB, (char)0xBF };
        code_stream.write(utf8_bom, 3);
    }
    
    // 以 UTF-8 编码写入代码内容
    int wide_len = MultiByteToWideChar(65001, 0, code.c_str(), -1, nullptr, 0);
    wchar_t* wide_buf = new wchar_t[wide_len];
    MultiByteToWideChar(65001, 0, code.c_str(), -1, wide_buf, wide_len);
    
    int utf8_len = WideCharToMultiByte(65001, 0, wide_buf, -1, nullptr, 0, nullptr, nullptr);
    char* utf8_buf = new char[utf8_len];
    WideCharToMultiByte(65001, 0, wide_buf, -1, utf8_buf, utf8_len, nullptr, nullptr);
    
    code_stream.write(utf8_buf, utf8_len - 1);
    
    delete[] wide_buf;
    delete[] utf8_buf;
    code_stream.close();

    //计算代码大小
    try
    {
        m_code_size_bytes = std::filesystem::file_size(code_file);
    }
    catch (...)
    {
        m_code_size_bytes = 0;
    }

    return "";
}

void CodeExecutor::runCompile(LangType lang, const std::string& compile_cmd)
{
    // 获取语言配置
    const LanguageConfig* config = LanguageManager::getInstance()->getLanguage(lang);
    if (!config) {
        return;
    }

    // 检查是否需要编译
    if (!config->needCompile() || compile_cmd.empty()) {
        return;
    }

    // 执行编译操作
    HANDLE hCompileErrRead = nullptr, hCompileErrWrite = nullptr;
    STARTUPINFOA si_compile = { 0 };
    PROCESS_INFORMATION pi_compile = { 0 };
    si_compile.cb = sizeof(STARTUPINFOA);

    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;//允许子进程继承句柄
    sa.lpSecurityDescriptor = nullptr;
    //创建管道指定缓冲区大小避免默认导致阻塞
    if (!CreatePipe(&hCompileErrRead, &hCompileErrWrite, &sa, 4096))
    {
        DWORD err = GetLastError();
        throw std::runtime_error("Create pipe failed: " + std::to_string(err));
    }
    this->setHandleNonInheritable(hCompileErrRead);

    //重定向编译错误输出
    si_compile.hStdError = hCompileErrWrite;
    si_compile.hStdOutput = hCompileErrWrite;//编译输出也重定向到输出错误管道
    si_compile.dwFlags |= STARTF_USESTDHANDLES;

    //启动编译进程
    std::cout << "[debug]CodeExecutor::runCompile: starting compile..." << std::endl;
    char* cmd_buf = const_cast<char*>(compile_cmd.c_str());//CreateProcess需要可写缓冲区
    if (!CreateProcessA(nullptr, cmd_buf, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si_compile, &pi_compile))
    {
        DWORD err = GetLastError();
        this->closeHandle(hCompileErrRead);
        this->closeHandle(hCompileErrWrite);
        throw std::runtime_error("compile process failed (code: " + std::to_string(err) + ")" + (err == 2 ? " - G++ not found in PATH!" : ""));
    }
    this->closeHandle(hCompileErrWrite);

    DWORD wait_res = WaitForSingleObject(pi_compile.hProcess, this->m_compile_time_limit_ms);
    if (wait_res == WAIT_TIMEOUT)
    {
        //500ms读取错误信息
        std::string compile_err = this->readPipeNonBlocking(hCompileErrRead, 500);
        //超时强制终止编译进程
        TerminateProcess(pi_compile.hProcess, 1);
        this->closeHandle(pi_compile.hProcess);
        this->closeHandle(pi_compile.hThread);
        this->closeHandle(hCompileErrRead);
        if (!compile_err.empty())
        {
            throw std::runtime_error(codeResultToString(CodeResult::CODE_CE) + ": " + compile_err);
        }
        throw std::runtime_error(codeResultToString(CodeResult::CODE_CE) + ": Compile timeout (>" + std::to_string(this->m_compile_time_limit_ms / 1000.0) + "s)");
    }

    DWORD compile_exit_code = 0;
    GetExitCodeProcess(pi_compile.hProcess, &compile_exit_code);
    std::cout << "[debug]CodeExecutor::runCompile: compile exit code: " << compile_exit_code << std::endl;
    //防1秒阻塞读取编译错误
    std::string compile_err = this->readPipeNonBlocking(hCompileErrRead, 1000);
    //清理如\t与\n之类无关字符
    compile_err.erase(std::remove(compile_err.begin(), compile_err.end(), '\r'), compile_err.end());
    std::cout << "[debug]CodeExecutor::runCompile: compile error: " << (compile_err.empty() ? "none" : compile_err) << std::endl;

    this->closeHandle(pi_compile.hProcess);
    this->closeHandle(pi_compile.hThread);
    this->closeHandle(hCompileErrRead);

    //编译失败
    if (compile_exit_code != 0)
    {
        throw std::runtime_error(codeResultToString(CodeResult::CODE_CE) + ": " + (compile_err.empty() ? "unknown compile error" : compile_err));
    }
}

void CodeExecutor::closeHandle(HANDLE& h)
{
    if (h != nullptr && h != INVALID_HANDLE_VALUE)
    {
        CloseHandle(h);
        h = nullptr;
    }
}

void CodeExecutor::setHandleNonInheritable(HANDLE& h)
{
    if (h != nullptr && h != INVALID_HANDLE_VALUE)
    {
        SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0);
    }
}

std::string CodeExecutor::readPipeNonBlocking(HANDLE hRead, int timeout_ms)
{
    if (hRead == nullptr || hRead == INVALID_HANDLE_VALUE)
    {
        return "";
    }

    std::string result;
    char buf[4096] = { 0 };//注意此处最多读4096个字符
    DWORD bytes_avail = 0;
    DWORD bytes_read = 0;
    auto start = std::chrono::steady_clock::now();

    while (true)
    {
        //超时检查
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
        if (elapsed >= timeout_ms)
        {
            break;
        }
        //数据检查
        if (!PeekNamedPipe(hRead, nullptr, 0, nullptr, &bytes_avail, nullptr))
        {
            break;
        }
        //数据等待
        if (bytes_avail == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        //读取数据
        DWORD to_read = min(bytes_avail, (DWORD)sizeof(buf) - 1);
        if (ReadFile(hRead, buf, to_read, &bytes_read, nullptr) && bytes_read > 0)
        {
            result += std::string(buf, bytes_read);
            memset(buf, 0, sizeof(buf));
        }
        else
        {
            break;
        }
    }

    return result;
}

std::string CodeExecutor::evaluateCode(LangType lang, const std::string& code, const std::vector<std::vector<std::string>>& test_case_inputs, const std::vector<std::vector<std::string>>& test_case_outputs)
{
    std::string evaluate_flag = "";

    std::filesystem::path temp_dir;
    std::string result = codeResultToString(CodeResult::CODE_ERROR);
    HANDLE hJob = nullptr;
    const int TIME_LIMIT = this->m_time_limit_ms;
    const int COMPILE_TIMEOUT = this->m_compile_time_limit_ms;
    const SIZE_T MEM_LIMIT = this->m_memory_limit_mb * 1024 * 1024;// 内存限制：128MB = 128 * 1024 * 1024
    const std::string CUSTOM_TEMP_ROOT = this->m_temp_dir;

    try
    {
        //创建临时目录
        temp_dir = this->createTempDir();
        temp_dir = std::filesystem::path(temp_dir);

        //代码安全处理
        std::string safe_code = code;
        this->sanitizeCode(safe_code, lang);

        //生成代码文件与控制台命令
        std::filesystem::path code_file;
        std::string compile_cmd, run_cmd;
        evaluate_flag = this->generateCodeCommands(lang, safe_code, temp_dir, code_file, run_cmd, compile_cmd);
        if (!evaluate_flag.empty())
        {
            return evaluate_flag;
        }

        //编译处理
        this->runCompile(lang, compile_cmd);

        //创建Job Object限制资源
        hJob = CreateJobObject(nullptr, nullptr);
        if (hJob)
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_limits = { 0 };
            job_limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            job_limits.ProcessMemoryLimit = MEM_LIMIT;
            SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &job_limits, sizeof(job_limits));
        }

        //执行测试用例
        result = this->runTests(lang, run_cmd, hJob, temp_dir, test_case_inputs, test_case_outputs);
    }
    catch (const std::exception& e)
    {
        result = e.what();
        std::cerr << "[error]CodeExecutor::evaluateCode: " << result << std::endl;
    }

    //清理资源
    this->closeHandle(hJob);
    if (std::filesystem::exists(temp_dir))
    {
        try
        {
            //延迟删除避免句柄未释放
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::filesystem::remove_all(temp_dir);
        }
        catch (...)
        {
        }
    }

    return result;
}

std::string CodeExecutor::runTests(LangType lang, const std::string& run_cmd, HANDLE hJob, const std::filesystem::path& temp_dir, const std::vector<std::vector<std::string>>& test_case_input, const std::vector<std::vector<std::string>>& test_case_output)
{
    m_run_time_ms = 0;
    m_test_outputs.clear();
    for (size_t case_idx = 0; case_idx < test_case_output.size(); case_idx++)
    {
        std::cout << "[debug]CodeExecutor::evaluateCode: running case " << case_idx + 1 << "/" << test_case_output.size() << std::endl;
        std::string case_input, case_expected;
        for (const std::string& line : test_case_input[case_idx]) case_input += line + "\n";
        for (const std::string& line : test_case_output[case_idx]) case_expected += line + "\n";

        //创建输入输出管道
        HANDLE hInRead = nullptr, hInWrite = nullptr;
        HANDLE hOutRead = nullptr, hOutWrite = nullptr;
        SECURITY_ATTRIBUTES sa = { 0 };
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE; //关键：允许子进程继承句柄
        sa.lpSecurityDescriptor = nullptr;
        CreatePipe(&hInRead, &hInWrite, &sa, 4096);
        CreatePipe(&hOutRead, &hOutWrite, &sa, 4096);

        //重要：确保子进程不会继承写端！避免死锁！
        SetHandleInformation(hInWrite, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(hOutRead, HANDLE_FLAG_INHERIT, 0);

        //创建运行进程（先挂起）
        STARTUPINFOA si_run = { 0 };
        PROCESS_INFORMATION pi_run = { 0 };
        si_run.cb = sizeof(STARTUPINFOA);
        si_run.hStdInput = hInRead;
        si_run.hStdOutput = hOutWrite;
        si_run.hStdError = hOutWrite;
        si_run.dwFlags |= STARTF_USESTDHANDLES;

        char* run_cmd_buf = const_cast<char*>(run_cmd.c_str());
        if (!CreateProcessA(nullptr, run_cmd_buf, nullptr, nullptr, TRUE, CREATE_SUSPENDED, nullptr, nullptr, &si_run, &pi_run))
        {
            this->closeHandle(hInRead);
            this->closeHandle(hInWrite);
            this->closeHandle(hOutRead);
            this->closeHandle(hOutWrite);
            throw std::runtime_error("run process failed (case " + std::to_string(case_idx + 1) + ")");
        }

        if (hJob)
        {
            AssignProcessToJobObject(hJob, pi_run.hProcess);
        }

        //记录开始时间
        auto start_time = std::chrono::steady_clock::now();

        ResumeThread(pi_run.hThread);

        //现在写入输入数据！！
        if (!case_input.empty())
        {
            DWORD bytes_written = 0;
            WriteFile(hInWrite, case_input.c_str(), case_input.size(), &bytes_written, nullptr);
        }

        //关键！！写入数据后立即关闭写端，让Python知道数据结束了！
        this->closeHandle(hInWrite);

        DWORD wait_res = WaitForSingleObject(pi_run.hProcess, this->m_time_limit_ms);
        if (wait_res == WAIT_TIMEOUT)
        {
            TerminateProcess(pi_run.hProcess, 0);
            this->closeHandle(pi_run.hProcess);
            this->closeHandle(pi_run.hThread);
            this->closeHandle(hInRead);
            this->closeHandle(hOutRead);
            this->closeHandle(hOutWrite);
            throw std::runtime_error(codeResultToString(CodeResult::CODE_TLE) + ": case " + std::to_string(case_idx + 1));
        }

        //记录结束时间并计算运行时间
        auto end_time = std::chrono::steady_clock::now();
        long long case_run_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        m_run_time_ms += case_run_time;

        this->closeHandle(hOutWrite);
        std::string case_actual = this->readPipeNonBlocking(hOutRead, 1000);
        std::cout << "[debug]CodeExecutor::runTests: raw output (before conversion): \"" << case_actual << "\" (size: " << case_actual.size() << ")" << std::endl;

        //仅对非 C/C++ 语言进行 GBK 到 UTF-8 转换
        //C/C++ 已通过 -fexec-charset=UTF-8 确保输出为 UTF-8
        if (lang != LangType::C && lang != LangType::CPP)
        {
            case_actual = util::gbk_to_utf8(case_actual);
            std::cout << "[debug]CodeExecutor::runTests: converted output: \"" << case_actual << "\"" << std::endl;
        }

        //存储测试用例输出
        m_test_outputs.emplace_back(case_actual);

        //归一化输出对比
        auto normalize = [](std::string s)
            {
                s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
                size_t pos = s.find_last_not_of(" \t\n");
                return pos == std::string::npos ? "" : s.substr(0, pos + 1);
            };
        std::string norm_actual = normalize(case_actual);
        std::string norm_expected = normalize(case_expected);

        std::cout << "[debug]CodeExecutor::runTests: expected: " << norm_expected << ", actual: " << norm_actual << "" << std::endl;
        if (norm_actual != norm_expected)
        {
            this->closeHandle(pi_run.hProcess);
            this->closeHandle(pi_run.hThread);
            this->closeHandle(hInRead);
            this->closeHandle(hOutRead);
            throw std::runtime_error(codeResultToString(CodeResult::CODE_WA) + ": case " + std::to_string(case_idx + 1) + "\nexp: " + norm_expected + "\nact: " + norm_actual);
        }

        this->closeHandle(pi_run.hProcess);
        this->closeHandle(pi_run.hThread);
        this->closeHandle(hInRead);
        this->closeHandle(hOutRead);
    }

    std::cout << "[debug]CodeExecutor::runTests: all cases passed!" << std::endl;
    return codeResultToString(CodeResult::CODE_AC);
}

void CodeExecutor::cleanupTempDir(const std::filesystem::path& path)
{
    try
    {
        if (std::filesystem::exists(path))
        {
            //延迟删除避免句柄未释放
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::filesystem::remove_all(path);
            std::cout << "[debug]CodeExecutor::cleanupTempDir: cleaned up temp dir: " << path.string() << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "[warn]CodeExecutor::cleanupTempDir: failed to cleanup temp dir: " << e.what() << std::endl;
    }
}
