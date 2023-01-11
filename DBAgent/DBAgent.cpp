#include "pch.h"
#include "DBAgent.h"
#include "IServer.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[DBAgent] Creating zone server instance..." << endl;

	DBAgent* server = new DBAgent();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[DBAgent] Zone server instance created." << endl;

	return 0;
}

int DBAgent::AddRef(void)
{
	return ++m_refs;
}

int DBAgent::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int DBAgent::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[DBAgent] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int DBAgent::OnLoad()
{
	cout << "[DBAgent] OnLoad" << endl;
	return 0;
}

int DBAgent::OnStart()
{
	cout << "[DBAgent] OnStart" << endl;
	return 0;
}

int DBAgent::OnUnload()
{
	cout << "[DBAgent] OnUnload" << endl;
	return 0;
}

int DBAgent::SetConnector()
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
int DBAgent::Handle_C_CHAT(std::string msgStr)
{
	return 0;
}
