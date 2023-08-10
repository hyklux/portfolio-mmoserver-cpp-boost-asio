#include "pch.h"
#include "ZoneModule.h"
#include "IServerModule.h"

#include "Protocol.pb.h"

int CreateServerModuleInstance(IServerContainer* pServerContainer, IServerModule*& pModule)
{
	cout << "[ZoneModule] Creating zone module instance..." << endl;

	ZoneModule* module = new ZoneModule();
	if (nullptr == module)
	{
		return -1;
	}

	module->OnCreate(pServerContainer, pModule);

	cout << "[ZoneModule] Zone module instance created." << endl;

	return 0;
}

int ZoneModule::AddRef(void)
{
	return ++m_refs;
}

int ZoneModule::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ZoneModule::OnCreate(IServerContainer* pServerContainer, IServerModule*& pModule)
{
	cout << "[ZoneModule] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pModule = static_cast<IServerModule*>(this);
	pModule->AddRef();

	return 0;
}

int ZoneModule::OnLoad()
{
	cout << "[ZoneModule] OnLoad" << endl;

	SetConnector();

	return 0;
}

int ZoneModule::OnStart()
{
	cout << "[ZoneModule] OnStart" << endl;

	InitZone();
	RunTick();

	return 0;
}

int ZoneModule::OnUnload()
{
	cout << "[ZoneModule] OnUnload" << endl;

	m_CanTick = false;

	//존에 존재하는 모든 플레이어 삭제
	m_PlayerList.clear();

	//존에 존재하는 모든 몬스터 삭제
	m_MonsterList.clear();

	return 0;
}

int ZoneModule::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ZoneModule] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_ENTER_GAME:
		Handle_C_ENTER_GAME(msg);
		break;
	case MSG_C_CHAT:
		break;
	default:
		break;
	}

	return 0;
}

int ZoneModule::SetConnector()
{
	if (!m_pServerContainer)
	{
		return -1;
	}

	void* pContainerPtr = m_pServerContainer->GetConnectorModule();
	if (pContainerPtr)
	{
		m_pConnectorModule = static_cast<IServerModule*>(pContainerPtr);
	}

	return m_pConnectorModule ? 0 : -1;
}

void ZoneModule::InitZone()
{
	CreateNPCs();
}

void ZoneModule::RunTick()
{
	m_CanTick = true;

	while (m_CanTick)
	{
		// 마지막 틱으로 부터 경과한 시간(deltaTime) 계산
		std::clock_t currentTime = std::clock();
		float deltaTime = ((float)(currentTime - lastTickTime)) / CLOCKS_PER_SEC;

		// deltaTime이 기준 프레임 시간 이상 되었는지 확인
		if (deltaTime >= (1.0f / TICK_RATE)) 
		{
			//Tick함수 실행
			Tick(deltaTime);

			lastTickTime = currentTime;
		}
	}
}

//매 프레임 처리해야 할 작업 수행
void ZoneModule::Tick(float deltaTime)
{
	for (auto player : m_PlayerList)
	{
		player->Update(deltaTime);
	}

	for (auto monster : m_MonsterList)
	{
		monster->Update(deltaTime);
	}
}

void ZoneModule::CreateNPCs()
{
	cout << "[ZoneModule] CreateNPCs" << endl;

	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<CMonster> monster(new CMonster(1000 + i, "Monster" + i));
		monster->SetPosition(i, 0);
		m_MonsterList.push_back(monster);
	}
}

//handlers
int ZoneModule::Handle_C_ENTER_GAME(NetMsg msg)
{
	cout << "[ZoneModule] Handle_C_ENTER_GAME" << endl;

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	std::string playerName = "Player" + to_string(pkt.playerid());

	cout << "[ZoneModule] " << playerName << " entering game..." << endl;

	std::shared_ptr<CPlayer> player(new CPlayer(pkt.playerid(), playerName));
	m_PlayerList.push_back(player);

	cout << "[ZoneModule] " << playerName << " enter game success." << endl;

	return 0;
}
