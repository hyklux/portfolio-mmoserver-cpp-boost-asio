#include "pch.h"
#include "UserServer.h"
#include "IServerModule.h"
#include "NetGameSession.h"
#include "NetMsg.h"
#include "UserConnection.h"
#include "Types.h"

#include "Protocol.pb.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[UserModule] Creating user module instance..." << endl;

	UserServer* server = new UserServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[UserModule] User module instance created." << endl;

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

int UserServer::OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[UserModule] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServerModule*>(this);
	pServer->AddRef();

	return 0;
}

int UserServer::OnLoad()
{
	cout << "[UserModule] OnLoad" << endl;

	SetConnector();
	m_ThreadPool.Activate(m_JobQueueThreadCnt);

	return 0;
}

int UserServer::OnStart()
{
	cout << "[UserModule] OnStart" << endl;
	return 0;
}

int UserServer::OnUnload()
{
	cout << "[UserModule] OnUnload" << endl;
	return 0;
}

//JobQueue에 넣어준다.
int UserServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] HandleMsg. PktId:" << msg.GetPktId() << endl;

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
		m_pConnectorServer = static_cast<IServerModule*>(pContainerPtr);
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
	cout << "[UserModule] Login" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
		return EResultType::NULL_ERROR;
	}

	m_pConnectorServer->DispatchMsgToServer(EDBAgentModule, msg, session);

	return EResultType::SUCCESS;
}

EResultType UserServer::EnterGame(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] EnterGame" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
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
	cout << "[UserModule] Chat" << endl;

	if (!m_pConnectorServer)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
		return EResultType::NULL_ERROR;
	}

	m_pConnectorServer->DispatchMsgToServer(EChatModule, msg, session);

	return EResultType::SUCCESS;
}
