#pragma once

#include "IServerModule.h"
#include "IServerContainer.h"
#include "CPlayer.h"
#include "CMonster.h"

#include <iostream>
#include <vector>
#include <string>
#include <sdkddkver.h>
#include <unordered_map> //이건 왜 ? 
//다중접속 서버이기 때문에. 소켓 여러개의 연결을 유지하기 위해 그걸 담아둘 컨테이너.
//클라 구조체를 만들어서 그걸 담아두도록 하자.

#include <chrono>
#include <memory>

using namespace std;

extern "C" __declspec(dllexport) int CreateServerModuleInstance(IServerContainer * pServerContainer, IServerModule*& pModule);

class ZoneModule : public IServerModule
{
private:
	std::atomic_int m_refs = 0;
	IServerContainer* m_pServerContainer;
	IServerModule* m_pConnectorModule;
	
	bool m_CanTick = false;

	// Declare a constant for the desired tick rate (in ticks per second)
	const float TICK_RATE = 10.0f;

	// Declare a variable to store the last tick time
	std::clock_t lastTickTime;

	std::vector<std::shared_ptr<CPlayer>> m_PlayerList;
	std::vector<std::shared_ptr<CMonster>> m_MonsterList;

public:
	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;

	virtual int OnCreate(IServerContainer* pServerContainer, IServerModule*& pModule) override;
	virtual int OnLoad() override;
	virtual int OnStart() override;
	virtual int OnUnload() override;

	virtual int HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session) override;

private:
	virtual int SetConnector() override;
	void InitZone();
	void RunTick();
	void Tick(float deltaTime);
	void CreateNPCs();

	//handlers
	int Handle_C_ENTER_GAME(NetMsg msg);
};

