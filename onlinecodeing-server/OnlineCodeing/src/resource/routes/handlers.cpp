#include "routes/handlers.h"

#include "database/data_access.h"
#include "runtime/program_access.h"
#include "runtime/language_config.h"
#include "agent/model_access.h"
#include "utils/config.h"

extern const std::string SECURE_SECRET_KEY;
extern const std::string PROBLEM_PATH;
extern const std::string TESTPOINTS_PATH;

extern const int TOKEN_VALID_TIME_SET;
extern const double TOKEN_REGENERATE_PERCENT;

void handleRegister(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleRegister: get register." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"注册失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        bool success = false;
        int64_t user_id = -1, pt_id = -1;

        nlohmann::json req_data = nlohmann::json::parse(req.body);
        DBType role = RoleStringToDBType(req_data["role"]);

        std::cout << "[debug]req role: " << req_data["role"] << std::endl;

        std::string account = req_data["account"];
        std::string email = req_data["email"];
        std::string phone = req_data["phone"];
        std::string password = req_data["password"];
        std::string niko_name = req_data["nickname"].empty() ? ("user_" + std::to_string(util::generateRandomInt(1000, 9999))) : req_data["nickname"].get<std::string>();

        if (account.empty() || email.empty() || phone.empty() || password.empty())
        {
            resp["msg"] = u8"必填部分不能为空";
        }
        else
        {
            std::cout << "[debug]handleRegister: try to register, account: " << account << ", phone: " << phone << ", email: " << email << ", role: " << req_data["role"];
            switch (role)
            {
            case DBType::ADMIN:
            {
                std::cout << std::endl;
                std::string audit_admin = req_data["auditAdmin"];
                std::string audit_password = req_data["auditPassword"];
                std::unique_ptr<PooledResultSet> admin_res = DataAccess::getInstance()->selectDBResources<std::string>(account, DBType::USER, "account");
                if (admin_res && admin_res->next())
                {
                    if (admin_res->getString("account") == audit_admin && sha256::hashPasswordBysha256(password, admin_res->getString("salt")) == admin_res->getString("password"))
                    {
                        success = DataAccess::getInstance()->createDBadmin(account, phone, email, password, niko_name, "desc", 0, user_id, pt_id);
                        break;
                    }
                    else
                    {
                        resp["code"] = -500;
                        resp["msg"] = u8"管理员账号或密码错误";
                        success = false;
                        break;

                    }
                }
                else
                {
                    resp["code"] = -401;
                    resp["msg"] = u8"未搜索到该管理员用户";
                    success = false;
                    break;
                }
            }
            case DBType::STUDENT:
            {
                std::string school = req_data["school"];
                int grade = req_data["grade"].get<int>();
                std::string real_name = req_data["real_name"];

                if (school.empty() || real_name.empty())
                {
                    resp["msg"] = u8"必填部分不能为空";
                }
                else
                {
                    std::cout << ", school: " << school << ", grade: " << grade << ", real_name: " << real_name << std::endl;
                    success = DataAccess::getInstance()->createDBstudent(account, phone, email, password, niko_name, "desc", 0, school, grade, "1", real_name, "nothing", user_id, pt_id);
                }
                break;
            }
            case DBType::TEACHER:
            {
                std::string real_name = req_data["real_name"];
                std::string school = req_data["school"];
                if (real_name.empty() || school.empty())
                {
                    resp["msg"] = u8"必填部分不能为空";
                }
                else
                {
                    std::cout << ", school: " << school << ", real_name: " << real_name << std::endl;
                    success = DataAccess::getInstance()->createDBteacher(account, phone, email, password, niko_name, "desc", 0, school, real_name, "nothing", user_id, pt_id);
                }
                break;
            }
            default:
            {
                std::cout << "[error]handleRegister: Invalid role identity." << std::endl;
                resp["msg"] = u8"无效的角色类型：仅支持 admin/student/teacher";
                success = false;
                break;
            }
            }
        }

        if (success)
        {
            resp["code"] = 0;
            resp["msg"] = u8"success";
            resp["data"] =
            {
                {"id", user_id},
                {"account", account},
                {"nickname", niko_name},
                {"role", DBTypeToRoleString(role)},
                {"token", hs256::generateToken(SECURE_SECRET_KEY, user_id, DBTypeToRoleString(role))}
            };
        }
        else
        {
            resp["code"] = -401;
            if (resp["code"].empty()) resp["code"] = u8"未知原因，注册失败请稍后尝试...";
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleRegister: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleRegister: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"注册失败：" + std::string(e.what());
        std::cerr << "[error]handleRegister: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"注册失败：未知系统错误";
        std::cerr << "[error]handleRegister: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleLogin(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleLogin: get login." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"登录失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        nlohmann::json req_data = nlohmann::json::parse(req.body);
        std::string account = req_data["account"];
        std::string password = req_data["password"];
        std::string role_str = req_data["role"];

        if (account.empty() || password.empty())
        {
            resp["msg"] = u8"账号或密码不能为空";
        }
        else
        {
            nlohmann::json user_data;
            std::unique_ptr<PooledResultSet> user_res = DataAccess::getInstance()->selectDBResources<std::string>(account, DBType::USER, "account");

            if (user_res && user_res->next())
            {
                std::string psw = user_res->getString("password");
                std::string slt = user_res->getString("salt");
                std::string hashpsw = sha256::hashPasswordBysha256(password, slt);
                int rol = user_res->getInt("role");

                std::cout << "[debug]account: " << account << ", password: " << password << ", role: " << role_str << ", hash input: " << hashpsw << ", dbpassword: " << psw << std::endl;

                if (rol == (int)RoleStringToDBType(role_str) && psw == hashpsw)
                {
                    int64_t user_id = user_res->getInt("user_id");
                    std::string nikoname = "";
                    try
                    {
                        nikoname = user_res->getString("nickname");
                    }
                    catch (...)
                    {
                        nikoname = "user_" + util::generateRandomInt(10000, 99999);
                    }

                    std::cout << "[debug]handleLogin: try to login, account: " << account << ", password: " << password << ", role: " << role_str << std::endl;

                    resp["code"] = 0;
                    resp["msg"] = u8"登录成功";
                    resp["data"] =
                    {
                        {"id", user_id},
                        {"account", account},
                        {"nickname", nikoname},
                        {"role", role_str},
                        {"token", hs256::generateToken(SECURE_SECRET_KEY, user_id, DBTypeToRoleString(RoleIntToDBType(rol)))}
                    };
                }
                else
                {
                    resp["msg"] = u8"账号登录错误";
                }


            }
            else
            {
                std::cerr << "[error]handleLogin: mysql select empty." << std::endl;
                resp["msg"] = u8"账号登录错误";
            }
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleLogin: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleLogin: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"登录失败：" + std::string(e.what());
        std::cerr << "[error]handleLogin: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"登录失败：未知系统错误";
        std::cerr << "[error]handleLogin: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleRunTese(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleRunTese: sub code for test." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", 0},
        {"msg", u8"运行失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);

            int64_t set_from_group = -1;
            if (req_data.contains("problem_set_id") && req_data["problem_set_id"].is_string())
            {
                set_from_group = std::stoi(req_data["problem_set_id"].get<std::string>());
            }
            std::cout << "[info]handleRunTese: set from group " << set_from_group << std::endl;

            std::string code = req_data["code"];
            std::string language = req_data["language"];
            int question_id = req_data["question_id"].get<int>();

            std::cout << "[info]handleRunTese: sub: \n" << code << "\n" << std::endl;
            LangType lang_type = codeStringToLang(language);
            if (lang_type == LangType::UNKNOW)
            {
                std::cerr << "[error]handleRunTese: unknow language." << std::endl;
                res.status = 200;
                resp["msg"] = u8"不支持的语言";
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            // 检查语言是否启用
            if (!LanguageManager::getInstance()->isLanguageSupported(lang_type))
            {
                std::cerr << "[error]handleRunTese: language not supported." << std::endl;
                res.status = 200;
                resp["msg"] = u8"该语言暂未启用";
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            int test_point_num = 1;
            std::vector<std::vector<std::string>> test_inputs, test_outputs;
            test_inputs.push_back({ req_data["testCase"]["input"] });
            test_outputs.push_back({ req_data["testCase"]["output"] });

            int time_limit_ms = 1000;
            int memory_limit_mb = 128;
            std::filesystem::path config_path = std::filesystem::path(PROBLEM_PATH) / std::to_string(question_id) / "config.json";
            std::ifstream config_file(config_path, std::ios::binary);
            if (config_file.is_open())
            {
                skipBOM(config_file);
                nlohmann::json config_json = nlohmann::json::parse(config_file);
                std::string lang_key = codeLangToString(lang_type);

                if (config_json.contains("time_limit") && config_json["time_limit"].contains(lang_key))
                {
                    time_limit_ms = config_json["time_limit"][lang_key].get<int>();
                }
                if (config_json.contains("memory_limit") && config_json["memory_limit"].contains(lang_key))
                {
                    memory_limit_mb = config_json["memory_limit"][lang_key].get<int>();
                }
                config_file.close();
            }

            ProgramAccess::ProgramStatus status = ProgramAccess::getInstance()->runProgram({ lang_type, time_limit_ms, memory_limit_mb }, code, test_inputs, test_outputs);

            std::string error_message = status.error_message;
            if (error_message.substr(0, 7) == " case 1")
            {
                error_message = error_message.substr(8);
            }

            if (status.status == "error")
            {
                resp["msg"] = u8"" + error_message;
                res.status = 200;
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }
            else if (status.status == "retry")
            {
                resp["msg"] = "retry";
                res.status = 200;
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            //更新题目状态
            int db_status = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id);
            if (db_status == -1)
            {
                //不存在记录
                int64_t status_id;
                if (!DataAccess::getInstance()->createDBuser_question_status(parsed_user_id, question_id, 1, 0, status_id))
                {
                    std::cerr << "[warn]handleRunTese: create question status failed." << std::endl;
                }
                db_status = 1;
            }

            //更新题目所属题库
            if (set_from_group == -1)
            {
                std::unique_ptr<PooledResultSet> group_question_res = DataAccess::getInstance()->selectDBdiscussion_groupByQuestionId(question_id);
                while (group_question_res && group_question_res->next())
                {
                    int64_t user_group_status_id = -1;
                    int discussion_group_id = group_question_res->getInt("group_id");
                    if (DataAccess::getInstance()->selectDBcount(DBType::USER_GROUP_STATUS, { "user_id", "group_id" }, { static_cast<int>(parsed_user_id), discussion_group_id }) == 0)
                    {
                        if (!DataAccess::getInstance()->createDBuser_group_status(parsed_user_id, discussion_group_id, user_group_status_id, 1))
                        {
                            std::cerr << "[error]handleRunTese: create user_group_status status failed." << std::endl;
                        }
                    }
                }
            }
            else
            {
                if (DataAccess::getInstance()->selectDBcount(DBType::USER_GROUP_STATUS, { "group_id" }, { static_cast<int>(set_from_group) }) < 1)
                {
                    int64_t group_status_id = -1;
                    if (!DataAccess::getInstance()->createDBuser_group_status(parsed_user_id, set_from_group, group_status_id, 1))
                    {
                        std::cerr << "[error]handleRunTese: create user_group_status status failed." << std::endl;
                    }
                }
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                {"output", (status.test_outputs.empty() ? "" : status.test_outputs[0])},
                {"status", status.status},
                {"executionTime", status.run_time_ms},
                {"memoryUsed", status.code_size_bytes},
                {"code", code},
                {"errorMessage", error_message}
            };
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleRunTese: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleRunTese: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleRunTese: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleRunTese: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleRunTese: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleRunTese: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"运行失败：" + std::string(e.what());
        std::cerr << "[error]handleRunTese: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"运行失败：未知系统错误";
        std::cerr << "[error]handleRunTese: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSubmitCode(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSubmitCode: sub code." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", 0},
        {"msg", u8"运行失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);

            int64_t set_from_group = -1;
            if (req_data.contains("problem_set_id") && req_data["problem_set_id"].is_string())
            {
                set_from_group = std::stoi(req_data["problem_set_id"].get<std::string>());
            }
            std::cout << "[info]handleSubmitCode: set from group " << set_from_group << std::endl;

            std::string code = req_data["code"];
            std::string language = req_data["language"];
            int question_id = req_data["question_id"].get<int>();

            std::cout << "[info]handleSubmitCode: sub: \n" << code << "\n" << std::endl;
            //属于题组，提交是否过期
            if (set_from_group != -1)
            {
                int is_allow = DataAccess::getInstance()->selectDBdiscussion_groupIsAllowSubmission(set_from_group);

                int num = DataAccess::getInstance()->selectDBcount(DBType::GROUP_QUESTION, { "group_id", "question_id" }, { static_cast<int>(set_from_group), question_id });
                if (is_allow == 0)
                {
                    std::cerr << "[error]handleSubmitCode: discussion not start." << std::endl;
                    res.status = 200;
                    resp["msg"] = u8"discussion not start";
                    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                    return;
                }
                else if (is_allow == 1)
                {
                    std::cerr << "[error]handleSubmitCode: sub code is late." << std::endl;
                    res.status = 200;
                    resp["msg"] = u8"discussion end";
                    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                    return;
                }
                else if (is_allow == -1)
                {
                    std::cerr << "[error]handleSubmitCode: something error." << std::endl;
                    res.status = -401;
                    resp["msg"] = u8"discussion end";
                    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                    return;
                }
            }

            LangType lang_type = codeStringToLang(language);
            if (lang_type == LangType::UNKNOW)
            {
                std::cerr << "[error]handleSubmitCode: unknow language." << std::endl;
                res.status = 200;
                resp["msg"] = u8"不支持的语言";
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            // 检查语言是否启用
            if (!LanguageManager::getInstance()->isLanguageSupported(lang_type))
            {
                std::cerr << "[error]handleSubmitCode: language not supported." << std::endl;
                res.status = 200;
                resp["msg"] = u8"该语言暂未启用";
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            std::vector<std::vector<std::string>> test_inputs, test_outputs;
            std::filesystem::path testpoints_dir = std::filesystem::path(TESTPOINTS_PATH) / std::to_string(question_id);
            int test_point_num = 1;
            while (true)
            {
                std::filesystem::path test_point_dir = testpoints_dir / std::to_string(test_point_num);
                std::filesystem::path input_path = test_point_dir / "input.txt";
                std::filesystem::path output_path = test_point_dir / "output.txt";

                if (!std::filesystem::exists(input_path) || !std::filesystem::exists(output_path))
                {
                    break;
                }

                std::string input_content;
                std::ifstream input_file(input_path, std::ios::binary);
                if (input_file.is_open())
                {
                    skipBOM(input_file);

                    std::string line;
                    while (std::getline(input_file, line))
                    {
                        input_content += line + "\n";
                    }
                    input_file.close();
                }

                std::string output_content;
                std::ifstream output_file(output_path, std::ios::binary);
                if (output_file.is_open())
                {
                    skipBOM(output_file);

                    std::string line;
                    while (std::getline(output_file, line))
                    {
                        output_content += line + "\n";
                    }
                    output_file.close();
                }

                test_inputs.push_back({ input_content });
                test_outputs.push_back({ output_content });
                test_point_num++;
            }
            if (test_inputs.empty())
            {
                resp["msg"] = u8"未找到测试点";
                res.status = 200;
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            int time_limit_ms = 1000;
            int memory_limit_mb = 128;
            std::filesystem::path config_path = std::filesystem::path(PROBLEM_PATH) / std::to_string(question_id) / "config.json";
            std::ifstream config_file(config_path, std::ios::binary);
            if (config_file.is_open())
            {
                skipBOM(config_file);
                nlohmann::json config_json = nlohmann::json::parse(config_file);
                std::string lang_key = codeLangToString(lang_type);

                if (config_json.contains("time_limit") && config_json["time_limit"].contains(lang_key))
                {
                    time_limit_ms = config_json["time_limit"][lang_key].get<int>();
                }
                if (config_json.contains("memory_limit") && config_json["memory_limit"].contains(lang_key))
                {
                    memory_limit_mb = config_json["memory_limit"][lang_key].get<int>();
                }
                config_file.close();
            }

            ProgramAccess::ProgramStatus status = ProgramAccess::getInstance()->runProgram({ lang_type, time_limit_ms, memory_limit_mb }, code, test_inputs, test_outputs);
            std::string error_message = status.error_message;
            if (error_message.substr(0, 5) == " case")
            {
                //隐藏测试点未通过的情况的错误信息
                error_message = "";
            }

            if (status.status == "error")
            {
                resp["msg"] = u8"" + error_message;
                res.status = 200;
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }
            else if (status.status == "retry")
            {
                resp["msg"] = "retry";
                res.status = 200;
                res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
                return;
            }

            //创建提交记录
            int64_t submisstion_id;
            if (!DataAccess::getInstance()->createDBuser_submission(parsed_user_id, question_id, code, codeLangToString(codeStringToLang(language)), codeStringToInt(status.status), (status.status == "AC" ? 10 : 0), status.run_time_ms, status.code_size_bytes, submisstion_id, (set_from_group == -1 ? 0 : 1)))//todo: 此处暂定写死每题 10 分，后续修改为自定题目分数或测试点分数
            {
                std::cerr << "[warn]handleSubmitCode: sub misstion faild." << std::endl;
            }
            else
            {
                std::cout << "[info]handleSubmitCode: sub misstion success, id: " << submisstion_id << std::endl;
            }

            //更新题目状态
            std::cerr << "[info]handleSubmitCode: update question status." << std::endl;
            int db_status = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id);
            if (db_status == -1)
            {
                //不存在记录
                int64_t status_id;
                if (!DataAccess::getInstance()->createDBuser_question_status(parsed_user_id, question_id, (status.status == "AC" ? 2 : 1), (status.status == "AC" ? 10 : 0), status_id))
                {
                    std::cerr << "[warn]handleSubmitCode: create question status failed." << std::endl;
                }
                db_status = 1;
            }
            else
            {
                //存在记录
                if (db_status != 2 && status.status == "AC")
                {
                    DataAccess::getInstance()->updateDBuser_question_statusStatus(question_id, parsed_user_id, 2);
                    std::cout << "[info]handleSubmitCode: update user_question_status." << std::endl;
                }
            }

            //更新题目所属题库状态
            std::cerr << "[info]handleSubmitCode: update user_group_status status." << std::endl;
            if (set_from_group == -1)
            {
                std::unique_ptr<PooledResultSet> group_question_res = DataAccess::getInstance()->selectDBdiscussion_groupByQuestionId(question_id);
                while (group_question_res && group_question_res->next())
                {
                    int64_t user_group_status_id = -1;
                    int discussion_group_id = group_question_res->getInt("group_id");
                    if (DataAccess::getInstance()->selectDBcount(DBType::USER_GROUP_STATUS, { "user_id", "group_id" }, { static_cast<int>(parsed_user_id), discussion_group_id }) == 0)
                    {
                        DataAccess::getInstance()->createDBuser_group_status(parsed_user_id, discussion_group_id, user_group_status_id, (status.status == "AC" ? 2 : 1));
                    }
                    else
                    {
                        if (!DataAccess::getInstance()->updateDBuser_group_statusStatus(parsed_user_id, discussion_group_id, (status.status == "AC" ? 2 : 1)))
                        {
                            std::cerr << "[error]handleSubmitCode: update user_group_status status failed." << std::endl;
                        }
                    }
                }
            }
            else
            {
                std::unique_ptr<PooledResultSet> discussion_group_res = DataAccess::getInstance()->selectDBResources<int64_t>(set_from_group, DBType::USER_GROUP_STATUS);
                if (discussion_group_res && discussion_group_res->next())
                {
                    if (discussion_group_res->getInt("status") == 1 && status.status == "AC" && DataAccess::getInstance()->updateDBuser_group_statusStatus(parsed_user_id, set_from_group, 2))
                    {
                        std::cerr << "[error]handleSubmitCode: update user_group_status status failed." << std::endl;
                    }
                }
                else
                {
                    int64_t user_group_status_id = -1;
                    if (!DataAccess::getInstance()->createDBuser_group_status(parsed_user_id, set_from_group, user_group_status_id, (status.status == "AC" ? 2 : 1)))
                    {
                        std::cerr << "[error]handleSubmitCode: create user_group_status failed." << std::endl;
                    }
                }
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                {"id", submisstion_id},
                {"question_id", question_id},
                {"userId", parsed_user_id},
                {"code", code},
                {"language", language},
                {"status", status.status},
                {"score", (status.status == "AC" ? 10 : 0)},
                {"executionTime", status.run_time_ms},
                {"memoryUsed", status.code_size_bytes},
                {"errorMessage", error_message}
            };
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleSubmitCode: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleSubmitCode: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"运行失败：" + std::string(e.what());
        std::cerr << "[error]handleSubmitCode: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"运行失败：未知系统错误";
        std::cerr << "[error]handleSubmitCode: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleCreateProblem(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleCreateProblem: create problem." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", 0},
        {"msg", u8"创建题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);

            std::string title = req_data["title"];
            int difficulty = req_data["difficulty"];
            int time_limit = req_data["timeLimit"];
            int memory_limit = req_data["memoryLimit"];
            std::string description = req_data["description"];
            std::string input_format = req_data["inputFormat"];
            std::string output_format = req_data["outputFormat"];
            std::string sample = req_data["sample"];
            DBType post_role = RoleStringToDBType(parsed_role);
            int is_audited = (post_role == DBType::ADMIN || post_role == DBType::TEACHER ? 1 : 0);

            nlohmann::json tags_json = req_data["tags"];
            nlohmann::json test_case_json = req_data["testCases"];
            nlohmann::json language_limits_json = req_data["languageLimits"];

            int64_t question_id;
            bool success = DataAccess::getInstance()->createDBquestion(title, difficulty, tags_json, is_audited, question_id);
            if (success)
            {
                std::filesystem::path problem_dir = std::filesystem::path(PROBLEM_PATH) / std::to_string(question_id);
                std::filesystem::path testpoints_dir = std::filesystem::path(TESTPOINTS_PATH) / std::to_string(question_id);
                std::filesystem::path question_path = problem_dir / "question.txt";
                if (!std::filesystem::exists(problem_dir))
                {
                    std::filesystem::create_directories(problem_dir);
                }
                if (!std::filesystem::exists(testpoints_dir))
                {
                    std::filesystem::create_directories(testpoints_dir);
                }

                std::ofstream question_file(question_path, std::ios::binary);
                if (question_file.is_open())
                {
                    const char bom[] = { 0xEF, 0xBB, 0xBF };
                    question_file.write(bom, sizeof(bom));

                    const unsigned char SECTION_QUESTION_BYTES[] = { 0x23, 0x23, 0x23, 0x20, 0xe9, 0x97, 0xae, 0xe9, 0xa2, 0x98, 0xe6, 0x8f, 0x8f, 0xe8, 0xbf, 0xb0 };
                    const unsigned char SECTION_INPUT_BYTES[] = { 0x23, 0x23, 0x23, 0x20, 0xe8, 0xbe, 0x93, 0xe5, 0x85, 0xa5, 0xe6, 0xa0, 0xbc, 0xe5, 0xbc, 0x8f };
                    const unsigned char SECTION_OUTPUT_BYTES[] = { 0x23, 0x23, 0x23, 0x20, 0xe8, 0xbe, 0x93, 0xe5, 0x87, 0xba, 0xe6, 0xa0, 0xbc, 0xe5, 0xbc, 0x8f };
                    const unsigned char SECTION_SAMPLE_BYTES[] = { 0x23, 0x23, 0x23, 0x20, 0xe6, 0xa0, 0xb7, 0xe4, 0xbe, 0x8b };

                    std::string section_question(reinterpret_cast<const char*>(SECTION_QUESTION_BYTES), sizeof(SECTION_QUESTION_BYTES));
                    std::string section_input(reinterpret_cast<const char*>(SECTION_INPUT_BYTES), sizeof(SECTION_INPUT_BYTES));
                    std::string section_output(reinterpret_cast<const char*>(SECTION_OUTPUT_BYTES), sizeof(SECTION_OUTPUT_BYTES));
                    std::string section_sample(reinterpret_cast<const char*>(SECTION_SAMPLE_BYTES), sizeof(SECTION_SAMPLE_BYTES));

                    question_file << section_question << "\n" << description << "\n";
                    question_file << section_input << "\n" << input_format << "\n";
                    question_file << section_output << "\n" << output_format << "\n";
                    question_file << section_sample << "\n" << sample << "\n";

                    question_file.close();
                }

                std::filesystem::path config_path = problem_dir / "config.json";
                std::ofstream config_file(config_path, std::ios::binary);
                if (config_file.is_open())
                {
                    nlohmann::json config_json;

                    nlohmann::json time_limit_json;
                    time_limit_json["cpp"] = time_limit * 1000;
                    time_limit_json["c"] = time_limit * 1000;
                    time_limit_json["java"] = time_limit * 1000;
                    time_limit_json["python3"] = time_limit * 1000;
                    time_limit_json["pypy3"] = time_limit * 1000;
                    time_limit_json["go"] = time_limit * 1000;
                    time_limit_json["javascript"] = time_limit * 1000;
                    config_json["time_limit"] = time_limit_json;

                    nlohmann::json memory_limit_json;
                    memory_limit_json["cpp"] = memory_limit;
                    memory_limit_json["c"] = memory_limit;
                    memory_limit_json["java"] = memory_limit;
                    memory_limit_json["python3"] = memory_limit;
                    memory_limit_json["pypy3"] = memory_limit;
                    memory_limit_json["go"] = memory_limit;
                    memory_limit_json["javascript"] = memory_limit;
                    config_json["memory_limit"] = memory_limit_json;

                    config_json["difficulty"] = difficulty;
                    config_json["tags"] = tags_json;

                    if (!language_limits_json.empty())
                    {
                        for (int i = 0; i < language_limits_json.size(); i++)
                        {
                            nlohmann::json lang_limit = language_limits_json[i];
                            std::string language = lang_limit["language"];
                            int lang_time_limit = lang_limit["timeLimit"];
                            int lang_memory_limit = lang_limit["memoryLimit"];

                            std::transform(language.begin(), language.end(), language.begin(), ::tolower);
                            if (language == "c++")
                            {
                                language = "cpp";
                            }
                            else if (language == "python")
                            {
                                language = "python3";
                            }
                            if (time_limit_json.contains(language))
                            {
                                time_limit_json[language] = lang_time_limit * 1000;
                            }
                            if (memory_limit_json.contains(language))
                            {
                                memory_limit_json[language] = lang_memory_limit;
                            }
                        }

                        config_json["time_limit"] = time_limit_json;
                        config_json["memory_limit"] = memory_limit_json;
                    }
                    const char bom[] = { 0xEF, 0xBB, 0xBF };
                    config_file.write(bom, sizeof(bom));

                    config_file << config_json.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
                    config_file.close();
                }

                int test_case_index = 1;
                for (int i = 0; i < test_case_json.size(); i++)
                {
                    nlohmann::json test_case = test_case_json[i];
                    std::string test_input = test_case["input"];
                    std::string test_output = test_case["output"];

                    std::filesystem::path test_case_dir = testpoints_dir / std::to_string(test_case_index);
                    if (!std::filesystem::exists(test_case_dir))
                    {
                        std::filesystem::create_directories(test_case_dir);
                    }

                    std::filesystem::path input_path = test_case_dir / "input.txt";
                    std::ofstream input_file(input_path);
                    if (input_file.is_open())
                    {
                        input_file << test_input;
                        input_file.close();
                    }

                    std::filesystem::path output_path = test_case_dir / "output.txt";
                    std::ofstream output_file(output_path);
                    if (output_file.is_open())
                    {
                        output_file << test_output;
                        output_file.close();
                    }

                    test_case_index++;
                }

                resp["code"] = 0;
                resp["msg"] = "success";
                resp["data"] =
                {
                    {"question_id", question_id}
                };
            }
            else
            {
                resp["msg"] = u8"创建题目失败";
                std::cerr << "[error]handleCreateProblem: create question failed." << std::endl;
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleCreateProblem: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleCreateProblem: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"创建题目失败：" + std::string(e.what());
        std::cerr << "[error]handleCreateProblem: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"创建题目失败：未知系统错误";
        std::cerr << "[error]handleCreateProblem: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleProblemApprove(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleProblemApprove: approve problem." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"通过题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (hs256::ReturnType::SUCCESS == isValid)
        {
            if (RoleStringToDBType(parsed_role) == DBType::ADMIN || RoleStringToDBType(parsed_role) == DBType::TEACHER)
            {
                bool success = DataAccess::getInstance()->updateDBquestionAudited(std::stoi(question_id), 1);

                if (success)
                {
                    resp["code"] = 0;
                    resp["msg"] = "success";
                }
                else
                {
                    resp["code"] = -500;
                    resp["msg"] = "failed";
                }
            }
            else
            {
                resp["code"] = -403;
                resp["msg"] = u8"权限不足";
                std::cerr << "[error]handleProblemApprove: not authorized" << std::endl;
            }
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token失效，请先重新登录";
            std::cerr << "[error]handleProblemApprove: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"通过题目失败：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"通过题目失败：未知系统错误";
        std::cerr << "[error]handleProblemList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleProblemReject(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleProblemReject: reject problem." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"驳回题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (hs256::ReturnType::SUCCESS == isValid)
        {
            if (RoleStringToDBType(parsed_role) == DBType::ADMIN || RoleStringToDBType(parsed_role) == DBType::TEACHER)
            {
                bool success = DataAccess::getInstance()->updateDBquestionAudited(std::stoi(question_id), 2);
                //todo: 此处暂时设为2表清理后续可以注册sql事件，定期删除即可，随后删除资源中题目文件与其测试点即可

                if (success)
                {
                    resp["code"] = 0;
                    resp["msg"] = "success";
                }
                else
                {
                    resp["code"] = -500;
                    resp["msg"] = "failed";
                }
            }
            else
            {
                resp["code"] = -403;
                resp["msg"] = u8"权限不足";
                std::cerr << "[error]handleProblemApprove: not authorized" << std::endl;
            }
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token失效，请先重新登录";
            std::cerr << "[error]handleProblemApprove: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"驳回题目失败：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"驳回题目失败：未知系统错误";
        std::cerr << "[error]handleProblemList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleProblemHide(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleProblemHide: hide problem." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"隐藏题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (hs256::ReturnType::SUCCESS == isValid)
        {
            if (RoleStringToDBType(parsed_role) == DBType::ADMIN)
            {
                bool success = DataAccess::getInstance()->updateDBquestionAudited(std::stoi(question_id), 3);

                if (success)
                {
                    resp["code"] = 0;
                    resp["msg"] = "success";
                }
                else
                {
                    resp["code"] = -500;
                    resp["msg"] = "failed";
                }
            }
            else
            {
                resp["code"] = -403;
                resp["msg"] = u8"权限不足";
                std::cerr << "[error]handleProblemHide: not authorized" << std::endl;
            }
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token失效，请先重新登录";
            std::cerr << "[error]handleProblemHide: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemHide: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemHide: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"隐藏题目失败：" + std::string(e.what());
        std::cerr << "[error]handleProblemHide: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"隐藏题目失败：未知系统错误";
        std::cerr << "[error]handleProblemHide: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSearchProblem(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handlesearchProblem: search problem by comprehensiveness." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        nlohmann::json req_data = nlohmann::json::parse(req.body);

        std::string keyword = (req_data["keyword"].empty() ? "" : req_data["keyword"]);
        std::vector<std::string> tags;
        nlohmann::json tags_json = req_data["tags"];
        for (const std::string& str : tags_json)
        {
            tags.emplace_back(str);
        }

        int difficulty = req_data["difficulty"].empty() ? -1 : req_data["difficulty"].get<int>();
        int page = req_data["page"].get<int>();
        int page_size = req_data["pageSize"].get<int>();
        bool is_include_hide = req_data["includeHidden"].get<bool>();

        std::vector<nlohmann::json> question_list;
        int total = 0;

        //尝试keyword为question_id的搜索方法
        try
        {
            size_t pos;
            int64_t keyword_id = std::stoi(keyword, &pos);

            if (pos == keyword.length())
            {
                std::unique_ptr<PooledResultSet> k_id = DataAccess::getInstance()->selectDBResources<int64_t>(keyword_id, DBType::QUESTION);
                if (k_id && k_id->next())
                {
                    std::vector<std::string> tags;
                    nlohmann::json tags_json = k_id->getString("tags");
                    for (const std::string& t : tags_json)
                    {
                        tags.emplace_back(t);
                    }

                    nlohmann::json question =
                    {
                        {"id", keyword_id},
                        {"title", k_id->getString("question_name")},
                        {"tags", tags},
                        {"difficulty", k_id->getInt("difficulty")},
                        {"completed", DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, keyword_id)}
                    };

                    question_list.emplace_back(question);
                }
            }
        }
        catch (const std::invalid_argument&)
        {
            //不是有效整数
            std::cout << "[info]handlesearchProblem: seach keyword not a question id." << std::endl;
        }
        catch (const std::out_of_range&)
        {
            //超出整数范围
            std::cout << "[info]handlesearchProblem: seach keyword is a int, but too long." << std::endl;
        }

        //以综合模糊搜索补充其他可能的题目
        std::unique_ptr<PooledResultSet> question_res1 = DataAccess::getInstance()->selectDBquestionByComprehensiveness((page - 1) * page_size, page_size - question_list.size(), keyword, tags, difficulty, 1);
        while (question_res1 && question_res1->next())
        {
            int64_t question_id = question_res1->getInt("question_id");

            std::vector<std::string> tags;
            nlohmann::json tags_json = question_res1->getString("tags");
            for (const std::string& t : tags_json)
            {
                tags.emplace_back(t);
            }

            nlohmann::json question =
            {
                {"id", question_id},
                {"title", question_res1->getString("question_name")},
                {"tags", tags},
                {"difficulty", question_res1->getInt("difficulty")},
                {"completed", DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id)}
            };

            question_list.emplace_back(question);
            total++;
        }

        if (is_include_hide)
        {
            std::unique_ptr<PooledResultSet> question_res2 = DataAccess::getInstance()->selectDBquestionByComprehensiveness((page - 1) * page_size, page_size - question_list.size(), keyword, tags, difficulty, 3);
            while (question_res2 && question_res2->next())
            {
                int64_t question_id = question_res2->getInt("question_id");

                std::vector<std::string> tags;
                nlohmann::json tags_json = question_res2->getString("tags");
                for (const std::string& t : tags_json)
                {
                    tags.emplace_back(t);
                }

                nlohmann::json question =
                {
                    {"id", question_id},
                    {"title", question_res2->getString("question_name")},
                    {"tags", tags},
                    {"difficulty", question_res2->getInt("difficulty")},
                    {"completed", DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id)}
                };

                question_list.emplace_back(question);
                total++;
            }
        }


        resp["code"] = 0;
        resp["msg"] = "success";
        resp["data"] =
        {
            {"list", question_list},
            {"total", total},
            {"page", (page - 1) * page_size},
            {"pageSize", page_size}
        };
        std::cout << "[info]handlesearchProblem: " << resp["data"].dump() << std::endl;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handlesearchProblem: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handlesearchProblem: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handlesearchProblem: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handlesearchProblem: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleAgentHelp(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleAgentHelp: get agent help." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "Post, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取智能体失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);

            std::string mode = req_data.value("mode", "");
            std::string function_id = req_data.value("functionId", "");
            std::string context = req_data.value("context", "");

            nlohmann::json question_json = req_data["question"];
            std::string title;
            int difficulty = -1;
            std::vector<std::string> tags;
            std::vector<std::pair<std::string, std::string>> test_case;
            std::string problem_content;
            std::string input_content;
            std::string output_content;


            if (question_json.contains("title") && !question_json["title"].empty())
                title = question_json["title"];

            if (question_json.contains("difficulty") && question_json["difficulty"].is_number_integer())
                difficulty = question_json["difficulty"];

            if (question_json.contains("problem_content") && !question_json["problem_content"].empty())
                problem_content = question_json["problem_content"];

            if (question_json.contains("input_content") && !question_json["input_content"].empty())
                input_content = question_json["input_content"];

            if (question_json.contains("output_content") && !question_json["output_content"].empty())
                output_content = question_json["output_content"];

            if (question_json.contains("tags") && question_json["tags"].is_array())
                tags = question_json["tags"].get<std::vector<std::string>>();



            if (question_json.contains("tags") && question_json["tags"].is_array())
            {
                for (const auto& tag : question_json["tags"])
                {
                    if (tag.is_string())
                    {
                        tags.emplace_back(tag.get<std::string>());
                    }
                }
            }

            if (question_json.contains("testCases") && question_json["testCases"].is_array())
            {
                for (const auto& test : question_json["testCases"])
                {
                    if (test.is_object() && test.contains("input") && test.contains("output"))
                    {
                        test_case.emplace_back(std::make_pair(test["input"].get<std::string>(), test["output"].get<std::string>()));
                    }
                }
            }


            if (function_id == "create-generate-test-cases")
            {
                test_case = {};
            }
            if (function_id == "create-add-tags")
            {
                tags = {};
            }
            if (function_id == "create-evaluate-difficulty")
            {
                difficulty = -1;
            }
            if (function_id == "create-complete-problem")
            {
                problem_content = "";
            }
            Problem problem =
            {
                .title = title,
                .difficulty = difficulty,
                .tags = tags,
                .problem_content = problem_content,
                .input_content = input_content,
                .output_content = output_content,
                .test_case = test_case
            };

            ModelAccess::ReturnMessage rtm;
            if (mode == "create")
            {
                std::cout << "[info]handleAgentHelp: old question: " << problemToString(problem) << std::endl;
                ModelAccess::getInstance()->agentAdditionalProblem(problem, function_id, context);
            }
            else if (mode == "review")
            {
                std::cout << "[info]handleAgentHelp: review question: " << problemToString(problem) << std::endl;
                rtm = ModelAccess::getInstance()->agentReviewProblem(problem, function_id, context);
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                {"reviewRes", rtm.isSuccess},
                {"reviewMsg", rtm.Messages},
                {"content", "success"},
                {"title", problem.title},
                {"difficulty", problem.difficulty},
                {"tags", problem.tags},
                {"description", problem.problem_content},
                {"inputFormat", problem.input_content},
                {"outputFormat", problem.output_content},
                {"testCases", problem.test_case}
            };
            std::cout << "[info]handleAgentHelp: success, new question: " << resp["data"].dump() << std::endl;
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleAgentHelp: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleAgentHelp: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleAgentHelp: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleAgentHelp: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleAgentHelp: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取智能体失败：" + std::string(e.what());
        std::cerr << "[error]handleAgentHelp: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取智能体失败：未知系统错误";
        std::cerr << "[error]handleAgentHelp: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleCreateDiscussionGroup(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleCreateDiscussionGroup: create discussion group." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "Post, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"创建题组失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string question_id = req.matches[1];

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);
            //获取属性存储
            std::string name = req_data["name"];
            std::string description = req_data["description"];

            //是否为竞赛题组
            int is_contest = 0;
            if (req_data.contains("isContest") && req_data["isContest"].is_number())
                is_contest = req_data["isContest"].get<int>();

            std::cout << "[debug]handleCreateDiscussionGroup: is contest " << is_contest << std::endl;

            std::string start_time = (req_data["startTime"].empty() ? "" : req_data["startTime"]);
            std::string end_time = (req_data["endTime"].empty() ? "" : req_data["endTime"]);

            std::vector<int64_t> vec_question_id;
            nlohmann::json problems_id = req_data["problemIds"];
            for (const int& id : problems_id)
            {
                vec_question_id.emplace_back(static_cast<int64_t>(id));
            }

            int64_t discussion_group_id;
            if (DataAccess::getInstance()->createDBdiscussion_group(parsed_user_id, name, description, start_time, end_time, is_contest, vec_question_id, discussion_group_id))
            {
                resp["code"] = 0;
                resp["msg"] = "success";
            }
            else
            {
                resp["code"] = 0;
                resp["msg"] = "failed";
            }
            std::cout << "[info]handleCreateDiscussionGroup: is contest " << is_contest << std::endl;
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleCreateDiscussionGroup: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleCreateDiscussionGroup: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleCreateDiscussionGroup: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleCreateDiscussionGroup: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleCreateDiscussionGroup: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"创建题组失败：" + std::string(e.what());
        std::cerr << "[error]handleCreateDiscussionGroup: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"创建题组失败：未知系统错误";
        std::cerr << "[error]handleCreateDiscussionGroup: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetProblemList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleProblemList: get problem list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        int page = 1;
        int page_size = 15;

        const httplib::Params::const_iterator& page_it = req.params.find("page");
        if (page_it != req.params.end())
        {
            page = std::stoi(page_it->second);
        }

        const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
        if (page_size_it != req.params.end())
        {
            page_size = std::stoi(page_size_it->second);
        }

        std::unique_ptr<PooledResultSet> questions_res = DataAccess::getInstance()->selectDBResourcesByRange(DBType::QUESTION, (page - 1) * page_size, page_size, 1);
        std::vector<nlohmann::json> question_list;

        int total = 0;
        while (questions_res && questions_res->next())
        {
            nlohmann::json question;
            question["id"] = questions_res->getString("question_id");
            question["title"] = questions_res->getString("question_name");
            question["difficulty"] = questions_res->getInt("difficulty");

            std::string tags_str = questions_res->getString("tags");
            if (!tags_str.empty())
            {
                question["tags"] = nlohmann::json::parse(tags_str);
            }

            int db_status = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, questions_res->getInt("question_id"));
            question["completed"] = (db_status == -1 ? 0 : db_status);//0未开始，1未通过，2已通过

            question_list.emplace_back(question);
            total++;
        }

        resp["code"] = 0;
        resp["msg"] = "success";
        resp["data"] =
        {
            {"total", total},
            {"list", question_list},
            {"page", page},
            {"pageSize", page_size}
        };

    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handleProblemList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handleProblemList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetHideProblemList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetHideProblemList: get hide problem list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        int page = 1;
        int page_size = 15;

        const httplib::Params::const_iterator& page_it = req.params.find("page");
        if (page_it != req.params.end())
        {
            page = std::stoi(page_it->second);
        }

        const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
        if (page_size_it != req.params.end())
        {
            page_size = std::stoi(page_size_it->second);
        }

        std::unique_ptr<PooledResultSet> questions_res = DataAccess::getInstance()->selectDBResourcesByRange(DBType::QUESTION, (page - 1) * page_size, page_size, 3);
        std::vector<nlohmann::json> question_list;

        int total = 0;
        while (questions_res && questions_res->next())
        {
            nlohmann::json question;
            question["id"] = questions_res->getString("question_id");
            question["title"] = questions_res->getString("question_name");
            question["difficulty"] = questions_res->getInt("difficulty");

            std::string tags_str = questions_res->getString("tags");
            if (!tags_str.empty())
            {
                question["tags"] = nlohmann::json::parse(tags_str);
            }

            int db_status = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, questions_res->getInt("question_id"));
            question["completed"] = (db_status == -1 ? 0 : db_status);//0未开始，1未通过，2已通过 3隐藏

            question_list.emplace_back(question);
            total++;
        }

        resp["code"] = 0;
        resp["msg"] = "success";
        resp["data"] =
        {
            {"total", total},
            {"list", question_list},
            {"page", page},
            {"pageSize", page_size}
        };

    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetHideProblemList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetHideProblemList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetHideProblemList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handleGetHideProblemList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetProblem(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetProblem: get problem." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        int64_t parsed_user_id = -1;
        std::string parsed_role = "STUDENT";
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);


        std::string problem_id = req.matches[1];
        std::unique_ptr<PooledResultSet> question_res = DataAccess::getInstance()->selectDBResources<int>(std::stoi(problem_id), DBType::QUESTION);

        if (question_res && question_res->next())
        {
            resp["code"] = 0;

            int64_t set_from_group = -1;
            const httplib::Params::const_iterator& problem_set_id_it = req.params.find("problem_set_id");
            if (problem_set_id_it != req.params.end())
            {
                set_from_group = std::stoi(problem_set_id_it->second);

                int status = DataAccess::getInstance()->selectDBdiscussion_groupIsAllowSubmission(set_from_group);
                std::cout << "[info]handleGetProblem: set from group " << set_from_group << ", status " << status << std::endl;
                switch (status)
                {
                case -1:
                case 0:
                {
                    resp["msg"] = "error";
                    std::cout << "[info]handleGetProblem: get discussion failed." << std::endl;
                    return;
                }
                default:
                    break;
                }
            }

            int is_audited = -1;
            if (question_res->getInt("is_audited") != 1 && RoleStringToDBType(parsed_role) == DBType::STUDENT && set_from_group == -1)
            {
                resp["msg"] = "error";
                std::cout << "[info]handleGetProblem: get discussion failed." << std::endl;
                return;
            }
            if (set_from_group != -1) is_audited = question_res->getInt("is_audited");
            std::cout << "[info]handleGetProblem: is_audited" << is_audited << std::endl;

            std::string tags_str = question_res->getString("tags");
            nlohmann::json tags_json;
            if (!tags_str.empty())
            {
                tags_json = nlohmann::json::parse(tags_str);
            }


            int question_id = question_res->getInt("question_id");
            std::string cont;
            std::filesystem::path file_path = std::filesystem::path(PROBLEM_PATH) / std::to_string(question_id) / "question.txt";
            std::filesystem::path json_path = std::filesystem::path(PROBLEM_PATH) / std::to_string(question_id) / "config.json";
            std::ifstream file(file_path, std::ios::binary);
            if (!file.is_open())
            {
                std::cerr << "[error]handleGetProblem: failed to open file: " << file_path << std::endl;
            }
            else
            {
                skipBOM(file);

                std::string line;
                while (std::getline(file, line))
                {
                    cont += line + "\n";
                }
                file.close();
                std::cout << "[debug]handleGetProblem: successfully read file: " << file_path << std::endl;
            }

            std::ifstream ifs(json_path, std::ios::binary);

            skipBOM(ifs);
            nlohmann::json j2 = nlohmann::json::parse(ifs);
            nlohmann::json language_limits;
            std::unordered_map<std::string, std::string> lang_name_map =
            {
                {"cpp", "C++"},
                {"c++", "C++"},
                {"c", "C"},
                {"java", "Java"},
                {"python3", "Python3"},
                {"pypy3", "PyPy3"},
                {"go", "Go"},
                {"javascript", "JavaScript"}
            };
            if (j2.contains("time_limit"))
            {
                for (auto& [lang, limit] : j2["time_limit"].items())
                {
                    std::string lang_name = lang;
                    if (lang_name_map.find(lang) != lang_name_map.end())
                    {
                        lang_name = lang_name_map[lang];
                    }

                    nlohmann::json lang_limit;
                    lang_limit["timeLimit"] = limit.get<int>() / 1000;//单位转换，ms换为s

                    if (j2.contains("memory_limit") && j2["memory_limit"].contains(lang))
                    {
                        lang_limit["memoryLimit"] = j2["memory_limit"][lang].get<int>();
                    }

                    language_limits[lang_name] = lang_limit;
                }
            }
            ifs.close();

            std::string question_desc, input_format, output_format, sample_input;
            util::splitQuestionContent(cont, question_desc, input_format, output_format, sample_input);

            resp["msg"] = "success";
            resp["data"] =
            {
                {"id", question_id},
                {"title", std::string(question_res->getString("question_name"))},
                {"description", question_desc},
                {"inputFormat", input_format},
                {"outputFormat", output_format},
                {"sample", sample_input},
                {"difficulty", question_res->getInt("difficulty")},
                {"languageLimits", language_limits},
                {"tags", tags_json},
                {"audited", is_audited}
            };

            std::cout << "[info]handleGetProblem: " << resp["data"] << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetProblem: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetProblem: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]handleGetProblem: exception while reading file: " << e.what() << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetQuestionStats(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetQuestionStats: get stats." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", 0},
        {"msg", u8"运行失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        int total_problems = DataAccess::getInstance()->selectDBcount(DBType::QUESTION);
        int supported_languages = 3;//static_cast<int>(LangType::UNKNOW);//其他语言未部署
        int total_submissions = DataAccess::getInstance()->selectDBcount(DBType::USER_SUBMISSION);
        int judge_accuracy = 98;

        resp["data"] =
        {
            {"totalProblems", total_problems},
            {"supportedLanguages", supported_languages},
            {"totalSubmissions", total_submissions},
            {"judgeAccuracy", judge_accuracy}
        };
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetQuestionStats: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetQuestionStats: " << e.what() << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetAuditedList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetAuditedList: get audited problem list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取审核题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        resp["code"] = 0;
        resp["msg"] = "success";

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS && RoleStringToDBType(parsed_role) == DBType::ADMIN)
        {
            int page = 1;
            int page_size = 15;

            const httplib::Params::const_iterator& page_it = req.params.find("page");
            if (page_it != req.params.end())
            {
                page = std::stoi(page_it->second);
            }

            const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
            if (page_size_it != req.params.end())
            {
                page_size = std::stoi(page_size_it->second);
            }

            std::unique_ptr<PooledResultSet> questions_res = DataAccess::getInstance()->selectDBResourcesByRange(DBType::QUESTION, (page - 1) * page_size, page_size, 0);
            std::vector<nlohmann::json> question_list;

            int total = 0;
            while (questions_res && questions_res->next())
            {
                nlohmann::json question;
                question["id"] = questions_res->getString("question_id");
                question["title"] = questions_res->getString("question_name");
                question["difficulty"] = questions_res->getInt("difficulty");
                question["createdAt"] = questions_res->getString("create_time");

                std::string tags_str = questions_res->getString("tags");
                if (!tags_str.empty())
                {
                    question["tags"] = nlohmann::json::parse(tags_str);
                }

                question_list.emplace_back(question);
                total++;
            }
            resp["data"] =
            {
                {"total", total},
                {"list", question_list},
                {"page", page},
                {"pageSize", page_size}
            };

        }
        else
        {
            resp["code"] = -403;
            resp["msg"] = u8"权限不足";
            std::cerr << "[error]handleGetAuditedList: not authorized" << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取审核题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取审核题目失败：未知系统错误";
        std::cerr << "[error]handleGetAuditedList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetRejectedList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetRejectedList: get rejected problem list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取驳回题目失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        resp["code"] = 0;
        resp["msg"] = "success";

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS && RoleStringToDBType(parsed_role) == DBType::ADMIN)
        {
            int page = 1;
            int page_size = 15;

            const httplib::Params::const_iterator& page_it = req.params.find("page");
            if (page_it != req.params.end())
            {
                page = std::stoi(page_it->second);
            }

            const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
            if (page_size_it != req.params.end())
            {
                page_size = std::stoi(page_size_it->second);
            }

            std::unique_ptr<PooledResultSet> questions_res = DataAccess::getInstance()->selectDBResourcesByRange(DBType::QUESTION, (page - 1) * page_size, page_size, 2);
            std::vector<nlohmann::json> question_list;

            int total = 0;
            while (questions_res && questions_res->next())
            {
                nlohmann::json question;
                question["id"] = questions_res->getString("question_id");
                question["title"] = questions_res->getString("question_name");
                question["difficulty"] = u8"" + std::string(codeIntToStringDifficulty(questions_res->getInt("difficulty")));
                question["createdAt"] = questions_res->getString("create_time");

                std::string tags_str = questions_res->getString("tags");
                if (!tags_str.empty())
                {
                    question["tags"] = nlohmann::json::parse(tags_str);
                }

                question_list.emplace_back(question);
                total++;
            }
            resp["data"] =
            {
                {"total", total},
                {"list", question_list},
                {"page", page},
                {"pageSize", page_size}
            };

        }
        else
        {
            resp["code"] = -403;
            resp["msg"] = u8"权限不足";
            std::cerr << "[error]handleGetRejectedList: not authorized" << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetRejectedList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetRejectedList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取审核题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetRejectedList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取审核题目失败：未知系统错误";
        std::cerr << "[error]handleGetRejectedList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetSubmissionList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetSubmissionList: get submission list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取提交记录失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        int page = 1;
        int page_size = 15;
        int64_t id = -1;

        const httplib::Params::const_iterator& page_it = req.params.find("page");
        if (page_it != req.params.end())
        {
            page = std::stoi(page_it->second);
        }

        const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
        if (page_size_it != req.params.end())
        {
            page_size = std::stoi(page_size_it->second);
        }

        const httplib::Params::const_iterator& id_size_it = req.params.find("userId");
        if (id_size_it != req.params.end())
        {
            id = std::stoi(id_size_it->second);
        }

        if (id == -1)
        {
            std::string token = req.get_header_value("Authorization");
            token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

            std::string parsed_role;
            hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, id, parsed_role);
        }

        std::cout << "[info]handleGetSubmissionList: select submission id " << id << std::endl;
        std::unique_ptr<PooledResultSet> submission_db = DataAccess::getInstance()->selectDBuser_submissionByRange(id, (page - 1) * page_size, page_size);
        std::vector<std::string> submisstion;
        //解析查找内容
        int total = 0;
        while (submission_db && submission_db->next())
        {
            nlohmann::json json_db;
            json_db =
            {
                {"id", submission_db->getInt("submission_id")},
                {"question_id", submission_db->getInt("question_id")},
                {"userId", submission_db->getInt("user_id")},
                {"code", submission_db->getString("code")},
                {"language", submission_db->getString("language")},
                {"status", codeResultToString(codeIntToCodeResult(submission_db->getInt("status")))},
                {"score", submission_db->getInt("score")},
                {"executionTime", submission_db->getInt("execution_time")},
                {"memoryUsed", submission_db->getInt("memory_used")},
                {"errorMessage", ""},//todo: 错误信息暂不保留, 后续可以增加这里的内容
                {"createdAt", submission_db->getString("submit_at")}
            };
            std::cout << "[info]handleGetSubmissionList: " << json_db.dump() << std::endl;

            submisstion.emplace_back(json_db.dump());
            total++;
        }

        //响应前端请求
        resp["data"] =
        {
            {"list", submisstion},
            {"total", total},
            {"page", page},
            {"pageSize", page_size}
        };
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取提交记录失败：" + std::string(e.what());
        std::cerr << "[error]handleGetAuditedList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取提交记录失败：未知系统错误";
        std::cerr << "[error]handleGetAuditedList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetUserList(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetUserList: get user list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取用户失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS && RoleStringToDBType(parsed_role) == DBType::ADMIN)
        {

            std::unique_ptr<PooledResultSet> user_db = DataAccess::getInstance()->selectDBResourcesAll(DBType::USER);
            std::vector<nlohmann::json> user_list;
            while (user_db && user_db->next())
            {
                nlohmann::json user;
                user =
                {
                    {"id", user_db->getInt("user_id")},
                    {"account", user_db->getString("account")},
                    {"nickname", user_db->getString("nickname")},
                    {"role", DBTypeToRoleString(RoleIntToDBType(user_db->getInt("role")))}
                };

                user_list.emplace_back(user);
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] = user_list;
            std::cout << "[info]handleGetUserList: " << resp["data"] << std::endl;
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleGetUserList: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleGetUserList: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleGetUserList: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleGetUserList: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetUserList: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetUserList: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetUserList: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handleGetUserList: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetUser(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetUser: get user info." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取信息失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string user_id_str = req.matches[1];
        std::unique_ptr<PooledResultSet> user = DataAccess::getInstance()->selectDBResources<int64_t>(std::stoi(user_id_str), DBType::USER);
        std::unique_ptr<PooledResultSet> role_user = nullptr;
        if (user && user->next())
        {
            role_user = DataAccess::getInstance()->selectDBResources<int64_t>(static_cast<int64_t>(user->getInt("user_id")), RoleIntToDBType(user->getInt("role")), "user_id");
        }

        if (role_user && role_user->next())
        {
            std::string token = req.get_header_value("Authorization");//获取token信息
            if (token.substr(0, 7) == "Bearer ")
            {
                token = token.substr(7);
            }
            int64_t parsed_user_id;
            std::string parsed_role;
            hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

            std::string real_name = "";
            std::string school_no = "";
            std::string role = "";
            std::string school = "";
            int grade = 0;
            int submissions_count = 0;
            int solved_count = 0;
            nlohmann::json submission_stats =
            {
                {"c", 0},
                {"c++", 0},
                {"python3", 0},
                {"python2", 0},
                {"java", 0},
                {"pypy3", 0},
                {"go", 0},
                {"javascript", 0}
            };

            int info_display_setting = user->getInt("info_display_setting");
            //按位从低到高为：1角色，2学校，3年级，4提交总数，5已解决题目数，6提交语言统计, 7学号, 8真名
            int digit = 1, admin_flag = 0;
            while (info_display_setting || (RoleStringToDBType(parsed_role) == DBType::ADMIN && !admin_flag))
            {
                if (info_display_setting & 1 || RoleStringToDBType(parsed_role) == DBType::ADMIN)
                {
                    switch (digit)
                    {
                    case 1:
                        role = DBTypeToRoleString(RoleIntToDBType(user->getInt("role")));
                        break;
                    case 2:
                        school = (RoleIntToDBType(user->getInt("role")) != DBType::ADMIN ? role_user->getString("school") : "");
                        break;
                    case 3:
                        grade = (RoleIntToDBType(user->getInt("role")) == DBType::STUDENT ? role_user->getInt("grade") : 0);
                        break;
                    case 4:
                        submissions_count = DataAccess::getInstance()->selectDBcount(DBType::USER_SUBMISSION, { "user_id" }, { user->getInt("user_id") });
                        break;
                    case 5:
                        solved_count = DataAccess::getInstance()->selectDBcount(DBType::USER_SUBMISSION, { "user_id", "status" }, { user->getInt("user_id"), 3 });
                        break;
                    case 6:
                    {
                        std::unique_ptr<PooledResultSet> res = DataAccess::getInstance()->selectDBuser_submission({ "user_id" }, { user->getInt("user_id") });

                        while (res && res->next())
                        {
                            submission_stats[res->getString("language")] = submission_stats[res->getString("language")] + 1;
                        }
                    }
                    break;
                    case 7:
                        school_no = (RoleIntToDBType(user->getInt("role")) == DBType::STUDENT ? role_user->getString("student_no") : "");
                        break;
                    case 8:
                        real_name = (RoleIntToDBType(user->getInt("role")) != DBType::ADMIN ? role_user->getString("real_name") : "");
                        break;
                    default:
                        admin_flag = 1;
                        break;
                    }
                }
                digit++;
                info_display_setting = info_display_setting >> 1;
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                {"id", user->getInt("user_id")},
                {"nickname", user->getString("nickname")},
                {"role", role},
                {"school", school},
                {"grade", grade},
                {"userDesc", user->getString("user_desc")},
                {"submissionsCount", submissions_count},
                {"solvedCount", solved_count},
                {"submissionStats", submission_stats},
                {"userRoleNo", school_no},
                {"realName", real_name}
            };
            std::cout << "[info]handleGetUser: user" << resp["data"].dump() << std::endl;
        }
        else
        {
            resp["code"] = -500;
            resp["msg"] = u8"找不到用户或角色";
            std::cerr << "[error]handleGetUser: can't find user." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetUser: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetUser: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleGetUser: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleGetUser: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetUserSelf(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetUserSelf: get user self info." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取信息失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        int* time_left = new int();

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role, time_left);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            std::unique_ptr<PooledResultSet> res = DataAccess::getInstance()->selectDBResources<int64_t>(parsed_user_id, DBType::USER);
            if (res && res->next())
            {
                //若token剩余时长不足，但用户较为活跃度则可重新生成Token延长时间
                if (*time_left < static_cast<int>(TOKEN_VALID_TIME_SET * TOKEN_REGENERATE_PERCENT))
                {
                    token = hs256::generateToken(SECURE_SECRET_KEY, parsed_user_id, parsed_role);
                }

                resp["code"] = 0;
                resp["msg"] = "success";
                resp["data"] =
                {
                    {"id", parsed_user_id},
                    {"account", res->getString("account")},
                    {"nickname", res->getString("nickname")},
                    {"role", parsed_role},
                    {"token", token}
                };
            }
            else
            {
                resp["code"] = -500;
                resp["msg"] = u8"数据库查找失败";
                std::cerr << "[error]handleGetUserSelf: can't find user id: " << parsed_user_id << std::endl;
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSubmitCode: token failed." << std::endl;
        }

        if (time_left) delete time_left;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetUserSelf: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleGetUserSelf: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleGetUserSelf: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetProvacy(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetProvacy: get user provacy." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取信息失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);
            bool show_role = false, show_school = false, show_grade = false, show_submissions_count = false, show_solved_count = false, show_submission_stats = false, show_user_role_no = false, show_real_name = false;
            std::unique_ptr<PooledResultSet> user = DataAccess::getInstance()->selectDBResources<int64_t>(parsed_user_id, DBType::USER);
            if (user && user->next())
            {
                int info_display_setting = user->getInt("info_display_setting");
                //按位从低到高为：1角色，2学校，3年级，4提交总数，5已解决题目数，6提交语言统计, 7学号, 8真名
                //这里压行数或者压速度可以直接位运算与1, 2, 4, ..., 128即可
                int digit = 1;
                while (info_display_setting)
                {
                    if (info_display_setting & 1 || RoleStringToDBType(parsed_role) == DBType::ADMIN)
                    {
                        switch (digit)
                        {
                        case 1:
                            show_role = true;
                            break;
                        case 2:
                            show_school = true;
                            break;
                        case 3:
                            show_grade = true;
                            break;
                        case 4:
                            show_submissions_count = true;
                            break;
                        case 5:
                            show_solved_count = true;
                            break;
                        case 6:
                            show_submission_stats = true;
                            break;
                        case 7:
                            show_user_role_no = true;
                            break;
                        case 8:
                            show_real_name = true;
                            break;
                        default:
                            break;
                        }
                    }
                    digit++;
                    info_display_setting = info_display_setting >> 1;
                }

                resp["code"] = 0;
                resp["msg"] = "success";
                resp["data"] =
                {
                    {"showRole", show_role},
                    {"showSchool", show_school},
                    {"showGrade", show_grade},
                    {"showSubmissionsCount", show_submissions_count},
                    {"showSolvedCount", show_solved_count},
                    {"showSubmissionStats", show_submission_stats},
                    {"showUserRoleNo", show_user_role_no},
                    {"showRealName", show_real_name}
                };
            }
            else
            {
                resp["code"] = -500;
                resp["msg"] = u8"数据库查找失败";
                std::cerr << "[error]handleGetProvacy: can't find user id: " << parsed_user_id << std::endl;
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleGetProvacy: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleGetProvacy: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleGetProvacy: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleGetProvacy: token failed." << std::endl;
        }
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetProvacy: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleGetProvacy: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleGetProvacy: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetTrainingStats(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetTrainingStats: get training stats." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "Get, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        int total_problem_sets = 0, completed_problem_sets = 0, total_problems = 0;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            total_problem_sets = DataAccess::getInstance()->selectDBcount(DBType::DISCUSSION_GROUP, { "creator_id" }, { static_cast<int>(parsed_user_id) });
            completed_problem_sets = DataAccess::getInstance()->selectDBcount(DBType::USER_GROUP_STATUS, { "user_id", "status" }, { static_cast<int>(parsed_user_id), 2 });
            total_problems = DataAccess::getInstance()->selectDBcount(DBType::USER_QUESTION_STATUS, { "user_id", "status" }, { static_cast<int>(parsed_user_id), 2 });
        }

        resp["code"] = 0;
        resp["msg"] = "success";
        resp["data"] =
        {
            {"totalProblemSets", total_problem_sets},
            {"completedProblemSets", completed_problem_sets},
            {"totalProblems", total_problems}
        };
        std::cout << "[info]handleGetTrainingStats: " << resp["data"].dump() << std::endl;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetTrainingStats: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetTrainingStats: " << e.what() << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetTrainingActive(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetTrainingActive: get user training active." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "Get, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS)
        {
            int is_contest = 0;
            const httplib::Params::const_iterator& is_contest_it = req.params.find("isContest");
            if (is_contest_it != req.params.end())
            {
                is_contest = std::stoi(is_contest_it->second);
            }
            std::cout << "[info]handleGetTrainingActive: is contest " << is_contest << std::endl;

            std::vector<nlohmann::json> group_list;
            std::unique_ptr<PooledResultSet> group_res = DataAccess::getInstance()->selectDBdiscussion_groupByStatus(parsed_user_id, 1, is_contest);
            while (group_res && group_res->next())
            {
                nlohmann::json question_group;

                question_group["id"] = group_res->getInt("group_id");
                question_group["name"] = group_res->getString("group_name");
                question_group["description"] = group_res->getString("group_desc");
                question_group["totalProblems"] = DataAccess::getInstance()->selectDBcount(DBType::GROUP_QUESTION, { "group_id" }, { group_res->getInt("group_id") });

                group_list.emplace_back(question_group);
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                group_list
            };
            std::cout << "[info]handleGetTrainingActive: " << resp["data"].dump() << std::endl;
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleGetTrainingActive: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleGetTrainingActive: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleGetTrainingActive: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleGetTrainingActive: token failed." << std::endl;
        }

    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetTrainingActive: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetTrainingActive: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取审核题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetTrainingActive: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取审核题目失败：未知系统错误";
        std::cerr << "[error]handleGetTrainingActive: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetQuestionGroup(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetQuestionGroup: get question group list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题目组失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        int page = 1;
        int page_size = 15;

        const httplib::Params::const_iterator& page_it = req.params.find("page");
        if (page_it != req.params.end())
        {
            page = std::stoi(page_it->second);
        }

        const httplib::Params::const_iterator& page_size_it = req.params.find("pageSize");
        if (page_size_it != req.params.end())
        {
            page_size = std::stoi(page_size_it->second);
        }

        int is_contest = 0;
        const httplib::Params::const_iterator& is_contest_it = req.params.find("isContest");
        if (is_contest_it != req.params.end())
        {
            is_contest = std::stoi(is_contest_it->second);
        }
        std::cout << "[info]handleGetQuestionGroup: is contest " << is_contest << std::endl;

        int total = 0;
        std::vector<nlohmann::json> group_list;
        std::unique_ptr<PooledResultSet> group_res = DataAccess::getInstance()->selectDBResourcesByRange(DBType::DISCUSSION_GROUP, (page - 1) * page_size, page_size, -1, is_contest);

        while (group_res && group_res->next())
        {
            nlohmann::json question;
            question["id"] = group_res->getInt("group_id");
            question["name"] = group_res->getString("group_name");
            question["description"] = group_res->getString("group_desc");
            question["totalProblems"] = DataAccess::getInstance()->selectDBcount(DBType::GROUP_QUESTION, { "group_id" }, { group_res->getInt("group_id") });
            question["startTime"] = group_res->getString("start_time");
            question["endTime"] = group_res->getString("end_time");

            group_list.emplace_back(question);
            total++;
        }

        resp["code"] = 0;
        resp["msg"] = "success";
        resp["data"] =
        {
            {"list", group_list},
            {"total", total},
            {"page", page},
            {"pageSize", page_size}
        };
        std::cout << "[info]handleGetQuestionGroup: " << resp["data"].dump() << std::endl;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetQuestionGroup: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetQuestionGroup: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetQuestionGroup: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handleGetQuestionGroup: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetGroupHistory(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetGroupHistory: get question group list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题目组失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id;
        std::string parsed_role;
        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (isValid == hs256::ReturnType::SUCCESS)
        {
            int is_contest = 0;
            const httplib::Params::const_iterator& is_contest_it = req.params.find("isContest");
            if (is_contest_it != req.params.end())
            {
                is_contest = std::stoi(is_contest_it->second);
            }
            std::cout << "[info]handleGetGroupHistory: is contest " << is_contest << std::endl;

            std::unique_ptr<PooledResultSet> group_res1 = DataAccess::getInstance()->selectDBdiscussion_groupByStatus(parsed_user_id, 1, is_contest);
            std::unique_ptr<PooledResultSet> group_res2 = DataAccess::getInstance()->selectDBdiscussion_groupByStatus(parsed_user_id, 2, is_contest);
            std::vector<nlohmann::json> group_list;

            while (group_res1 && group_res1->next())
            {
                nlohmann::json gp = nlohmann::json::object();
                gp["id"] = group_res1->getInt("group_id");
                gp["date"] = group_res1->getString("last_activity_time");

                gp["problemSetId"] = group_res1->getInt("group_id");
                std::unique_ptr<PooledResultSet> discussion_group_res = DataAccess::getInstance()->selectDBResources<int64_t>(group_res1->getInt("group_id"), DBType::DISCUSSION_GROUP);
                if (discussion_group_res && discussion_group_res->next())
                {
                    gp["problemSetName"] = discussion_group_res->getString("group_name");
                }
                else
                {
                    std::cerr << "[warn]handleGetGroupHistory: question id " << group_res1->getInt("group_id") << "lost." << std::endl;
                    continue;
                }


                gp["totalProblems"] = DataAccess::getInstance()->selectDBcount(DBType::GROUP_QUESTION, { "group_id" }, { group_res1->getInt("group_id") });
                gp["status"] = 1;

                group_list.emplace_back(gp);
            }

            while (group_res2 && group_res2->next())
            {
                nlohmann::json gp = {};
                gp["id"] = group_res2->getInt("group_id");
                gp["date"] = group_res2->getString("last_activity_time");
                gp["problemSetId"] = group_res2->getInt("group_id");

                std::unique_ptr<PooledResultSet> discussion_group_res = DataAccess::getInstance()->selectDBResources<int64_t>(group_res2->getInt("group_id"), DBType::DISCUSSION_GROUP);
                if (discussion_group_res && discussion_group_res->next())
                {
                    gp["problemSetName"] = discussion_group_res->getString("group_name");
                }
                else
                {
                    std::cerr << "[warn]handleGetGroupHistory: question id " << group_res2->getInt("group_id") << "lost." << std::endl;
                    continue;
                }


                gp["totalProblems"] = DataAccess::getInstance()->selectDBcount(DBType::GROUP_QUESTION, { "group_id" }, { group_res2->getInt("group_id") });
                gp["status"] = 2;

                group_list.emplace_back(gp);
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                group_list
            };
            std::cout << "[info]handleGetGroupHistory: " << resp["data"].dump() << std::endl;
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleGetGroupHistory: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleGetGroupHistory: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleGetGroupHistory: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleGetGroupHistory: token failed." << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupHistory: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupHistory: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题目失败：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupHistory: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取题目失败：未知系统错误";
        std::cerr << "[error]handleGetGroupHistory: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetGroupInfo(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetGroupInfo: get group info." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取题组失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string group_id_str = req.matches[1];
        std::unique_ptr<PooledResultSet> discussion_group_res = DataAccess::getInstance()->selectDBResources<int64_t>(std::stoi(group_id_str), DBType::DISCUSSION_GROUP);

        std::string token = req.get_header_value("Authorization");//获取token信息
        if (token.substr(0, 7) == "Bearer ")
        {
            token = token.substr(7);
        }
        int64_t parsed_user_id = -1;
        std::string parsed_role;

        hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);

        if (discussion_group_res && discussion_group_res->next())
        {
            int id = discussion_group_res->getInt("group_id");
            std::string name = discussion_group_res->getString("group_name");
            std::string description = discussion_group_res->getString("group_desc");
            std::string start_time = discussion_group_res->getString("start_time");
            std::string end_time = discussion_group_res->getString("end_time");
            std::string create_time = discussion_group_res->getString("create_time");
            int creator_id = discussion_group_res->getInt("creator_id");
            int is_contest_db = discussion_group_res->getInt("is_competition");

            int is_contest = 0;
            const httplib::Params::const_iterator& is_contest_it = req.params.find("isContest");
            if (is_contest_it != req.params.end())
            {
                is_contest = std::stoi(is_contest_it->second);
            }
            std::cout << "[info]handleGetGroupInfo: is contest " << is_contest << std::endl;

            std::vector<nlohmann::json> problems_list;
            if ((is_contest == 1 || is_contest_db == 1) && RoleStringToDBType(parsed_role) != DBType::ADMIN)
            {
                int status = DataAccess::getInstance()->selectDBdiscussion_groupIsAllowSubmission(id);

                std::cout << "[info]handleGetGroupInfo: now status " << status << std::endl;
                switch (status)
                {
                case 1:     //已结束
                case 2:     //进行中
                {
                    std::unique_ptr<PooledResultSet> question_res = DataAccess::getInstance()->selectDBquestionByGroupId(id);
                    while (question_res && question_res->next())
                    {
                        nlohmann::json qs;

                        int question_id = question_res->getInt("question_id");
                        qs["id"] = question_id;
                        qs["title"] = question_res->getString("question_name");
                        qs["difficulty"] = question_res->getInt("difficulty");
                        std::vector<std::string> tags;
                        nlohmann::json tags_json = question_res->getString("tags");
                        for (const std::string& str : tags_json)
                        {
                            tags.emplace_back(str);
                        }
                        qs["tags"] = tags;

                        if (parsed_user_id == -1)
                        {
                            qs["completed"] = 0;
                        }
                        else
                        {
                            int status_num = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id);

                            qs["completed"] = (status_num == -1 ? 0 : status_num);
                        }

                        problems_list.emplace_back(qs);
                    }
                }
                break;
                default:
                    break;
                }
            }
            else if ((is_contest_db != 1 && is_contest != 1) || RoleStringToDBType(parsed_role) == DBType::ADMIN)
            {
                std::unique_ptr<PooledResultSet> question_res = DataAccess::getInstance()->selectDBquestionByGroupId(id);
                while (question_res && question_res->next())
                {
                    nlohmann::json qs;

                    int question_id = question_res->getInt("question_id");
                    qs["id"] = question_id;
                    qs["title"] = question_res->getString("question_name");
                    qs["difficulty"] = question_res->getInt("difficulty");
                    std::vector<std::string> tags;
                    nlohmann::json tags_json = question_res->getString("tags");
                    for (const std::string& str : tags_json)
                    {
                        tags.emplace_back(str);
                    }
                    qs["tags"] = tags;

                    if (parsed_user_id == -1)
                    {
                        qs["completed"] = 0;
                    }
                    else
                    {
                        int status_num = DataAccess::getInstance()->selectDBuser_question_statusForStatus(parsed_user_id, question_id);

                        qs["completed"] = (status_num == -1 ? 0 : status_num);
                    }

                    problems_list.emplace_back(qs);
                }
            }

            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] =
            {
                {"id", id},
                {"name", name},
                {"description", description},
                {"startTime", start_time},
                {"endTime", end_time},
                {"createTime", create_time},
                {"creatorId", creator_id},
                {"problems", problems_list}
            };
            std::cout << "[info]handleGetGroupInfo: " << resp["data"].dump() << std::endl;
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupInfo: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupInfo: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取题组失败：" + std::string(e.what());
        std::cerr << "[error]handleGetGroupInfo: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleGetGroupInfo: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleGetStats(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleGetStats: get discussion submission list." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", u8"获取提交记录失败"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        int group_id = std::stoi(req.matches[1]);

        std::unique_ptr<PooledResultSet> submission_db = DataAccess::getInstance()->selectDBuser_submissionForContest(group_id);
        std::vector<std::string> submisstion;
        //解析查找内容
        int total = 0;
        while (submission_db && submission_db->next())
        {
            nlohmann::json json_db;
            json_db =
            {
                {"userId", submission_db->getInt("user_id")},
                {"language", submission_db->getString("language")},
                {"status", codeResultToString(codeIntToCodeResult(submission_db->getInt("status")))},
                {"question_id", submission_db->getInt("question_id")},
                {"score", submission_db->getInt("score")},
                {"executionTime", submission_db->getInt("execution_time")},
                {"memoryUsed", submission_db->getInt("memory_used")},
                {"createdAt", submission_db->getString("submit_at")}
            };
            std::cout << "[info]handleGetStats: " << json_db.dump() << std::endl;

            submisstion.emplace_back(json_db.dump());
            total++;
        }

        //响应前端请求
        resp["code"] = 0;
        resp["data"] =
        {
            {"list", submisstion},
            {"total", total}
        };
    }
    catch (const nlohmann::json::parse_error& e)
    {
        resp["msg"] = u8"请求参数格式错误：" + std::string(e.what());
        std::cerr << "[error]handleGetStats: " << e.what() << std::endl;
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleGetStats: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取提交记录失败：" + std::string(e.what());
        std::cerr << "[error]handleGetStats: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取提交记录失败：未知系统错误";
        std::cerr << "[error]handleGetStats: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSetNickname(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSetNickname: set user nickname." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "PUT, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);
            if (DataAccess::getInstance()->updateDBuserInfo(parsed_user_id, "nickname", req_data["nickname"]))
            {
                resp["code"] = 0;
                resp["msg"] = "success";
            }
            else
            {
                resp["code"] = -500;
                resp["msg"] = "failed";
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSetNickname: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSetNickname: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSetNickname: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSetNickname: token failed." << std::endl;
        }
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleSetNickname: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleSetNickname: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleSetNickname: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSetPassword(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSetPassword: set user password." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "PUT, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        nlohmann::json req_data = nlohmann::json::parse(req.body);
        std::unique_ptr<PooledResultSet> user_res = DataAccess::getInstance()->selectDBResources<int64_t>(parsed_user_id, DBType::USER);
        if (user_res && user_res->next())
        {
            int64_t manage_change_id = -1;
            if (req_data.contains("userId") && req_data["userId"].is_number())
                manage_change_id = req_data["userId"].get<int64_t>();

            if (isValid == hs256::ReturnType::SUCCESS && sha256::hashPasswordBysha256(req_data["oldPassword"], user_res->getString("salt")) == user_res->getString("password"))
            {
                if (manage_change_id != -1)
                {
                    std::cout << "[info]handleSetPassword: change password, id2: " << manage_change_id << std::endl;
                    if (DataAccess::getInstance()->updateDBuserPassword(manage_change_id, req_data["newPassword"]))
                    {
                        resp["code"] = 0;
                        resp["msg"] = "success";
                    }
                    else
                    {
                        resp["code"] = -500;
                        resp["msg"] = "failed";
                    }
                }
                else
                {
                    std::cout << "[info]handleSetPassword: change password, id1: " << parsed_user_id << std::endl;
                    if (DataAccess::getInstance()->updateDBuserPassword(parsed_user_id, req_data["newPassword"]))
                    {
                        resp["code"] = 0;
                        resp["msg"] = "success";
                    }
                    else
                    {
                        resp["code"] = -500;
                        resp["msg"] = "failed";
                    }
                }
            }
            else if (isValid == hs256::ReturnType::OVERDUEERROR)
            {
                resp["code"] = -401;
                resp["msg"] = u8"Token登录失效，请先重新登录";
                std::cerr << "[error]handleSetPassword: token failed." << std::endl;
            }
            else if (isValid == hs256::ReturnType::TYPEERROR)
            {
                resp["code"] = -401;
                resp["msg"] = u8"Token格式错误，请登录后进行操作";
                std::cerr << "[error]handleSetPassword: token failed." << std::endl;
            }
            else if (isValid == hs256::ReturnType::VERIFYERROR)
            {
                resp["code"] = -401;
                resp["msg"] = u8"Token验证错误，请登录后进行操作";
                std::cerr << "[error]handleSetPassword: token failed." << std::endl;
            }
            else
            {
                resp["code"] = -401;
                resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
                std::cerr << "[error]handleSetPassword: token failed." << std::endl;
            }


        }
        else
        {
            resp["code"] = -500;
            resp["msg"] = u8"未检索到当前修改用户，请尝试重新登陆";
            std::cerr << "[error]handleSetPassword: select user id failed." << std::endl;
        }
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleSetPassword: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleSetPassword: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleSetPassword: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSetEmail(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSetEmail: set user email." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "PUT, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);
            if (DataAccess::getInstance()->updateDBuserInfo(parsed_user_id, "email", req_data["email"]))
            {
                resp["code"] = 0;
                resp["msg"] = "success";
            }
            else
            {
                resp["code"] = -500;
                resp["msg"] = "failed";
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSetEmail: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSetEmail: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSetEmail: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSetEmail: token failed." << std::endl;
        }
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleSetEmail: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleSetEmail: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleSetEmail: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleSetProvacy(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSetProvacy: set user provacy." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "PUT, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            nlohmann::json req_data = nlohmann::json::parse(req.body);

            int info_display_setting = 0;
            //按位从低到高为：1角色，2学校，3年级，4提交总数，5已解决题目数，6提交语言统计, 7学号, 8真名
            //所以这里写入要反过来
            info_display_setting += (req_data["showRealName"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showUserRoleNo"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showSubmissionStats"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showSolvedCount"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showSubmissionsCount"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showGrade"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showSchool"] == true ? 1 : 0);
            info_display_setting = info_display_setting << 1;
            info_display_setting += (req_data["showRole"] == true ? 1 : 0);


            resp["code"] = 0;
            resp["msg"] = "success";
            resp["data"] = nlohmann::json::object();
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleSetProvacy: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleSetProvacy: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleSetProvacy: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleSetProvacy: token failed." << std::endl;
        }
    }
    catch (const sql::SQLException& e)
    {
        resp["msg"] = u8"数据库错误：" + std::string(e.what());
        std::cerr << "[error]handleSetProvacy: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        resp["msg"] = u8"获取用户失败：" + std::string(e.what());
        std::cerr << "[error]handleSetProvacy: " << e.what() << std::endl;
    }
    catch (...)
    {
        resp["msg"] = u8"获取用户失败：未知系统错误";
        std::cerr << "[error]handleSetProvacy: 捕获未知异常" << std::endl;
    }

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}

void handleDeleteDiscussion(const httplib::Request& req, httplib::Response& res)
{
    std::cout << "[debug]handleSetProvacy: set user provacy." << std::endl;

    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.set_header("Content-Type", "application/json; charset=utf-8");

    if (req.method == "OPTIONS")
    {
        res.status = 200;
        return;
    }

    nlohmann::json resp =
    {
        {"code", -1},
        {"msg", "failed"},
        {"data", nlohmann::json::object()}
    };

    try
    {
        std::string token = req.get_header_value("Authorization");
        token = (token.substr(0, 7) == "Bearer " ? token.substr(7) : token);

        int64_t parsed_user_id;
        std::string parsed_role;

        hs256::ReturnType isValid = hs256::verifyToken(token, SECURE_SECRET_KEY, parsed_user_id, parsed_role);
        if (isValid == hs256::ReturnType::SUCCESS)
        {
            std::string discussion_group_id_str = req.matches[1];
            int64_t discussion_group_id = std::stoi(discussion_group_id_str);

            std::unique_ptr<PooledResultSet> discussion_group_res = DataAccess::getInstance()->selectDBResources<int64_t>(discussion_group_id, DBType::DISCUSSION_GROUP);
            if (discussion_group_res && discussion_group_res->next())
            {
                int64_t creator_id = discussion_group_res->getInt("creator_id");
                if (creator_id == parsed_user_id || RoleStringToDBType(parsed_role) == DBType::ADMIN)
                {
                    if (DataAccess::getInstance()->deleteDBproperty(discussion_group_id, DBType::DISCUSSION_GROUP))
                    {
                        resp["code"] = 0;
                        resp["msg"] = u8"success";
                        resp["data"] = nlohmann::json::object();
                    }
                    else
                    {
                        resp["code"] = -401;
                        resp["msg"] = u8"未知原因删除失败，请稍后重试";
                        std::cerr << "[error]handleDeleteDiscussion: delete failed." << std::endl;
                    }
                }
                else
                {
                    resp["code"] = -401;
                    resp["msg"] = u8"没有删除权限";
                    std::cerr << "[error]handleDeleteDiscussion: delete failed, no ability." << std::endl;
                }
            }
        }
        else if (isValid == hs256::ReturnType::OVERDUEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token登录失效，请先重新登录";
            std::cerr << "[error]handleDeleteDiscussion: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::TYPEERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token格式错误，请登录后进行操作";
            std::cerr << "[error]handleDeleteDiscussion: token failed." << std::endl;
        }
        else if (isValid == hs256::ReturnType::VERIFYERROR)
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token验证错误，请登录后进行操作";
            std::cerr << "[error]handleDeleteDiscussion: token failed." << std::endl;
        }
        else
        {
            resp["code"] = -401;
            resp["msg"] = u8"Token未知错误，请重新登录后进行操作";
            std::cerr << "[error]handleDeleteDiscussion: token failed." << std::endl;
        }
    }
    catch (...) {}

    res.status = 200;
    res.set_content(resp.dump(-1, ' ', true, nlohmann::json::error_handler_t::replace), "application/json; charset=utf-8");
}
