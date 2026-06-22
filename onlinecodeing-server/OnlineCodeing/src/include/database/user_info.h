#ifndef _USER_H_
#define _USER_H_

#include <string>
#include <ctime>

//新增表时需要更新部分，可全局搜索: *新增表需要更新*
enum class DBType
{
	ADMIN = 1,
	STUDENT = 2,
	TEACHER = 3,
	QUESTION,
	USER,
	USER_QUESTION_STATUS,
	USER_GROUP_STATUS,
	USER_SUBMISSION,
	DISCUSSION_GROUP,
	GROUP_QUESTION,
	UNKNOWN
};

std::string DBTypeToRoleString(DBType type);

//由于DBType中枚举数字与数据库中标记符相同，所以可以直接转，若需要自定义可以通过此处修改，不过同样需要修改数据库与DataAccess提供的函数操作
DBType RoleIntToDBType(int type);

DBType RoleStringToDBType(std::string dbtype);

enum class DBRole
{
	UNKNOW = -1,
	ADMIN = 1,
	STUDENT = 2,
	TEACHER = 3
};

class DBUser
{
public:
	//未注册构造
	DBUser(int user_id, std::string account, std::string phone, std::string email, std::string password);
	//已注册构造
	DBUser(int user_id, std::string account, std::string phone, std::string email, std::string password,
		std::string nickname, std::string user_desc, int role, int info_display_setting);
	//拷贝构造方便子类传参构造
	DBUser(const DBUser& user);

	//查看账号是否一致
	bool checkAccount(std::string account);
	//查看密码是否一致
	bool checkPassword(std::string password);

private:
	std::string m_salt;					//盐值
	std::tm m_create_time = {};			//账号创建时间

protected:
	//基础信息
	int m_user_id = -1;					//账号编号
	std::string m_account = "NULL";		//登录账号
	std::string m_phone = "NULL";		//手机号
	std::string m_email = "NULL";		//邮箱
	std::string m_password = "";		//加密密码

	//注册信息
	std::string m_nickname = "user";	//昵称
	std::string m_user_desc = "";		//个性签名
	int m_role = -1;					//用户角色

	//设置信息
	int m_info_display_setting = 0;		//按位存储对外显示设置

};

#endif // !_USER_H_
