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
#include "Protocol.pb.h"

using namespace std;

class NetworkServer;

class NetGameSession : public std::enable_shared_from_this<NetGameSession>
{
private:
	NetworkServer* m_pNetworkServer;
	uint32_t m_SessionId;

	boost::asio::ip::tcp::socket m_Socket;
	boost::asio::streambuf streamBuf;
	NetMsg m_Msg;

public:
	NetGameSession(NetworkServer* pNetworkServer, uint32_t sessionId, boost::asio::ip::tcp::socket& _socket) : m_pNetworkServer(pNetworkServer), m_SessionId(sessionId), m_Socket(std::move(_socket))
	{
		cout << "Session created. SessionId:" << m_SessionId << endl;
	}

	~NetGameSession()
	{

	}

	boost::asio::ip::tcp::socket& Socket()
	{
		return m_Socket;
	}

	void RegisterSend();

	void RegisterReceive();

private:
	void OnSend(const boost::system::error_code& error, size_t bytes_transferred);

	void OnReceive(error_code error, size_t bytes_transferred);
};
