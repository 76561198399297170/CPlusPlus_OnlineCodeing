#ifndef _DATABASES_H_
#define _DATABASES_H_

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

#include <string>
#include <vector>
#include <memory>
#include <stdint.h>

#include "database/connection_pool.h"
#include "database/user_info.h"
#include "utils/security.h"

class DataAccess : public Singleton<DataAccess>
{
    friend class Singleton<DataAccess>;

public:
    //创建学生用户
    bool createDBstudent(const std::string& account, const std::string& phone, const std::string& email, const std::string& password,
        const std::string& nickname, const std::string& user_desc, int info_display_setting, const std::string& school,
        int grade, const std::string& student_no, const std::string& real_name, const std::string& training_monitor, int64_t& out_user_id,
        int64_t& out_student_id);
    //创建教师用户
    bool createDBteacher(const std::string& account, const std::string& phone, const std::string& email, const std::string& password,
        const std::string& nickname, const std::string& user_desc, int info_display_setting, const std::string& school,
        const std::string& real_name, const std::string& training_monitor, int64_t& out_user_id, int64_t& out_teacher_id);
    //创建管理员用户（密码使用明文，存储自动加密）
    bool createDBadmin(const std::string& account, const std::string& phone, const std::string& email, const std::string& password,
        const std::string& nickname, const std::string& user_desc, int info_display_setting, int64_t& out_user_id,
        int64_t& out_admin_id);
    //创建题目元数据
    bool createDBquestion(const std::string& question_name, int difficulty, const std::vector<std::string>& tags, bool is_audited,
        int64_t& question_id);
    //创建题目完成状态
    bool createDBuser_submission(const int64_t& user_id, const int64_t& question_id, const std::string& code,
        const std::string language, int status, int score, int execution_time, int memory_used, int64_t& out_submission_id, int is_audited = 0);
    //创建题目提交记录
    bool createDBuser_question_status(const int64_t& user_id, const int64_t& question_id, int status, int score, int64_t& out_id);
    //创建题目组
    bool createDBdiscussion_group(const int64_t& create_id, const std::string& name, const std::string& desc,
        const std::string& start_time, const std::string& end_time, int is_contest, const std::vector<int64_t>& problems_id_list,
        int64_t& out_group_id);
    //创建题目组题目关联条目
    bool createDBgroup_question(const int64_t& group_id, const int64_t& question_id, const int order_num, int64_t& out_id);
    //创建用户操作题目组记录
    bool createDBuser_group_status(int64_t user_id, int64_t group_id, int64_t& out_id, int status = 1);

    //*新增表需要更新*
    //删除数据库条目
    bool deleteDBproperty(int64_t primary_id, DBType type);

    //*新增表需要更新*
    //数据库查询: 仅限一条记录
    template<class T>
    std::unique_ptr<PooledResultSet> selectDBResources(T value, DBType type, std::string limit = "")
    {
        std::string table_name = "";

        switch (type)
        {
        case DBType::ADMIN:
            table_name = "admin";
            limit = (limit == "" ? "admin_id" : limit);
            break;
        case DBType::QUESTION:
            table_name = "question";
            limit = (limit == "" ? "question_id" : limit);
            break;
        case DBType::STUDENT:
            table_name = "student";
            limit = (limit == "" ? "student_id" : limit);
            break;
        case DBType::TEACHER:
            table_name = "teacher";
            limit = (limit == "" ? "teacher_id" : limit);
            break;
        case DBType::USER:
            table_name = "user";
            limit = (limit == "" ? "user_id" : limit);
            break;
        case DBType::USER_QUESTION_STATUS:
            table_name = "user_question_status";
            limit = (limit == "" ? "id" : limit);
            break;
        case DBType::USER_GROUP_STATUS:
            table_name = "user_group_status";
            limit = (limit == "" ? "id" : limit);
            break;
        case DBType::USER_SUBMISSION:
            table_name = "user_submission";
            limit = (limit == "" ? "submission_id" : limit);
            break;
        case DBType::DISCUSSION_GROUP:
            table_name = "discussion_group";
            limit = (limit == "" ? "group_id" : limit);
            break;
        case DBType::GROUP_QUESTION:
            table_name = "group_question";
            limit = (limit == "" ? "id" : limit);
            break;
        default:
            std::cerr << "[warn]DataAccess::deleteDBproperty: invalid DBType." << std::endl;
            return nullptr;
        }
        std::string sql = "SELECT * FROM " + table_name + " WHERE " + limit + " = ? LIMIT 1";

        std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
        if (!pooled_conn || !pooled_conn->get())
        {
            std::cerr << "[warn]DataAccess::deleteDBproperty: get connection from pool failed." << std::endl;
            return nullptr;
        }
        try
        {
            sql::Connection* conn = pooled_conn->get();
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

            if constexpr (std::is_same_v<T, std::string>)
            {
                pstmt->setString(1, value);
            }
            else if constexpr (std::is_integral_v<T>)
            {
                pstmt->setInt64(1, static_cast<int64_t>(value));
            }
            else
            {
                pstmt->setInt64(1, value);
            }

            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
        }
        catch (sql::SQLException& e)
        {
            return nullptr;
        }
    }

    //*新增表需要更新*
    //查询表所有数据【数据量大可能延迟大，一般不建议使用】
    std::unique_ptr<PooledResultSet> selectDBResourcesAll(DBType type);

    //题目特殊查询: 查询跳过前begin条共计size条题目数据（是否审核与是否竞赛二选一，选择后者前者传入-1
    std::unique_ptr<PooledResultSet> selectDBResourcesByRange(DBType type, int begin, int size, int is_audited = -1, int is_contest = -1);

    //题目特殊查询: 通过关键字, 标签, 难度, 页面综合查询，传入空即可省略综合部分，整形为空设为-1
    std::unique_ptr<PooledResultSet> selectDBquestionByComprehensiveness(int begin, int size, std::string keyword = "", std::vector<std::string> tags = {}, int difficulty = -1, int is_audited = -1);

    //题目特殊查询: 通过题目组查询题目
    std::unique_ptr<PooledResultSet> selectDBquestionByGroupId(const int64_t group_id);

    //题目特殊查询: 通过题目查询题目组
    std::unique_ptr<PooledResultSet> selectDBdiscussion_groupByQuestionId(const int64_t question_id, int is_contest = 0);

    //题目特殊查询: 查询用户答题状态(-1无记录状态 1未通过 2已通过)
    int selectDBuser_question_statusForStatus(const int64_t user_id, const int64_t question_id);

    //题组特殊查询: 通过用户题组状态查询, 增加列last_activity_time记录上次更新时间，列status题组状态
    std::unique_ptr<PooledResultSet> selectDBdiscussion_groupByStatus(int64_t user_id, int status, int is_contest = 0);

    //题组特殊查询：通过题组id查询当前是否结束提交时间,-1-错误/异常, 0-未开始, 1-已结束, 2-进行中
    int selectDBdiscussion_groupIsAllowSubmission(int64_t group_id);

    //查询答题记录: 查询限定记录, 为空可能查询全部不建议
    std::unique_ptr<PooledResultSet> selectDBuser_submission(std::vector<std::string> limit, std::vector<int> value);

    //查询答题记录: 查询跳过前begin条共计size条提交记录
    std::unique_ptr<PooledResultSet> selectDBuser_submissionByRange(int64_t user_id, int begin, int size);

    //查询答题记录: 特殊查询竞赛所有提交记录
    std::unique_ptr<PooledResultSet> selectDBuser_submissionForContest(int64_t group_id);

    //*新增表需要更新*
    //搜索表数据数量: 可输入WHERE后限制条件与参数, 仅限整形
    int selectDBcount(DBType type, std::vector<std::string> limit = {}, std::vector<int> value = {});

    //更新问题审核
    bool updateDBquestionAudited(int64_t question_id, int is_audited, PooledConnection* external_conn = nullptr);

    //更新答题状态
    bool updateDBuser_question_statusStatus(int64_t question_id, int64_t user_id, int status, PooledConnection* external_conn = nullptr);

    //更新用户信息: 自定设置用户信息
    bool updateDBuserInfo(int64_t user_id, std::string update_name, std::string update_value, PooledConnection* external_conn = nullptr);

    //更新用户信息: 特殊的密码和盐需要同时更新（传入明文密码，自动加密更换盐值，不做旧密码验证）
    bool updateDBuserPassword(int64_t user_id, std::string password, PooledConnection* external_conn = nullptr);

    //更新用户操作题目组（用户题组提交记录id若非-1则使用主键查询可忽略user_id与group_id）
    bool updateDBuser_group_statusStatus(int64_t user_id, int64_t group_id, int status = 1, int64_t user_group_status_id = -1, PooledConnection* external_conn = nullptr);

    //连接池是否可用
    bool isPoolAvailable() const { return ConnectionPool::getInstance()->getIdleCount() > 0; }

    //析构公有保证单例释放能够访问
    ~DataAccess() = default;

private:
    //私有构造
    DataAccess() = default;

    //禁用拷贝与移动
    DataAccess(const DataAccess&) = delete;
    DataAccess& operator=(const DataAccess&) = delete;
    DataAccess(DataAccess&&) = delete;
    DataAccess& operator=(DataAccess&&) = delete;

    //插入用户表
    bool insertDBuser(const std::string& account, const std::string& phone, const std::string& email, const std::string& password, const std::string& nickname, const std::string& user_desc, int role, int info_display_setting, int64_t& user_id, PooledConnection* external_conn = nullptr);
    //插入学生表
    bool insertDBstudent(int64_t user_id, const std::string& school, int grade, const std::string& student_no, const std::string& real_name, const std::string& training_monitor, int64_t& student_id, PooledConnection* external_conn = nullptr);
    //插入教师表
    bool insertDBteacher(int64_t user_id, const std::string& school, const std::string& real_name, const std::string& training_monitor, int64_t& teacher_id, PooledConnection* external_conn = nullptr);
    //插入管理员表
    bool insertDBadmin(int64_t user_id, int64_t& admin_id, PooledConnection* external_conn = nullptr);
    //插入题目表
    bool insertDBquestion(const std::string& question_name, int difficulty, const std::vector<std::string>& tags, bool is_audited, int64_t& out_question_id, PooledConnection* external_conn = nullptr);
    //插入提交记录数据
    bool insertDBuser_submission(const int64_t& user_id, const int64_t& question_id, const std::string& code, const std::string language, int status, int score, int execution_time, int memory_used, int64_t& out_submission_id, int is_audited = 0, PooledConnection* external_conn = nullptr);
    //插入提交状态数据
    bool insertDBuser_question_status(const int64_t& user_id, const int64_t& question_id, int status, int score, int64_t& out_id, PooledConnection* external_conn = nullptr);
    //插入训练或竞赛题组表
    bool insertDBdiscussion_group(const int64_t& create_id, const std::string& name, const std::string& desc, const std::string& start_time, const std::string& end_time, int is_contest, int64_t& out_group_id, PooledConnection* external_conn = nullptr);
    //插入题目题组关联表
    bool insertDBgroup_question(const int64_t& group_id, const int64_t& question_id, const int order_num, int64_t& out_id, PooledConnection* external_conn = nullptr);
    //插入用户题组记录表
    bool insertDBuser_group_status(const int& status, const int64_t& group_id, const int64_t& user_id, int64_t& out_id, PooledConnection* external_conn = nullptr);
    //插入数据表
    bool insertDBData(const std::string& sql, const std::vector<std::string>& params, int64_t& last_insert_id, PooledConnection* external_conn = nullptr);
    //更新数据表
    bool updateDBData(const std::string& sql, const std::vector<std::string>& params, PooledConnection* external_conn = nullptr);

private:
    //修改了链接方式由持久单独持有数据库改为从数据库连接池获取
    //数据库链接数据
    //std::string m_host = "127.0.0.1";
    //std::string m_port = "3306";
    //std::string m_username = "root";
    //std::string m_password = "123456";
    //std::string m_dbname = "ocdb";

    //持久连接数据库
    //sql::mysql::MySQL_Driver* m_driver = nullptr;
    //std::unique_ptr<sql::Connection> m_connection = nullptr;

};

#endif // !_DATABASES_H_
