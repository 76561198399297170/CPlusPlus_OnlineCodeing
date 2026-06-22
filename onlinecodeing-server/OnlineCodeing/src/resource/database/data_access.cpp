#include "database/data_access.h"

#define MYSQL_MAX_STRING_LENGTH 500

bool DataAccess::insertDBuser(const std::string& account, const std::string& phone, const std::string& email, const std::string& password, const std::string& nickname, const std::string& user_desc, int role, int info_display_setting, int64_t& user_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO user (account, phone, email, password, salt, nickname, user_desc, role, info_display_setting) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(account);
	params.emplace_back(phone);
	params.emplace_back(email);
	std::string salt = sha256::generateSalt();
	params.emplace_back(sha256::hashPasswordBysha256(password, salt));
	params.emplace_back(salt);
	params.emplace_back(nickname);
	params.emplace_back(user_desc);
	params.emplace_back(std::to_string(role));
	params.emplace_back(std::to_string(info_display_setting));

	return this->insertDBData(sql, params, user_id, external_conn);
}

bool DataAccess::insertDBstudent(int64_t user_id, const std::string& school, int grade, const std::string& student_no, const std::string& real_name, const std::string& training_monitor, int64_t& student_id, PooledConnection* external_conn)
{
	if (user_id <= 0)
	{
		std::cerr << "[warn]DataAccess::insertDBstudent: invalid user id: " << user_id << std::endl;
		return false;
	}

	std::string sql = "INSERT INTO student (user_id, school, grade, student_no, real_name, training_monitor) VALUES (?, ?, ?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(school);
	params.emplace_back(std::to_string(grade));
	params.emplace_back(student_no);
	params.emplace_back(real_name);
	params.emplace_back(training_monitor);

	return this->insertDBData(sql, params, student_id, external_conn);
}

bool DataAccess::insertDBteacher(int64_t user_id, const std::string& school, const std::string& real_name, const std::string& training_monitor, int64_t& teacher_id, PooledConnection* external_conn)
{
	if (user_id <= 0)
	{
		std::cerr << "[warn]DataAccess::insertDBteacher: invalid user id: " << user_id << std::endl;
		return false;
	}

	std::string sql = "INSERT INTO teacher (user_id, school, real_name, training_monitor) VALUES (?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(school);
	params.emplace_back(real_name);
	params.emplace_back(training_monitor);

	return this->insertDBData(sql, params, teacher_id, external_conn);
}

bool DataAccess::insertDBadmin(int64_t user_id, int64_t& admin_id, PooledConnection* external_conn)
{
	if (user_id <= 0)
	{
		std::cerr << "[warn]DataAccess::insertDBteacher: invalid user id: " << user_id << std::endl;
		return false;
	}

	std::string sql = "INSERT INTO admin (user_id) VALUES (?)";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));

	return this->insertDBData(sql, params, admin_id, external_conn);
}

bool DataAccess::insertDBquestion(const std::string& question_name, int difficulty, const std::vector<std::string>& tags, bool is_audited, int64_t& question_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO question (question_name, difficulty, tags, is_audited) VALUES (?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(question_name);
	params.emplace_back(std::to_string(difficulty));

	std::string tags_json = "[";
	for (size_t i = 0; i < tags.size(); i++) 
	{
		tags_json += "\"" + tags[i] + "\"";
		if (i < tags.size() - 1)
		{
			tags_json += ",";
		}
	}
	tags_json += "]";
	params.emplace_back(tags_json);
	params.emplace_back(std::to_string(is_audited ? 1 : 0));

	return this->insertDBData(sql, params, question_id, external_conn);
}

bool DataAccess::insertDBuser_submission(const int64_t& user_id, const int64_t& question_id, const std::string& code, const std::string language, int status, int score, int execution_time, int memory_used, int64_t& out_submission_id, int is_audited, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO user_submission (user_id, question_id, code, language, status, score, execution_time, memory_used, is_audited) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(std::to_string(question_id));
	params.emplace_back(code);
	params.emplace_back(language);
	params.emplace_back(std::to_string(status));
	params.emplace_back(std::to_string(score));
	params.emplace_back(std::to_string(execution_time));
	params.emplace_back(std::to_string(memory_used));
	params.emplace_back(std::to_string(is_audited));

	return this->insertDBData(sql, params, out_submission_id, external_conn);
}

bool DataAccess::insertDBuser_question_status(const int64_t& user_id, const int64_t& question_id, int status, int score, int64_t& out_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO user_question_status (user_id, question_id, status, best_score) VALUES (?, ?, ?, ?)";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(std::to_string(question_id));
	params.emplace_back(std::to_string(status));
	params.emplace_back(std::to_string(score));

	return this->insertDBData(sql, params, out_id, external_conn);
}

bool DataAccess::insertDBdiscussion_group(const int64_t& create_id, const std::string& name, const std::string& desc, const std::string& start_time, const std::string& end_time, int is_contest, int64_t& out_group_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO discussion_group (group_name, group_desc,";
	if (!start_time.empty()) sql += " start_time, ";
	if (!end_time.empty()) sql += "end_time, ";
	if (is_contest != -1) sql += "is_competition, ";
	sql += "creator_id) VALUES (?, ?, ";
	if (!start_time.empty()) sql += "?, ";
	if (!end_time.empty()) sql += "?, ";
	if (is_contest != -1) sql += "?, ";
	sql += "?);";

	std::vector<std::string> params;
	params.emplace_back(name);
	params.emplace_back(desc);
	if (!start_time.empty()) params.emplace_back(start_time);
	if (!end_time.empty()) params.emplace_back(end_time);
	if (is_contest != -1) params.emplace_back(std::to_string(is_contest));
	params.emplace_back(std::to_string(create_id));//创建者id

	return this->insertDBData(sql, params, out_group_id, external_conn);
}

bool DataAccess::insertDBgroup_question(const int64_t& group_id, const int64_t& question_id, const int order_num, int64_t& out_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO group_question (group_id, question_id, order_num) VALUES (?, ?, ?);";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(group_id));
	params.emplace_back(std::to_string(question_id));
	params.emplace_back(std::to_string(order_num));

	return this->insertDBData(sql, params, out_id, external_conn);
}

bool DataAccess::insertDBuser_group_status(const int& status, const int64_t& group_id, const int64_t& user_id, int64_t& out_id, PooledConnection* external_conn)
{
	std::string sql = "INSERT INTO user_group_status (user_id, group_id, status) VALUES (?, ?, ?);";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(std::to_string(group_id));
	params.emplace_back(std::to_string(status));

	return this->insertDBData(sql, params, out_id, external_conn);
}

bool DataAccess::createDBstudent(const std::string& account, const std::string& phone, const std::string& email, const std::string& password, const std::string& nickname, const std::string& user_desc, int info_display_setting, const std::string& school, int grade, const std::string& student_no, const std::string& real_name, const std::string& training_monitor, int64_t& out_user_id, int64_t& out_student_id)
{
	out_user_id = -1;
	out_student_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBstudent: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser(account, phone, email, password, nickname, user_desc, (int)DBRole::STUDENT, info_display_setting, out_user_id, pooled_conn.get()) || out_user_id <= 0)
		{
			throw std::runtime_error("insert user table failed");
		}

		if (!this->insertDBstudent(out_user_id, school, grade, student_no, real_name, training_monitor, out_student_id, pooled_conn.get()) || out_student_id <= 0)
		{
			throw std::runtime_error("insert student table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBstudent: create student success, user_id: " << out_user_id << ", student_id: " << out_student_id << std::endl;
		return true;
	}
	catch (const std::exception& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBstudent: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBteacher(const std::string& account, const std::string& phone, const std::string& email, const std::string& password, const std::string& nickname, const std::string& user_desc, int info_display_setting, const std::string& school, const std::string& real_name, const std::string& training_monitor, int64_t& out_user_id, int64_t& out_teacher_id)
{
	out_user_id = -1;
	out_teacher_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBteacher: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser(account, phone, email, password, nickname, user_desc, (int)DBRole::TEACHER, info_display_setting, out_user_id, pooled_conn.get()) || out_user_id <= 0)
		{
			throw std::runtime_error("insert user table failed");
		}
		if (!this->insertDBteacher(out_user_id, school, real_name, training_monitor, out_teacher_id, pooled_conn.get()) || out_teacher_id <= 0)
		{
			throw std::runtime_error("insert teacher table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBteacher: create student success, user_id: " << out_user_id << ", teacher_id: " << out_teacher_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBteacher: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBadmin(const std::string& account, const std::string& phone, const std::string& email, const std::string& password, const std::string& nickname, const std::string& user_desc, int info_display_setting, int64_t& out_user_id, int64_t& out_admin_id)
{
	out_user_id = -1;
	out_admin_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBadmin: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser(account, phone, email, password, nickname, user_desc, (int)DBRole::ADMIN, info_display_setting, out_user_id, pooled_conn.get()) || out_user_id <= 0)
		{
			throw std::runtime_error("insert user table failed");
		}
		if (!this->insertDBadmin(out_user_id, out_admin_id, pooled_conn.get()) || out_admin_id <= 0)
		{
			throw std::runtime_error("insert admin table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBadmin: create student success, user_id: " << out_user_id << ", admin_id: " << out_admin_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBadmin: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBquestion(const std::string& question_name, int difficulty, const std::vector<std::string>& tags, bool is_audited, int64_t& out_question_id)
{
	out_question_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBquestion: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try 
	{
		if (!this->insertDBquestion(question_name, difficulty, tags, is_audited, out_question_id, pooled_conn.get()) || out_question_id <= 0)
		{
			throw std::runtime_error("insert question table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBquestion: create question success, question_id: " << out_question_id  << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBquestion: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBuser_submission(const int64_t& user_id, const int64_t& question_id, const std::string& code, const std::string language, int status, int score, int execution_time, int memory_used, int64_t& out_submission_id, int is_audited)
{
	out_submission_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection(); if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBuser_submission: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser_submission(user_id, question_id, code, language, status, score, execution_time, memory_used, out_submission_id, is_audited, pooled_conn.get()) || out_submission_id <= 0)
		{
			throw std::runtime_error("insert user_submission table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBuser_submission: create question success, question_id: " << out_submission_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBuser_submission: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBuser_question_status(const int64_t& user_id, const int64_t& question_id, int status, int score, int64_t& out_id)
{
	out_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection(); if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBuser_question_status: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser_question_status(user_id, question_id, status, score, out_id, pooled_conn.get()) || out_id <= 0)
		{
			throw std::runtime_error("insert user_question_status table failed");
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBuser_question_status: create user_question_status success, user_question_status: " << out_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBuser_question_status: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBdiscussion_group(const int64_t& create_id, const std::string& name, const std::string& desc, const std::string& start_time, const std::string& end_time, int is_contest, const std::vector<int64_t>& problems_id_list, int64_t& out_group_id)
{
	out_group_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection(); if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBdiscussion_group: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBdiscussion_group(create_id, name, desc, start_time, end_time, is_contest, out_group_id, pooled_conn.get()) || out_group_id <= 0)
		{
			throw std::runtime_error("insert discussion_group table failed");
		}

		for (size_t i = 0; i < problems_id_list.size(); i++)
		{
			int64_t out_id = -1;
			if (!this->insertDBgroup_question(out_group_id, problems_id_list[i], i, out_id, pooled_conn.get()) || out_id <= 0)
			{
				throw std::runtime_error("insert group_question table failed");
			}
		}

		conn->commit();
		std::cout << "[info]DataAccess::createDBdiscussion_group: create discussion_group success, discussion_group: " << out_group_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBdiscussion_group: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBgroup_question(const int64_t& group_id, const int64_t& question_id, const int order_num, int64_t& out_id)
{
	out_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection(); 
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBgroup_question: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		int64_t out_id = -1;
		if (!this->insertDBgroup_question(group_id, question_id, order_num, out_id, pooled_conn.get()) || out_id <= 0)
		{
			throw std::runtime_error("insert group_question table failed");
		}
		conn->commit();
		std::cout << "[info]DataAccess::createDBgroup_question: create group_question success, group_question: " << out_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBdiscussion_group: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::createDBuser_group_status(int64_t user_id, int64_t group_id, int64_t& out_id, int status)
{
	out_id = -1;

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[error]DataAccess::createDBuser_group_status: get connection from pool failed." << std::endl;
		return false;
	}
	sql::Connection* conn = pooled_conn->get();

	TransactionGuard guard(conn);
	try
	{
		if (!this->insertDBuser_group_status(status, group_id, user_id, out_id, pooled_conn.get()) || out_id <= 0)
		{
			throw std::runtime_error("insert user_group_status table failed");
		}
		conn->commit();
		std::cout << "[info]DataAccess::createDBuser_group_status: create user_group_status success, user_group_status: " << out_id << std::endl;
		return true;
	}
	catch (sql::SQLException& e)
	{
		conn->rollback();
		std::cerr << "[warn]DataAccess::createDBuser_group_status: failed, rollback. Error: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::deleteDBproperty(int64_t primary_id, DBType type)
{
	std::string sql = "";
	std::string primary_column = "";
	std::string table_name = "";

	switch (type)
	{
	case DBType::USER:
		table_name = "user";
		primary_column = "user_id";
		break;
	case DBType::STUDENT:
		table_name = "student";
		primary_column = "student_id";
		break;
	case DBType::TEACHER:
		table_name = "teacher";
		primary_column = "teacher_id";
		break;
	case DBType::ADMIN:
		table_name = "admin";
		primary_column = "admin_id";
		break;
	case DBType::QUESTION:
		table_name = "question";
		primary_column = "question_id";
		break;
	case DBType::USER_QUESTION_STATUS:
		table_name = "user_question_status";
		primary_column = "id";
		break;
	case DBType::USER_GROUP_STATUS:
		table_name = "user_group_status";
		primary_column = "id";
		break;
	case DBType::DISCUSSION_GROUP:
		table_name = "discussion_group";
		primary_column = "group_id";
		break;
	case DBType::GROUP_QUESTION:
		table_name = "group_question";
		primary_column = "id";
		break;
	default:
		std::cerr << "[warn]DataAccess::deleteDBproperty: invalid DBType." << std::endl;
		return false;
	}
	sql = "DELETE FROM " + table_name + " WHERE " + primary_column + " = ?";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::deleteDBproperty: get connection from pool failed." << std::endl;
		return false;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		pstmt->setInt64(1, primary_id);

		int affected_rows = pstmt->executeUpdate();
		if (affected_rows <= 0)
		{
			std::cerr << "[warn]DataAccess::deleteDBproperty: delete failed, affected rows 0. SQL: " << sql << ", primary: " << primary_id << std::endl;
			return false;
		}
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error " << e.getErrorCode() << "]DataAccess::deleteDBproperty: " << e.what() << std::endl;
		return false;
	}
	catch (const std::invalid_argument& e)
	{
		//primary无法转换为数字
		std::cerr << "[warn]DataAccess::deleteDBproperty: invalid primary value: " << primary_id << ". Error: " << e.what() << std::endl;
		return false;
	}
	catch (const std::out_of_range& e)
	{
		//primary数值超出范围
		std::cerr << "[warn]DataAccess::deleteDBproperty: primary value out of range: " << primary_id << ". Error: " << e.what() << std::endl;
		return false;
	}

	return true;
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBResourcesAll(DBType type)
{
	std::string table_name = "";

	switch (type)
	{
	case DBType::ADMIN:
		table_name = "admin";
		break;
	case DBType::QUESTION:
		table_name = "question";
		break;
	case DBType::STUDENT:
		table_name = "student";
		break;
	case DBType::TEACHER:
		table_name = "teacher";
		break;
	case DBType::USER:
		table_name = "user";
		break;
	case DBType::USER_QUESTION_STATUS:
		table_name = "user_question_status";
		break;
	case DBType::USER_GROUP_STATUS:
		table_name = "user_group_status";
		break;
	case DBType::DISCUSSION_GROUP:
		table_name = "discussion_group";
		break;
	case DBType::GROUP_QUESTION:
		table_name = "group_question";
		break;
	default:
		std::cerr << "[warn]DataAccess::deleteDBproperty: invalid DBType." << std::endl;
		return nullptr;
	}
	std::string sql = "SELECT * FROM " + table_name;

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

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		return nullptr;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBResourcesByRange(DBType type, int begin, int size, int is_audited, int is_contest)
{
	std::string table_name = "";

	switch (type)
	{
	case DBType::ADMIN:
		table_name = "admin";
		break;
	case DBType::QUESTION:
		table_name = "question";
		break;
	case DBType::STUDENT:
		table_name = "student";
		break;
	case DBType::TEACHER:
		table_name = "teacher";
		break;
	case DBType::USER:
		table_name = "user";
		break;
	case DBType::USER_QUESTION_STATUS:
		table_name = "user_question_status";
		break;
	case DBType::USER_GROUP_STATUS:
		table_name = "user_group_status";
		break;
	case DBType::DISCUSSION_GROUP:
		table_name = "discussion_group";
		break;
	default:
		std::cerr << "[warn]DataAccess::selectDBResourcesByRange: invalid DBType." << std::endl;
		return nullptr;
	}

	std::string sql = "SELECT * FROM " + table_name;
	if (is_audited != -1)
	{
		sql += " WHERE is_audited = ?";
	}
	else if (is_contest != -1)
	{
		sql += " WHERE is_competition = ?";
	}
	sql += " LIMIT ?, ?";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBResourcesByRange: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		if (is_audited != -1) pstmt->setInt(1, is_audited);
		else if (is_contest != -1) pstmt->setInt(1, is_contest);
		pstmt->setInt(1 + (is_audited != -1 || is_contest != -1 ? 1 : 0), begin);
		pstmt->setInt(2 + (is_audited != -1 || is_contest != -1 ? 1 : 0), size);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::selectDBResourcesByRange: " << e.what() << std::endl;
		return nullptr;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBquestionByComprehensiveness(int begin, int size, std::string keyword, std::vector<std::string> tags, int difficulty, int is_audited)
{
	std::string sql = "SELECT * FROM question";
	std::vector<std::string> params;

	bool is_limit = false;
	
	if (!keyword.empty())
	{
		sql = sql + " WHERE";
		is_limit = true;

		//Mysql转义特殊字符
		while (true)
		{
			std::string::iterator it = std::find(keyword.begin(), keyword.end(), '%');
			if (it != keyword.end() && *(it - 1) == '\\') keyword.replace(it, it + 1, "\\%");
			else break;
		}
		while (true)
		{
			std::string::iterator it = std::find(keyword.begin(), keyword.end(), '_');
			if (it != keyword.end() && *(it - 1) == '\\') keyword.replace(it, it + 1, "\\_");
			else break;
		}
		while (true)
		{
			std::string::iterator it = std::find(keyword.begin(), keyword.end(), '\\');
			if (it != keyword.end() && *(it - 1) == '\\') keyword.replace(it, it + 1, "\\\\");
			else break;
		}

		sql = sql + " question_name LIKE ?";
		params.emplace_back('%' + keyword + '%');
	}

	if (!tags.empty())
	{
		if (is_limit) sql = sql + " AND";
		else sql = sql + " WHERE";
		is_limit = true;

		//Json标签插入
		for (size_t i = 0; i < tags.size(); i++)
		{
			sql = sql + std::string(" JSON_CONTAINS(tags, ?)") + (i == tags.size() - 1 ? "" : " AND");
			params.emplace_back("\"" + tags[i] + "\"");
		}
	}

	if (difficulty != -1)
	{
		if (is_limit) sql = sql + " AND";
		else sql = sql + " WHERE";
		is_limit = true;

		//整形属性条件
		sql = sql + " difficulty = ?";
		params.emplace_back(std::to_string(difficulty));
	}

	if (is_audited != -1)
	{
		if (is_limit) sql = sql + " AND";
		else sql = sql + " WHERE";
		is_limit = true;

		//整形属性条件
		sql = sql + " is_audited = ?";
		params.emplace_back(std::to_string(is_audited));
	}

	sql = sql + " ORDER BY question_id LIMIT ? , ?";
	params.emplace_back(std::to_string(begin));
	params.emplace_back(std::to_string(size));

	//获取sql连接进行数据库搜索
	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBquestionByComprehensiveness: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		
		for (int i = 0; i < params.size(); i++)
		{
			pstmt->setString(i + 1, params[i]);
		}

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		return nullptr;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBquestionByGroupId(const int64_t group_id)
{
	std::string sql = "SELECT * FROM question \
					INNER JOIN group_question ON question.question_id = group_question.question_id \
					WHERE group_question.group_id = ? \
					ORDER BY group_question.order_num ASC;";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBquestionByGroupId: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

		pstmt->setInt64(1, group_id);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		return nullptr;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBdiscussion_groupByQuestionId(const int64_t question_id, int is_contest)
{
	std::string sql = "SELECT * FROM discussion_group \
		INNER JOIN group_question ON group_question.group_id = discussion_group.group_id \
		INNER JOIN question ON question.question_id = group_question.question_id \
		WHERE question.question_id = ? AND discussion_group.is_competition = ?;";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBdiscussion_groupByQuestionId: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

		pstmt->setInt64(1, question_id);
		pstmt->setInt(2, is_contest);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		return nullptr;
	}

	return nullptr;
}

int DataAccess::selectDBuser_question_statusForStatus(const int64_t user_id, const int64_t question_id)
{
	std::string sql = "SELECT * FROM user_question_status WHERE user_id = ? AND question_id = ?";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBquestionForStatus: get connection from pool failed." << std::endl;
		return 0;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

		pstmt->setInt64(1, user_id);
		pstmt->setInt64(2, question_id);

		std::unique_ptr<sql::ResultSet> status(pstmt->executeQuery());
		int completed = -1;
		while (status && status->next())
		{
			if (status->getInt("status") == 2) return 2;
			completed = 1;
		}

		return completed;
	}
	catch (sql::SQLException& e)
	{
		return 0;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBdiscussion_groupByStatus(int64_t user_id, int status, int is_contest)
{
	std::string sql = "(\
    SELECT \
        dg.*, \
        IFNULL(ugs.status, 0) AS status, \
        IFNULL(ugs.update_time, 0) AS last_activity_time \
    FROM discussion_group dg \
    LEFT JOIN ( \
        SELECT group_id, user_id, status, MAX(update_time) AS update_time \
        FROM user_group_status \
        WHERE user_id = ? \
        GROUP BY group_id, user_id, status \
    ) ugs ON dg.group_id = ugs.group_id \
    WHERE \
        IFNULL(ugs.status, 0) = ? \
        AND dg.is_competition = ? \
)";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBgroup_questionByStatus: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));

		pstmt->setInt64(1, user_id);
		pstmt->setInt(2, status);
		pstmt->setInt(3, is_contest);

		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[warn]DataAccess::selectDBgroup_questionByStatus: " << e.what() << std::endl;
		return nullptr;
	}

}

int DataAccess::selectDBdiscussion_groupIsAllowSubmission(int64_t group_id)
{
	std::string sql = "SELECT \
		CASE \
        WHEN is_competition = 0 THEN 2 \
		WHEN NOW() < start_time THEN 0 \
		WHEN NOW() > end_time THEN 1 \
		ELSE 2 \
		END AS status \
		FROM discussion_group \
		WHERE group_id = ?;";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBdiscussion_groupIsAllowSubmission: get connection from pool failed." << std::endl;
		return -1;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		pstmt->setInt64(1, group_id);

		std::unique_ptr<sql::ResultSet> ans_res(pstmt->executeQuery());
		if (ans_res && ans_res->next())
		{
			int ans = ans_res->getInt("status");

			return ans;
		}
		else
		{
			return -1;
		}
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::selectDBdiscussion_groupIsAllowSubmission: " << e.what() << std::endl;
		return -1;
	}

	return -1;
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBuser_submission(std::vector<std::string> limit, std::vector<int> value)
{
	std::string sql = "SELECT * FROM user_submission";
	if (!limit.empty())
	{
		sql = sql + " WHERE ";
		for (int i = 0; i < limit.size(); i++)
		{
			sql += limit[i] + " = ?";
			if (i < limit.size() - 1)
			{
				sql += " AND ";
			}
		}
	}
	sql = sql + ";";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBuser_submission: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		if (!limit.empty())
		{
			for (int i = 0; i < limit.size(); i++)
			{
				pstmt->setInt(i + 1, value[i]);
			}
		}
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::selectDBuser_submissionByRange: " << e.what() << std::endl;
		return nullptr;
	}

	return nullptr;
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBuser_submissionByRange(int64_t user_id, int begin, int size)
{
	std::string sql = "SELECT * FROM user_submission WHERE user_id = ? ORDER BY submit_at LIMIT ?, ?";

	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBuser_submissionByRange: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		pstmt->setInt64(1, user_id);
		pstmt->setInt(2, begin);
		pstmt->setInt(3, size);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::selectDBuser_submissionByRange: " << e.what() << std::endl;
		return nullptr;
	}
}

std::unique_ptr<PooledResultSet> DataAccess::selectDBuser_submissionForContest(int64_t group_id)
{
	std::string sql = "SELECT * FROM user_submission JOIN group_question ON user_submission.question_id = group_question.question_id WHERE group_question.group_id = ? ORDER BY user_submission.score DESC; ";
	
	std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
	if (!pooled_conn || !pooled_conn->get())
	{
		std::cerr << "[warn]DataAccess::selectDBuser_submissionByContest: get connection from pool failed." << std::endl;
		return nullptr;
	}
	try
	{
		sql::Connection* conn = pooled_conn->get();
		std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
		pstmt->setInt64(1, group_id);
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		return std::make_unique<PooledResultSet>(std::move(pooled_conn), std::move(pstmt), std::move(res));
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::selectDBuser_submissionByContest: " << e.what() << std::endl;
		return nullptr;
	}
}

int DataAccess::selectDBcount(DBType type, std::vector<std::string> limit, std::vector<int> value)
{
	std::string sql;
	switch (type)
	{
		case DBType::ADMIN:
			sql = "SELECT COUNT(*) AS count FROM admin";
			break;
		case DBType::STUDENT:
			sql = "SELECT COUNT(*) AS count FROM student";
			break;
		case DBType::TEACHER:
			sql = "SELECT COUNT(*) AS count FROM teacher";
			break;
		case DBType::QUESTION:
			sql = "SELECT COUNT(*) AS count FROM question";
			break;
		case DBType::USER:
			sql = "SELECT COUNT(*) AS count FROM user";
			break;
		case DBType::USER_QUESTION_STATUS:
			sql = "SELECT COUNT(*) AS count FROM user_question_status";
			break;
		case DBType::USER_GROUP_STATUS:
			sql = "SELECT COUNT(*) AS count FROM user_group_status";
			break;
		case DBType::USER_SUBMISSION:
			sql = "SELECT COUNT(*) AS count FROM user_submission";
			break;
		case DBType::DISCUSSION_GROUP:
			sql = "SELECT COUNT(*) AS count FROM discussion_group";
			break;
		case DBType::GROUP_QUESTION:
			sql = "SELECT COUNT(*) AS count FROM group_question";
			break;
		default:
			std::cerr << "[warn]DataAccess::selectDBcount: select type nonsupport." << std::endl;
			break;
	}
	if (!limit.empty())
	{
		sql = sql + " WHERE ";
		
		for (int i = 0; i < limit.size(); i++)
		{
			sql += limit[i] + " = ?";
			if (i < limit.size() - 1)
			{
				sql += " AND ";
			}
		}
	}
	sql += ";";

	try
	{
		std::unique_ptr<PooledConnection> pooled_conn = ConnectionPool::getInstance()->getConnection();
		if (pooled_conn && pooled_conn->get())
		{
			sql::Connection* conn = pooled_conn->get();
			std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
			if (!limit.empty())
			{
				for (int i = 0; i < limit.size(); i++)
				{
					pstmt->setInt(i + 1, value[i]);
				}
			}

			std::unique_ptr<sql::ResultSet> count_res(pstmt->executeQuery());
			
			int count = 0;
			if (count_res && count_res->next())
			{
				count = count_res->getInt("count");
			}
			
			return count;
		}
		else
		{
			std::cerr << "[warn]DataAccess::selectDBcount: get connection from pool failed." << std::endl;
			return 0;
		}
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[warn]DataAccess::selectDBcount:  [" << e.getErrorCode() << "]: " << e.what() << std::endl;
		return 0;
	}
}

bool DataAccess::updateDBquestionAudited(int64_t question_id, int is_audited, PooledConnection* external_conn)
{
	std::string sql = "UPDATE question SET is_audited = ? WHERE question_id = ?";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(is_audited));
	params.emplace_back(std::to_string(question_id));

	return this->updateDBData(sql, params, external_conn);
}

bool DataAccess::updateDBuser_question_statusStatus(int64_t question_id, int64_t user_id, int status, PooledConnection* external_conn)
{
	std::string sql = "UPDATE user_question_status SET status = ? WHERE question_id = ? AND user_id = ?";
	std::vector<std::string> params;
	params.emplace_back(std::to_string(status));
	params.emplace_back(std::to_string(question_id));
	params.emplace_back(std::to_string(user_id));

	return this->updateDBData(sql, params, external_conn);
}

bool DataAccess::updateDBuserInfo(int64_t user_id, std::string update_name, std::string update_value, PooledConnection* external_conn)
{
	if (update_name == "salt" || update_name == "password")
	{
		std::cerr << "[error]DataAccess::updateDBuserInfo: updateDBuserInfo can't update salt or password." << std::endl;
		return false;
	}

	std::string sql = "UPDATE user SET " + update_name + " = ? WHERE user_id = ?";
	std::vector<std::string> params;
	params.emplace_back(update_value);
	params.emplace_back(std::to_string(user_id));

	return this->updateDBData(sql, params, external_conn);
}

bool DataAccess::updateDBuserPassword(int64_t user_id, std::string password, PooledConnection* external_conn)
{
	std::string sql = "UPDATE user SET salt = ?, password = ? WHERE user_id = ?";
	std::vector<std::string> params;
	std::string salt = sha256::generateSalt();
	params.emplace_back(salt);
	params.emplace_back(sha256::hashPasswordBysha256(password, salt));
	params.emplace_back(std::to_string(user_id));

	return this->updateDBData(sql, params, external_conn);
}

bool DataAccess::updateDBuser_group_statusStatus(int64_t user_id, int64_t group_id, int status, int64_t user_group_status_id, PooledConnection* external_conn)
{
	std::string sql1 = "UPDATE user_group_status SET status = ? WHERE user_id = ? AND group_id = ?";
	std::string sql2 = "UPDATE user_group_status SET status = ? WHERE id = ?";

	std::vector<std::string> params;
	params.emplace_back(std::to_string(status));
	if (user_group_status_id != -1)
	{
		params.emplace_back(std::to_string(user_group_status_id));
		return this->updateDBData(sql2, params, external_conn);
	}
	params.emplace_back(std::to_string(user_id));
	params.emplace_back(std::to_string(group_id));
	return this->updateDBData(sql1, params, external_conn);
}

bool DataAccess::insertDBData(const std::string& sql, const std::vector<std::string>& params, int64_t& last_insert_id, PooledConnection* external_conn)
{
	last_insert_id = -1;
	sql::Connection* conn = nullptr;
	std::unique_ptr<PooledConnection> local_conn;

	if (external_conn && external_conn->get())
	{
		conn = external_conn->get();
	}
	else
	{
		local_conn = ConnectionPool::getInstance()->getConnection();
		if (!local_conn || !local_conn->get())
		{
			std::cerr << "[warn]DataAccess::insertDBData: get connection from pool failed." << std::endl;
			return false;
		}
		conn = local_conn->get();
	}

	if (!conn)
	{
		std::cerr << "[warn]DataAccess::insertDBData: invalid connection." << std::endl;
		return false;
	}

	try
	{
		std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(sql));
		if (!stmt)
		{
			std::cerr << "[warn]DataAccess::insertDBData: get PreparedStatement failed." << std::endl;
			return false;
		}

		if (external_conn && external_conn->get())
		{
			conn = external_conn->get();
		}
		else
		{
			conn = stmt->getConnection();
		}
		if (!conn)
		{
			std::cerr << "[warn]DataAccess::insertDBData: get connection from stmt failed." << std::endl;
			return false;
		}

		for (size_t i = 0; i < params.size(); i++)
		{
			if (params[i].empty())
			{
				std::cerr << "[warn]DataAccess::insertDBData: param " << (i + 1) << " is empty." << std::endl;
				return false;
			}
			if (params[i].length() > MYSQL_MAX_STRING_LENGTH)
			{
				std::cerr << "[warn]DataAccess::insertDBData: param " << (i + 1) << " is too long (>" << MYSQL_MAX_STRING_LENGTH << ")." << std::endl;
				return false;
			}
			stmt->setString(i + 1, params[i]);
		}

		if (stmt->executeUpdate() <= 0)
		{
			std::cerr << "[warn]DataAccess::insertDBData: insert failed, affected rows 0." << std::endl;
			return false;
		}

		std::unique_ptr<sql::Statement> stmt_conn(conn->createStatement());
		std::unique_ptr<sql::ResultSet> rs(stmt_conn->executeQuery("SELECT LAST_INSERT_ID()"));
		if (rs && rs->next())
		{
			last_insert_id = rs->getInt64(1);
		}
		else
		{
			std::cerr << "[warn]DataAccess::insertDBData: get last insert id failed." << std::endl;
			return false;
		}

		return true;
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::insertDBData: [" << e.getErrorCode() << "]: " << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[error]DataAccess::insertDBData: general exception: " << e.what() << std::endl;
		return false;
	}
}

bool DataAccess::updateDBData(const std::string& sql, const std::vector<std::string>& params, PooledConnection* external_conn)
{
	sql::Connection* conn = nullptr;
	std::unique_ptr<PooledConnection> local_conn;

	if (external_conn && external_conn->get())
	{
		conn = external_conn->get();
	}
	else
	{
		local_conn = ConnectionPool::getInstance()->getConnection();
		if (!local_conn || !local_conn->get())
		{
			std::cerr << "[warn]DataAccess::updateDBData: get connection from pool failed." << std::endl;
			return false;
		}
		conn = local_conn->get();
	}

	if (!conn)
	{
		std::cerr << "[warn]DataAccess::updateDBData: invalid connection." << std::endl;
		return false;
	}

	try
	{
		std::unique_ptr<sql::PreparedStatement> stmt(conn->prepareStatement(sql));
		if (!stmt)
		{
			std::cerr << "[warn]DataAccess::updateDBData: get PreparedStatement failed." << std::endl;
			return false;
		}

		if (external_conn && external_conn->get())
		{
			conn = external_conn->get();
		}
		else
		{
			conn = stmt->getConnection();
		}

		if (!conn)
		{
			std::cerr << "[warn]DataAccess::updateDBData: get connection from stmt failed." << std::endl;
			return false;
		}

		for (size_t i = 0; i < params.size(); i++)
		{
			if (params[i].empty())
			{
				std::cerr << "[warn]DataAccess::updateDBData: param " << (i + 1) << " is empty." << std::endl;
				return false;
			}
			if (params[i].length() > MYSQL_MAX_STRING_LENGTH)
			{
				std::cerr << "[warn]DataAccess::updateDBData: param " << (i + 1) << " is too long (>" << MYSQL_MAX_STRING_LENGTH << ")." << std::endl;
				return false;
			}
			stmt->setString(i + 1, params[i]);
		}

		if (stmt->executeUpdate() <= 0)
		{
			std::cerr << "[warn]DataAccess::updateDBData: insert failed, affected rows 0." << std::endl;
			return false;
		}

		return true;
	}
	catch (sql::SQLException& e)
	{
		std::cerr << "[error]DataAccess::updateDBData: [" << e.getErrorCode() << "]: " << e.what() << std::endl;
		return false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[error]DataAccess::updateDBData: general exception: " << e.what() << std::endl;
		return false;
	}
}