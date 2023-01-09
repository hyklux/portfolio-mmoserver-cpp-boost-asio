#include "pch.h"
#include "UserServer.h"
#include "IServer.h"
#include "NetGameSession.h"
#include "NetMsg.h"
#include "UserConnection.h"

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

	for (int i = 0; i < m_JobQueueThreadCnt; ++i)
	{
		/*
		std::unique_ptr<std::thread> thr = std::make_unique<std::thread>([this]()
		{
			CallbackType callback;

			while (m_JobQueue.try_pop(callback))
			{
				callback();
			}
		});
		*/
	}

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

	//m_JobQueue.Terminate();

	return 0;
}

//JobQueue에 넣어준다.
int UserServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_LOGIN:
		Handle_C_LOGIN(msg, session);
		break;
	case MSG_C_ENTER_GAME:
		Handle_C_ENTER_GAME(msg, session);
		break;
	default:
		break;
	}

	return 0;
}

void UserServer::CreateUserConnection(std::shared_ptr<NetGameSession> session)
{
	UserConnection* userConnection = new UserConnection(session);
	//userConnection.SetSession(session);
	//userConnectionList.push_back(userConnection);
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

uint16_t UserServer::Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//패킷 분해
	Protocol::C_LOGIN pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	//해야할 작업을 MsgJobQueue에 함수 + 인자를 넣는다.
	//로그인
	Login(pkt.username());

	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	NetMsg resMsg;
	resMsg.MakeBuffer(loginPkt, MSG_S_LOGIN);

	session->SendMsgToClient(resMsg);

	return 0;
}

uint16_t UserServer::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	//해야할 작업을 MsgJobQueue에 함수 + 인자를 넣는다.
	//m_JobQueue.push(UserServer::EnterGame);

	return 0;
}

void UserServer::Login(std::string userName)
{
	cout << "[UserServer] " << userName << " logging in..." << endl;
	cout << "[UserServer] " << userName << " login success." << endl;

	//create user connection
}

void UserServer::EnterGame()
{
	//cout << "[UserServer] " << m_Username << "entering game..." << endl;
}