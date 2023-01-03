#pragma once

#include "IServer.h"
#include "IServerContainer.h"

#include <map>
#include <wtypes.h>
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace::std;

class ServerContainer : public IServerContainer
{
private:
	std::atomic_int m_refs = 0;
	std::map<std::string, IServer*> m_ServerMap;

private:

public:
	ServerContainer(const std::string& appDir, const std::string& serverRevision)
	{
		std::string configDir;

#ifdef	MY_SOLUTIONDIR
		std::string solutionDir = MY_SOLUTIONDIR;
		configDir = solutionDir.append("Config\\config.json");
#else
		cout << "[ServerLauncher] Solution directory error." << endl;
		return;
#endif

		boost::property_tree::ptree props;
		boost::property_tree::read_json(configDir, props);
		cout << "[ServerLauncher] Config loaded." << endl;

		cout << "\n[ServerLauncher] Creating server instances..." << endl;

		auto servers = props.get_child("servers");
		for (const auto& server : servers)
		{
			auto name = server.second.get<std::string>("name");
			auto serverPair = m_ServerMap.emplace(name, new IServer());

			auto executable = server.second.get<std::string>("executable");
			HMODULE handle = LoadLibrary(executable.c_str());
			if (nullptr == handle)
			{
				cout << "DLL load error. GetLastError :" << GetLastError() << std::endl;
				continue;
			}

			typedef int(*CREATE_SERVER_FUNC)(IServerContainer* pServerContainer, IServer*& pServer);
			CREATE_SERVER_FUNC funcPtr = (CREATE_SERVER_FUNC)::GetProcAddress(handle, "CreateServerInstance");
			int ret = (*funcPtr)((IServerContainer*)this, serverPair.first->second);
			if (ret != 0)
			{
				cout << "Server create error." << endl;
				FreeLibrary(handle);
				continue;
			}
		}
	}

	//https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=kmc7468&logNo=221269056289
	~ServerContainer()
	{
	}

	int32_t Load();
	int32_t Start();
	int32_t Unload();

	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;
	virtual void* GetConnectorServer() override;
	virtual int DispatchMsg(uint16_t targetServer, NetMsg msg) override;
};