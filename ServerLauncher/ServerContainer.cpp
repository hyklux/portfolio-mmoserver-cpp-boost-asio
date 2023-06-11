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

void* ServerContainer::GetConnectorModule()
{
	const auto itr = m_ServerMap.find("ConnectorModule");
	return itr == m_ServerMap.end() ? nullptr : (itr->second);
}

void* ServerContainer::GetUserModule()
{
	const auto itr = m_ServerMap.find("UserModule");
	return itr == m_ServerMap.end() ? nullptr : (itr->second);
}

void* ServerContainer::GetTargetModule(uint16_t targetModule)
{
	void* pTargetModule = nullptr;

	switch (targetModule)
	{
	case EUserModule:
	{
		auto itr = m_ServerMap.find("UserModule");
		if (itr != m_ServerMap.end())
		{
			pTargetModule = itr->second;
		}
		break;
	}
	case EZoneModule:
	{
		auto itr = m_ServerMap.find("ZoneModule");
		if (itr != m_ServerMap.end())
		{
			pTargetModule = itr->second;
		}
		break;
	}
	case EChatModule:
	{
		auto itr = m_ServerMap.find("ChatModule");
		if (itr != m_ServerMap.end())
		{
			pTargetModule = itr->second;
		}
		break;
	}
	case EDBAgentModule:
	{
		auto itr = m_ServerMap.find("DBAgentModule");
		if (itr != m_ServerMap.end())
		{
			pTargetModule = itr->second;
		}
		break;
	}
	default:
		break;
	}

	return pTargetModule;
}
