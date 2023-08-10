#pragma once

#pragma once

#include <string>
#include "IServerContainer.h"
#include "NetMsg.h"
#include <memory>

class NetGameSession;

enum : uint16_t
{
	EConnectorServer = 100,
	ENetworkModule = 0,
	EUserModule = 1,
	EZoneModule = 2,
	EChatModule = 3,
	EDBAgentModule = 4,
};

enum : uint16_t
{
	MSG_C_LOGIN = 1000,
	MSG_S_LOGIN = 1001,
	MSG_C_ENTER_GAME = 1002,
	MSG_S_ENTER_GAME = 1003,
	MSG_C_CHAT = 1004,
	MSG_S_CHAT = 1005,
};

struct IServerModule
{
	const char* m_ConnectorIpAddr = "127.0.0.1";
	const int m_ConnectorPortNo = 10001;

	virtual int AddRef(void) { return -1; }
	virtual int ReleaseRef(void) { return -1; }

	virtual int OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer) { return -1; }
	virtual int OnLoad() { return -1; }
	virtual int OnStart() { return -1; }
	virtual int OnUnload() { return -1; }

	virtual int SetConnector() { return -1; }
	virtual int ConnectToConnector() { return -1; }

	template<typename T>
	bool ParsePkt(T& pkt, const NetMsg& msg)
	{
		return pkt.ParseFromArray(msg.GetBody(), msg.GetBodyLength());
	}

	virtual int SendMsg(uint16_t targetServer) { return -1; }
	virtual int HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session) { return -1; }
	virtual void CreateUserConnection(std::shared_ptr<NetGameSession> session) {}
	virtual void DispatchClientMsg(uint16_t targetServer, NetMsg m_Msg, const std::shared_ptr<NetGameSession>& session) {}
	virtual void DispatchMsgToServer(uint16_t targetServer, NetMsg msg, const std::shared_ptr<NetGameSession>& session) {}
};