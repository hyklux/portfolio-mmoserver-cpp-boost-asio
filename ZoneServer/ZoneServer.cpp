#include "pch.h"
#include "ZoneServer.h"
#include "IServer.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ZoneServer] Creating zone server instance..." << endl;

	ZoneServer* server = new ZoneServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ZoneServer] Zone server instance created." << endl;

	return 0;
}

int ZoneServer::AddRef(void)
{
	return ++m_refs;
}

int ZoneServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ZoneServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ZoneServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int ZoneServer::OnLoad()
{
	cout << "[ZoneServer] OnLoad" << endl;
	return 0;
}

int ZoneServer::OnStart()
{
	cout << "[ZoneServer] OnStart" << endl;
	return 0;
}

int ZoneServer::OnUnload()
{
	cout << "[ZoneServer] OnUnload" << endl;
	return 0;
}

int ZoneServer::HandleMsg(NetMsg msg)
{
	cout << "[ZoneServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_CHAT:
		break;
	default:
		break;
	}

	return 0;
}

int ZoneServer::SetConnector()
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
int ZoneServer::Handle_C_CHAT(std::string msgStr)
{
	return 0;
}
