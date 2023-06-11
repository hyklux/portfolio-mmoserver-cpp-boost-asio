#include "pch.h"
#include "ZoneServer.h"
#include "IServerModule.h"

#include "Protocol.pb.h"

#define TICK_LENGTH_MILLISEC 16 //60 ticks per second

int CreateServerInstance(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[ZoneModule] Creating zone module instance..." << endl;

	ZoneServer* server = new ZoneServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ZoneModule] Zone module instance created." << endl;

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

int ZoneServer::OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[ZoneModule] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServerModule*>(this);
	pServer->AddRef();

	return 0;
}

int ZoneServer::OnLoad()
{
	cout << "[ZoneModule] OnLoad" << endl;

	SetConnector();

	return 0;
}

int ZoneServer::OnStart()
{
	cout << "[ZoneModule] OnStart" << endl;

	InitZone();
	RunTick();

	return 0;
}

int ZoneServer::OnUnload()
{
	cout << "[ZoneModule] OnUnload" << endl;

	m_CanTick = false;

	//존에 존재하는 모든 플레이어 삭제
	m_PlayerList.clear();

	//존에 존재하는 모든 몬스터 삭제
	m_MonsterList.clear();

	return 0;
}

int ZoneServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
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

int ZoneServer::SetConnector()
{
	if (!m_pServerContainer)
	{
		return -1;
	}

	void* pContainerPtr = m_pServerContainer->GetConnectorServer();
	if (pContainerPtr)
	{
		m_pConnectorServer = static_cast<IServerModule*>(pContainerPtr);
	}

	return m_pConnectorServer ? 0 : -1;
}

void ZoneServer::InitZone()
{
	CreateNPCs();
}

void ZoneServer::RunTick()
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
void ZoneServer::Tick(float deltaTime)
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

void ZoneServer::CreateNPCs()
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
int ZoneServer::Handle_C_ENTER_GAME(NetMsg msg)
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
