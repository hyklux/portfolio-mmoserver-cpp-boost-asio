#pragma once

#include <string>
#include <memory>

class UserConnection
{
	std::string	m_UserName;
	std::shared_ptr<NetGameSession> m_Session;

public:
	UserConnection(std::string userName, std::shared_ptr<NetGameSession> session) : m_UserName(userName), m_Session(session)
	{

	}
};

