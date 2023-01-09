#pragma once

#include <string>
#include <memory>

#include "NetGameSession.h"

class UserConnection
{
private:
	std::shared_ptr<NetGameSession> m_Session;

public:
	UserConnection(std::shared_ptr<NetGameSession> session) : m_Session(std::move(session))
	{
		cout << "UserConnection created." << endl;
	}

	void SetSession(std::shared_ptr<NetGameSession> session);
};

