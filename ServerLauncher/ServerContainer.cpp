#include "ServerContainer.h"

int ServerContainer::AddRef(void)
{
	return ++m_refs;
}

int ServerContainer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int32_t ServerContainer::Load()
{
	cout << "\n[ServerContainer] Load" << endl;

	for (const auto& server : m_ServerMap)
	{
		server.second->OnLoad();
	}

	AddRef();

	return 0;
}

int32_t ServerContainer::Start()
{
	cout << "\n[ServerContainer] Start" << endl;

	for (const auto& server : m_ServerMap)
	{
		server.second->OnStart();
	}

	return 0;
}

int32_t ServerContainer::Unload()
{
	cout << "\nSEMainContainer::Unload" << endl;

	for (const auto& server : m_ServerMap)
	{
		server.second->OnUnload();
	}

	ReleaseRef();

	return 0;
}

void* ServerContainer::GetConnectorServer()
{
	const auto itr = m_ServerMap.find("ConnectorServer");
	return itr == m_ServerMap.end() ? nullptr : &(itr->second);
}

void* ServerContainer::GetUserServer()
{
	const auto itr = m_ServerMap.find("UserServer");
	return itr == m_ServerMap.end() ? nullptr : (itr->second);
}

void* ServerContainer::GetTargetServer(uint16_t targetServer)
{
	void* pTargetServer = nullptr;

	switch (targetServer)
	{
	case EUserServer:
	{
		auto itr = m_ServerMap.find("UserServer");
		if (itr != m_ServerMap.end())
		{
			pTargetServer = itr->second;
		}
		break;
	}
	case EZoneServer:
	{
		auto itr = m_ServerMap.find("ZoneServer");
		if (itr != m_ServerMap.end())
		{
			pTargetServer = itr->second;
		}
		break;
	}
	case EChatServer:
	{
		auto itr = m_ServerMap.find("ChatServer");
		if (itr != m_ServerMap.end())
		{
			pTargetServer = itr->second;
		}
		break;
	}
	default:
		break;
	}

	return pTargetServer;
}
