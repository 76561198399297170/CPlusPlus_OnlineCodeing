
// 测试MySQL-Connector-C++链接
#define _TEST_CONNECT_MYSQL_ false

// 测试httpcpp链接与ai通讯
#define _TEST_CONNECT_HTTPCPP_ false

// 测试jsoncpp链接
#define _TEST_CONNECT_NLOHMANN_ false

// 测试数据库操作
#define _TEST_DATABASE_CONNECTIONPOOL_DATAACCESS_ false

// 测试数据库操作
#define _TEST_RUNTIME_CODEEXECUTOR_ false

// 测试Token生成与解析
#define _TEST_TOKEN_SECURITY_ false

#if _TEST_CONNECT_MYSQL_

#include <iostream>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;
using namespace sql;

int main()
{
    mysql::MySQL_Driver* driver = nullptr;
    Connection* con = nullptr;
    Statement* stmt = nullptr;
    ResultSet* res = nullptr;

    try
    {
        driver = mysql::get_mysql_driver_instance();
        con = driver->connect("127.0.0.1:3306", "root", "Yy220311");
        con->setSchema("testdb");

        con->setClientOption("characterEncoding", "utf8");
        con->setClientOption("characterSetResults", "utf8mb4");

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT id, name FROM books");

        while (res->next())
        {
            cout << "id:   " << res->getInt("id") << "\n";
            cout << "name: " << res->getString("name") << "\n";
        }
    }
    catch (SQLException& e)
    {
        cerr << "MySQL error: " << e.what() << endl;
    }

    res->close();
    //可以直接close也支持手动释放delete res;
    delete stmt;
    delete con;

    return 0;
}

#elif _TEST_CONNECT_HTTPCPP_

#include <iostream>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "httplib.h"

using namespace std;
using namespace httplib;

int main()
{
    const char* AI_HOST = "192.168.1.8";
    const int AI_PORT = 8000;
    const char* AI_PATH = "/v1/chat/completions";

    Client cli(AI_HOST, AI_PORT);
    cli.set_read_timeout(5);
    cli.set_write_timeout(5);

    std::string post_data = "{\"model\": \"liquid/lfm2.5-1.2b\",\"messages\": [{\"role\": \"system\",\"content\": \"Please use Chinese.\"},{\"role\": \"user\",\"content\": \"请使用中文给我一个简单自我介绍，随后复述一遍你收到的消息，注意复述前另起一行。 \"}],\"temperature\": 0.7,\"max_tokens\": -1,\"stream\": false}";
	  //"content": "Please give a brief self-introduction with Chinese and then retell the message I sent you. Start a new line before retelling."
    //"content": "请使用中文给我一个简单自我介绍，随后复述一遍你收到的消息，注意复述前另起一行。"
    auto res = cli.Post(AI_PATH, post_data.c_str(), "application/json");

    if (res && res->status == 200)
    {
        cout << "report：\n" << res->body << endl;
    }
    else
    {
        cout << "请求失败！原因：" << (res ? "状态码错误(" + to_string(res->status) + ")" : "连接失败") << endl;
    }

    return 0;
}

#elif _TEST_CONNECT_NLOHMANN_

#include <iostream>
#include <fstream>
#include <string>

#include "json.hpp"

#include "utils/security.h"

using namespace nlohmann;
using namespace std;

int main()
{
    // parse() 遇到非法JSON会抛出 nlohmann::json::parse_error 异常
    json j1 = json::parse(string(R"({"name": "Alice", "age": 30})"));
    cout << "example1: " << j1 << endl;

    try
    {
        std::ifstream ifs("./src/example/test.json");
        json j2 = json::parse(ifs);

        ifs = std::ifstream("./src/example/test.json");
        cout << "ifstream: " << endl;
        string str;
        while (getline(ifs, str))
            cout << str << endl;
        cout << "\n" << endl;

        cout << "example2: " << j2 << endl;
    }
    catch (json::parse_error)
    {
        cout << "example2: " << "j2 error" << endl;
    }

    return 0;
}

#elif _TEST_DATABASE_CONNECTIONPOOL_DATAACCESS_

#include <iostream>

#include "database/data_access.h"

void testCreateStudent(int64_t& user_id, int64_t& student_id)
{
    std::cout << "--- test create student ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->createDBstudent("18207730000", "15177360000", "3629115067@qq.com", "Yy220311", "mioniko", "a simple student", 0, "GuiLin University", 2022, "202213000001", "real_name", "**Nothing**", user_id, student_id);
        if (success)
        {
            std::cout << "success，user ID: " << user_id << "， student ID: " << student_id << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testDeleteStudent(int64_t& user_id)
{
    std::cout << "--- test delete student ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->deleteDBproperty(user_id, DBType::USER);
        if (success)
        {
            std::cout << "success." << std::endl;
        }
        else 
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testCreateTeacher(int64_t& user_id, int64_t& teacher_id)
{
    std::cout << "--- test create teacher ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->createDBteacher("18207730000", "15177360000", "3629115067@qq.com", "Yy220311", "nikoname", "this is a desc", 0, "GuiLin University", "real name", "**1**do something", user_id, teacher_id);
        if (success)
        {
            std::cout << "success，user ID: " << user_id << "， teacher ID: " << teacher_id << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testDeleteTeacher(int64_t& user_id)
{
    std::cout << "--- test delete teacher ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->deleteDBproperty(user_id, DBType::USER);
        if (success)
        {
            std::cout << "success." << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testCreateAdmin(int64_t& user_id, int64_t& admin_id)
{
    std::cout << "--- test create admin ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->createDBadmin("18207730000", "15177360000", "3629115067@qq.com", "Yy220311", "nikoniko", "this is my desc", 0, user_id, admin_id);
        if (success)
        {
            std::cout << "success，user ID: " << user_id << "， admin ID: " << admin_id << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testDeleteAdmin(int64_t& user_id)
{
    std::cout << "--- test delete admin ---" << std::endl;
    system("pause");
    try
    {
        bool success = DataAccess::getInstance()->deleteDBproperty(user_id, DBType::USER);
        if (success)
        {
            std::cout << "success." << std::endl;
        }
        else
        {
            std::cout << "failed." << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

void testSelect(DBType type, int64_t value, std::string limit)
{
    std::cout << "--- test select ---" << std::endl;
    system("pause");
    try
    {
        std::unique_ptr<PooledResultSet> res = DataAccess::getInstance()->selectDBResources<int64_t>(value, type, limit);
        if (!res)
        {
            std::cout << "nullptr." << std::endl;
            return;
        }
        
        while(res->next())
        {
            std::string usn = res->getString("nickname");
            std::string psw = res->getString("password");
            
            std::cout << "res: " << usn << " " << psw << std::endl;
        }
        return;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[error]test: " << e.what() << "." << std::endl;
        return;
    }
}

int main()
{
    std::cout << "start test ConnectionPool..." << std::endl;
    try
    {
        ConnectionPool::getInstance()->init(PoolConfig("127.0.0.1", "3306", "root", "Yy220311", "ocdb"));
        std::cout << "ConnectionPool init success." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "test init ConnectionPool failed: " << e.what() << std::endl;
        return -1;
    }

    system("pause");
    std::cout << "=== test create and delete [student, teacher, admin] ===" << std::endl;
    try
    {
        int64_t user_id = -1;
        int64_t student_id = -1;
        testCreateStudent(user_id, student_id);
        testSelect(DBType::USER, user_id, "");
        testDeleteStudent(user_id);

        user_id = -1;
        int64_t teacher_id = -1;
        testCreateTeacher(user_id, teacher_id);
        testDeleteTeacher(user_id);

        user_id = -1;
        int64_t admin_id = -1;
        testCreateAdmin(user_id, admin_id);
        testDeleteAdmin(user_id);
    }
    catch (...)
    {
        std::cerr << "ConnectionPool test failed." << std::endl;
        return -1;
    }

    return 0;
}

#elif _TEST_RUNTIME_CODEEXECUTOR_

#include "runtime/executor.h"

int main()
{
    CodeExecutor test_exe;
    test_exe.setTimeLimit(1000);
    test_exe.setCompileTimeLimit(7500);
    test_exe.setMemoryLimit(256);
    test_exe.setTempDir("C:\\Users\\Administrator\\Desktop\\OnlineCodeing\\onlinecodeing-server\\OnlineCodeing\\res\\temp");

    // ========== C++ (语法错误测试) ==========
    std::string test_code_cpp = R"(
#include <iostream>
int main()
{
    int a, b;
    std::cin >> //故意留错，测试语法问题
    std::cout << a + b << std::endl;
    return 0;
}
)";
    std::vector<std::vector<std::string>> inputs = { {"1 2"}, {"10 20"} };
    std::vector<std::vector<std::string>> outputs = { {"3"}, {"30"} };

    std::cout << "===== start evaluate cpp =====" << std::endl;
    std::string res = test_exe.evaluateCode(LangType::CPP, test_code_cpp, inputs, outputs);
    std::cout << "\n===== evaluate end =====" << std::endl;
    std::cout << "result: " << res << std::endl;
    std::cout << "===== finish =====" << std::endl;
    system("pause");




    // ========== Python (正确代码测试) ==========
    std::string test_code_py = R"(
a, b = map(int, input().split())
print(a + b)
)";
    std::vector<std::vector<std::string>> inputs2 = { {"1 2"}, {"10 20"} };
    std::vector<std::vector<std::string>> outputs2 = { {"3"}, {"30"} };

    std::cout << "\n===== start evaluate python =====" << std::endl;
    std::string res2 = test_exe.evaluateCode(LangType::PYTHON3, test_code_py, inputs2, outputs2);
    std::cout << "\n===== evaluate end =====" << std::endl;
    std::cout << "result: " << res2 << std::endl;
    std::cout << "===== finish =====" << std::endl;
    system("pause");

    return 0;
}

#elif _TEST_TOKEN_SECURITY_

#include "utils/security.h"

int main()
{
    const std::string secret = "my_secure_key_123";
    int64_t test_user_id = 64;
    std::string test_role = "student";

    // 生成Token
    std::string token = hs256::generateToken(secret, test_user_id, test_role);
    std::cout << "[debug] 生成的HS256 Token：" << std::endl << token << std::endl << std::endl;

    // 验证Token
    int64_t parsed_user_id;
    std::string parsed_role;
    hs256::ReturnType isValid = hs256::verifyToken(token, secret, parsed_user_id, parsed_role);

    if (isValid == hs256::ReturnType::SUCCESS)
    {
        std::cout << "[success] Token验证通过！" << std::endl;
        std::cout << "user_id: " << parsed_user_id << ", role: " << parsed_role << std::endl;
    }
    else
    {
        std::cerr << "[error] Token验证失败！" << std::endl;
    }

    return 0;
}


#endif
