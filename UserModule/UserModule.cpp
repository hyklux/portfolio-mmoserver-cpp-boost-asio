#include "pch.h"
#include "UserModule.h"
#include "IServerModule.h"
#include "NetGameSession.h"
#include "NetMsg.h"
#include "UserConnection.h"
#include "Types.h"

#include "Protocol.pb.h"

int CreateServerModuleInstance(IServerContainer* pServerContainer, IServerModule*& pModule)
{
	cout << "[UserModule] Creating user module instance..." << endl;

	UserModule* userModule = new UserModule();
	if (nullptr == userModule)
	{
		return -1;
	}

	userModule->OnCreate(pServerContainer, pModule);

	cout << "[UserModule] User module instance created." << endl;

	return 0;
}

int UserModule::AddRef(void)
{
	return ++m_refs;
}

int UserModule::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int UserModule::OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer)
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

int UserModule::OnLoad()
{
	cout << "[UserModule] OnLoad" << endl;

	SetConnector();
	m_ThreadPool.Activate(m_JobQueueThreadCnt);
	m_UserWorkerPool.Init(m_JobQueueThreadCnt);

	return 0;
}

int UserModule::OnStart()
{
	cout << "[UserModule] OnStart" << endl;
	return 0;
}

int UserModule::OnUnload()
{
	cout << "[UserModule] OnUnload" << endl;

	m_UserWorkerPool.Uninit();

	return 0;
}

//JobQueue에 넣어준다.
int UserModule::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
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

int UserModule::SetConnector()
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

uint16_t UserModule::Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//로그인
	Protocol::S_LOGIN loginPkt;

	if (0 == Login(msg, session))
	{
		loginPkt.set_success(true);
	}

	NetMsg resMsg;
	resMsg.MakeBuffer(loginPkt, MSG_S_LOGIN);
	session->SendMsgToClient(resMsg);

	return 0;
}

uint16_t UserModule::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	//게임 진입
	Protocol::S_ENTER_GAME enterGamePkt;

	if (0 == EnterGame(msg, session))
	{
		enterGamePkt.set_success(true);
	}

	NetMsg resMsg;
	resMsg.MakeBuffer(enterGamePkt, MSG_S_ENTER_GAME);
	session->SendMsgToClient(resMsg);

	return 0;
}

uint16_t UserModule::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	return Chat(msg, session);
}

uint16_t UserModule::Login(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] Login" << endl;

	if (!m_pConnectorModule)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
		return static_cast<uint16_t>(ERRORTYPE::NULL_ERROR);
	}

	m_pConnectorModule->DispatchMsgToServer(EDBAgentModule, msg, session);

	return static_cast<uint16_t>(ERRORTYPE::NONE_ERROR);
}

uint16_t UserModule::EnterGame(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] EnterGame" << endl;

	if (!m_pConnectorModule)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
		return static_cast<uint16_t>(ERRORTYPE::NULL_ERROR);
	}

	m_pConnectorModule->DispatchMsgToServer(EZoneModule, msg, session);
	m_pConnectorModule->DispatchMsgToServer(EChatModule, msg, session);

	return static_cast<uint16_t>(ERRORTYPE::NONE_ERROR);
}

uint16_t UserModule::Chat(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] Chat" << endl;

	if (!m_pConnectorModule)
	{
		cout << "[UserModule] Error : Connector module is null." << endl;
		return static_cast<uint16_t>(ERRORTYPE::NULL_ERROR);
	}

	m_pConnectorModule->DispatchMsgToServer(EChatModule, msg, session);

	return static_cast<uint16_t>(ERRORTYPE::NONE_ERROR);
}
