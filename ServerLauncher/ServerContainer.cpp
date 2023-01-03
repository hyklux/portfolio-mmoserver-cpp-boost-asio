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

	/*
	for (const auto& server : m_ServerMap)
	{
		server.second->OnUnload();
	}
	*/

	ReleaseRef();

	return 0;
}

void* ServerContainer::GetConnectorServer()
{
	const auto itr = m_ServerMap.find("ConnectorServer");
	return itr == m_ServerMap.end() ? nullptr : &(itr->second);
}

int ServerContainer::DispatchMsg(uint16_t targetServer, NetMsg msg)
{
	IServer* pTargetServer = nullptr;

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
	default:
		break;
	}

	if (pTargetServer == nullptr)
	{
		cout << "[NetworkServer][Error] No valid target server." << endl;
		return -1;
	}

	pTargetServer->HandleMsg(msg);

	return 0;
}
