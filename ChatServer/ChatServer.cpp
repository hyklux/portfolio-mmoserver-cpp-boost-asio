#include "pch.h"
#include "ChatServer.h"
#include "IServer.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ChatServer] Creating zone server instance..." << endl;

	ChatServer* server = new ChatServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ChatServer] Chat server instance created." << endl;

	return 0;
}

int ChatServer::AddRef(void)
{
	return ++m_refs;
}

int ChatServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ChatServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ChatServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int ChatServer::OnLoad()
{
	cout << "[ChatServer] OnLoad" << endl;

	SetConnector();

	return 0;
}

int ChatServer::OnStart()
{
	cout << "[ChatServer] OnStart" << endl;
	return 0;
}

int ChatServer::OnUnload()
{
	cout << "[ChatServer] OnUnload" << endl;
	return 0;
}

int ChatServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_ENTER_GAME:
		//khy todo : user가 room 입장
		break;
	case MSG_C_CHAT:
		break;
	default:
		break;
	}

	return 0;
}

int ChatServer::SetConnector()
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
int ChatServer::Handle_C_CHAT(std::string msgStr)
{
	return 0;
}
