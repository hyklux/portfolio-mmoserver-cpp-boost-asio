#include "pch.h"
#include "UserServer.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[UserServer] Creating user server instance..." << endl;

	UserServer* server = new UserServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[UserServer] User server instance created." << endl;

	return 0;
}

int UserServer::AddRef(void)
{
	return ++m_refs;
}

int UserServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int UserServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[UserServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer =  pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int UserServer::OnLoad()
{
	cout << "[UserServer] OnLoad" << endl;
	return 0;
}

int UserServer::OnStart()
{
	cout << "[UserServer] OnStart" << endl;
	return 0;
}

int UserServer::OnUnload()
{
	cout << "[UserServer] OnUnload" << endl;
	return 0;
}

int UserServer::HandleMsg(NetMsg msg)
{
	cout << "[UserServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_LOGIN:
		Handle_C_LOGIN(msg);
		break;
	case MSG_C_ENTER_GAME:
		Handle_C_ENTER_GAME(msg);
		break;
	default:
		break;
	}

	return 0;
}

int UserServer::SetConnector()
{
	if (!m_pServerContainer)
	{
		return -1;
	}

	void* pContainerPtr = m_pServerContainer->GetConnectorServer();
	if (pContainerPtr)
	{
		m_pConnectorServer = static_cast<IServer*>(pContainerPtr);
	}

	return m_pConnectorServer ? 0 : -1;
}

//handlers
int UserServer::Handle_C_LOGIN(NetMsg msg)
{
	return 0;
}

int UserServer::Handle_C_ENTER_GAME(NetMsg msg)
{
	return 0;
}
