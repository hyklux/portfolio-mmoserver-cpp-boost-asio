#pragma once

#include "IServer.h"
#include "IServerContainer.h"

#include "Protocol.pb.h"

#include <iostream>
#include <vector>
#include <sdkddkver.h>
#include <unordered_map> //이건 왜 ? 
//다중접속 서버이기 때문에. 소켓 여러개의 연결을 유지하기 위해 그걸 담아둘 컨테이너.
//클라 구조체를 만들어서 그걸 담아두도록 하자.

#include <functional>
#include <tbb/concurrent_queue.h>
#include <memory>

#include "ThreadPool.h"

using namespace std;

extern "C" __declspec(dllexport) int CreateServerInstance(IServerContainer * pServerContainer, IServer * &pServer);

class UserConnection;
class NetGameSession;

class UserServer : public IServer
{
private:
	std::atomic_int m_refs = 0;
	IServerContainer* m_pServerContainer;
	IServer* m_pConnectorServer;
	vector<UserConnection> userConnectionList;
	int8_t m_JobQueueThreadCnt = 3;
	ThreadPool m_ThreadPool;

public:
	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;

	virtual int OnCreate(IServerContainer* pServerContainer, IServer*& pServer) override;
	virtual int OnLoad() override;
	virtual int OnStart() override;
	virtual int OnUnload() override;

	virtual int HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session) override;

	// Handlers
	EResultType Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
	EResultType Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
	EResultType Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
private:
	virtual int SetConnector() override;

	EResultType Login(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
	EResultType EnterGame(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
	EResultType Chat(const NetMsg msg, const std::shared_ptr<NetGameSession>& session);
};

