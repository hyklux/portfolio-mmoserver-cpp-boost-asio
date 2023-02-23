#include "pch.h"
#include "ZoneServer.h"
#include "IServer.h"

#include "Protocol.pb.h"

#define TICK_LENGTH_MILLISEC 16 //60 ticks per second

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ZoneServer] Creating zone server instance..." << endl;

	ZoneServer* server = new ZoneServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ZoneServer] Zone server instance created." << endl;

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

int ZoneServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ZoneServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int ZoneServer::OnLoad()
{
	cout << "[ZoneServer] OnLoad" << endl;

	SetConnector();

	return 0;
}

int ZoneServer::OnStart()
{
	cout << "[ZoneServer] OnStart" << endl;

	RunTick();

	return 0;
}

int ZoneServer::OnUnload()
{
	cout << "[ZoneServer] OnUnload" << endl;

	m_CanTick = false;

	return 0;
}

int ZoneServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ZoneServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

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
		m_pConnectorServer = static_cast<IServer*>(pContainerPtr);
	}

	return m_pConnectorServer ? 0 : -1;
}

void ZoneServer::RunTick()
{
	m_CanTick = true;

	while (m_CanTick)
	{
		Tick();
		std::this_thread::sleep_for(std::chrono::milliseconds(TICK_LENGTH_MILLISEC));
	}
}

void ZoneServer::Tick()
{
	//매 프레임 처리해야 할 작업 수행
}

//handlers
int ZoneServer::Handle_C_ENTER_GAME(NetMsg msg)
{
	cout << "[ZoneServer] Handle_C_ENTER_GAME" << endl;

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	std::string playerName = "Player" + to_string(pkt.playerid());

	cout << "[ZoneServer] " << playerName << " entering game..." << endl;

	m_PlayerList.push_back(new CPlayer(pkt.playerid(), playerName));

	cout << "[ZoneServer] " << playerName << " enter game success." << endl;

	return 0;
}
