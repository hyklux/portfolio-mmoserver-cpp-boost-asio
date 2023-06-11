#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <memory>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "NetMsg.h"
#include "IServerModule.h"

using namespace std;

class NetMsg;

class NetGameSession : public std::enable_shared_from_this<NetGameSession>
{
private:
	IServerModule* m_pNetworkServer;
	uint32_t m_SessionId;
	uint64_t m_PlayerId;

	boost::asio::ip::tcp::socket m_Socket;
	boost::asio::streambuf streamBuf;
	NetMsg m_Msg;

public:
	NetGameSession(IServerModule* pNetworkServer, uint32_t sessionId, boost::asio::ip::tcp::socket& _socket) : m_pNetworkServer(pNetworkServer), m_SessionId(sessionId), m_PlayerId(0), m_Socket(std::move(_socket))
	{
		cout << "[NetGameSession] Session created. SessionId:" << m_SessionId << endl;
	}

	~NetGameSession()
	{

	}

	boost::asio::ip::tcp::socket& Socket()
	{
		return m_Socket;
	}

	uint32_t GetSessionId()
	{
		return m_SessionId;
	}

	uint64_t GetPlayerId()
	{
		return m_PlayerId;
	}

	void SetPlayerId(uint64_t playerId)
	{
		m_PlayerId = playerId;
	}

	void SendMsgToClient(NetMsg msg);

	void RegisterSend(NetMsg msg);

	void RegisterReceive();

	void Disconnect();
};
