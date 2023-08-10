#pragma once

#include "IServerModule.h"
#include "IServerContainer.h"
#include "DBConn.h"

#include "Protocol.pb.h"

#include <iostream>
#include <vector>
#include <string>
#include <sdkddkver.h>
#include <unordered_map> //이건 왜 ? 
//다중접속 서버이기 때문에. 소켓 여러개의 연결을 유지하기 위해 그걸 담아둘 컨테이너.
//클라 구조체를 만들어서 그걸 담아두도록 하자.

#include <memory>


using namespace std;

extern "C" __declspec(dllexport) int CreateServerModuleInstance(IServerContainer * pServerContainer, IServerModule * &pServer);

class DBAgentModule : public IServerModule
{
private:
	std::atomic_int m_refs = 0;
	IServerContainer* m_pServerContainer;
	IServerModule* m_pConnectorModule;
	DBConn m_DbConn;
	int m_UserId = 0;

public:
	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;

	virtual int OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer) override;
	virtual int OnLoad() override;
	virtual int OnStart() override;
	virtual int OnUnload() override;

	virtual int HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session) override;

	bool ExistsUserInDB(std::string userName);
	int CreateUserToDB(std::string userName);

private:
	virtual int SetConnector() override;

	int ConnectToDB();
	int InitDBTable();

	//handlers
	int Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
};

