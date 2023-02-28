#include "pch.h"
#include "UserServer.h"
#include "IServer.h"
#include "NetGameSession.h"
#include "NetMsg.h"
#include "UserConnection.h"
#include "Types.h"

#include "Protocol.pb.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[UserServer] Creating user server instance..." << endl;

	UserServer* server = new UserServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[UserServer] User server instance created." << endl;

	return 0;
}

int UserServer::AddRef(void)
{
	return ++m_refs;
}

int UserServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int UserServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[UserServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int UserServer::OnLoad()
{
	cout << "[UserServer] OnLoad" << endl;

	SetConnector();
	m_ThreadPool.Activate(m_JobQueueThreadCnt);

	return 0;
}

int UserServer::OnStart()
{
	cout << "[UserServer] OnStart" << endl;
	return 0;
}

int UserServer::OnUnload()
{
	cout << "[UserServer] OnUnload" << endl;
	return 0;
}

//JobQueue에 넣어준다.
int UserServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_LOGIN:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_LOGIN(msg, session);
		});
		break;
	case MSG_C_ENTER_GAME:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_ENTER_GAME(msg, session);
		});
		break;
	case MSG_C_CHAT:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_CHAT(msg, session);
		});
		break;
	default:
		break;
	}

	return 0;
}

int UserServer::SetConnector()
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

EResultType UserServer::Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//로그인
	Protocol::S_LOGIN loginPkt;

	if (EResultType::SUCCESS == Login(msg, session))
	{
		loginPkt.set_success(true);
	}

	NetMsg resMsg;
	resMsg.MakeBuffer(loginPkt, MSG_S_LOGIN);
	session->SendMsgToClient(resMsg);

	return EResultType::SUCCESS;
}

EResultType UserServer::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//게임 진입
	Protocol::S_ENTER_GAME enterGamePkt;

	if (EResultType::SUCCESS == EnterGame(msg, session))
	{
		enterGamePkt.set_success(true);
	}

	NetMsg resMsg;
	resMsg.MakeBuffer(enterGamePkt, MSG_S_ENTER_GAME);
	session->SendMsgToClient(resMsg);

	return EResultType::SUCCESS;
}

EResultType UserServer::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	return Chat(msg, session);
}

EResultType UserServer::Login(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] Login" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserServer] Error : Connector server is null." << endl;
		return EResultType::NULL_ERROR;
	}

	m_pConnectorServer->DispatchMsgToServer(EDBAgent, msg, session);

	return EResultType::SUCCESS;
}

EResultType UserServer::EnterGame(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] EnterGame" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserServer] Error : Connector server is null." << endl;
		return EResultType::SUCCESS;
	}

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return EResultType::PKT_ERROR;
	}

	session->SetPlayerId(pkt.playerid());
	m_pConnectorServer->DispatchMsgToServer(EZoneServer, msg, session);
	m_pConnectorServer->DispatchMsgToServer(EChatServer, msg, session);

	return EResultType::SUCCESS;
}

EResultType UserServer::Chat(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] Chat" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserServer] Error : Connector server is null." << endl;
		return EResultType::NULL_ERROR;
	}

	m_pConnectorServer->DispatchMsgToServer(EChatServer, msg, session);

	return EResultType::SUCCESS;
}
