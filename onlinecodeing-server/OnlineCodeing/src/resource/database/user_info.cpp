#include "database/user_info.h"
#include "utils/security.h"

std::string DBTypeToRoleString(DBType type)
{
	switch (type)
	{
	case DBType::ADMIN:
		return "admin";
	case DBType::STUDENT:
		return "student";
	case DBType::TEACHER:
		return "teacher";
	default:
		return "unknown";
	}
}

DBType RoleIntToDBType(int type)
{
	switch (type)
	{
	case 1:
		return DBType::ADMIN;
	case 2:
		return DBType::STUDENT;
	case 3:
		return DBType::TEACHER;
	default:
		break;
	}
	return DBType::UNKNOWN;
}

DBType RoleStringToDBType(std::string dbtype)
{
	if (dbtype == "admin" || dbtype == "ADMIN") return DBType::ADMIN;
	if (dbtype == "teacher" || dbtype == "TEACHER") return DBType::TEACHER;
	if (dbtype == "student" || dbtype == "STUDENT") return DBType::STUDENT;

	return DBType::UNKNOWN;
}

DBUser::DBUser(int user_id, std::string account, std::string phone, std::string email, std::string password)
	: DBUser(user_id, account, phone, email, password, "user_" + user_id, "", -1, 0)
{
	return;
}

DBUser::DBUser(int user_id, std::string account, std::string phone, std::string email, std::string password, std::string nickname, std::string user_desc, int role, int info_display_setting)
	: m_user_id(user_id), m_account(account), m_phone(phone), m_email(email), m_nickname(nickname), m_user_desc(user_desc), m_role(role), m_info_display_setting(info_display_setting)
{
	this->m_salt = sha256::generateSalt();
	this->m_password = sha256::hashPasswordBysha256(password, this->m_salt);
	return;
}

DBUser::DBUser(const DBUser& user)
	: m_user_id(user.m_user_id), m_account(user.m_account), m_phone(user.m_phone), m_email(user.m_email), m_nickname(user.m_nickname),
	m_user_desc(user.m_user_desc), m_role(user.m_role), m_info_display_setting(user.m_info_display_setting)
{
	return;
}

bool DBUser::checkAccount(std::string account)
{
	if (this->m_user_id == -1) return false;
	return this->m_account == account;
}

bool DBUser::checkPassword(std::string password)
{
	if (this->m_user_id == -1) return false;
	return this->m_password == sha256::hashPasswordBysha256(password, this->m_salt);
}
