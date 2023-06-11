#include "pch.h"
#include "ChatServer.h"
#include "IServerModule.h"
#include "NetGameSession.h"

#include "Protocol.pb.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[ChatModule] Creating chat module instance..." << endl;

	ChatServer* server = new ChatServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ChatModule] Chat module instance created." << endl;

	return 0;
}

int ChatServer::AddRef(void)
{
	return ++m_refs;
}

int ChatServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ChatServer::OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[ChatModule] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServerModule*>(this);
	pServer->AddRef();

	return 0;
}

int ChatServer::OnLoad()
{
	cout << "[ChatModule] OnLoad" << endl;

	SetConnector();

	return 0;
}

int ChatServer::OnStart()
{
	cout << "[ChatModule] OnStart" << endl;
	return 0;
}

int ChatServer::OnUnload()
{
	cout << "[ChatModule] OnUnload" << endl;
	return 0;
}

int ChatServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatModule] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_ENTER_GAME:
		Handle_C_ENTER_GAME(msg, session);
		break;
	case MSG_C_CHAT:
		Handle_C_CHAT(msg, session);
		break;
	default:
		break;
	}

	return 0;
}

int ChatServer::SetConnector()
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

void ChatServer::BroadCastAll(std::string broadcastMsgStr)
{
	NetMsg broadcastMsg;
	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(broadcastMsgStr);
	broadcastMsg.MakeBuffer(chatPkt, MSG_S_CHAT);

	for (std::shared_ptr<NetGameSession> session : m_UserSessionList)
	{
		session->SendMsgToClient(broadcastMsg);
	}
}

//handlers
int ChatServer::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatModule] Handle_C_ENTER_GAME" << endl;

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatModule] Player" << to_string(pkt.playerid()) << " has entered chat room." << endl;

	m_UserSessionList.push_back(session);

	return 0;
}

int ChatServer::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatModule] Handle_C_CHAT" << endl;

	//패킷 분해
	Protocol::C_CHAT pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatModule] [Player" << to_string(pkt.playerid()+1) << "] " << pkt.msg() << endl;

	std::string broadcastMsgStr = "[Player" + to_string(pkt.playerid()+1) + "] : " + pkt.msg();
	BroadCastAll(broadcastMsgStr);

	return 0;
}