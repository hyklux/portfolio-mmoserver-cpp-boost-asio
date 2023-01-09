#include "pch.h"
#include "UserConnection.h"
#include "NetGameSession.h"


void UserConnection::SetSession(std::shared_ptr<NetGameSession> session)
{
	m_Session = std::move(session);
}
