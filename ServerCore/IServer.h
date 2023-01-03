#pragma once

#include <string>
#include "IServerContainer.h"
#include "NetMsg.h"

enum : uint16_t
{
	ENetworkServer = 0,
	EUserServer = 1,
	EZoneServer = 2,
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

struct IServer
{
	const char* m_ConnectorIpAddr = "127.0.0.1";
	const int m_ConnectorPortNo = 10001;

	virtual int AddRef(void) { return -1; }
	virtual int ReleaseRef(void) { return -1; }

	virtual int OnCreate(IServerContainer* pServerContainer, IServer*& pServer) { return -1; }
	virtual int OnLoad() { return -1; }
	virtual int OnStart() { return -1; }
	virtual int OnUnload() { return -1; }

	virtual int SetConnector() { return -1; }
	virtual int ConnectToConnector() { return -1; }
	virtual int SendMsg(uint16_t targetServer) { return -1; }
	virtual int HandleMsg(NetMsg msg) { return -1; }
};