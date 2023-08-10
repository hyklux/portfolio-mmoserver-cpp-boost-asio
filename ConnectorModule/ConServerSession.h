#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "IServerModule.h"
#include "Protocol.pb.h"

using namespace std;

class ConServerSession
{
private:
	boost::asio::ip::tcp::socket m_Socket;
	boost::asio::streambuf streamBuf;
	uint16_t m_ServerId;

public:
	ConServerSession(uint16_t serverId, boost::asio::ip::tcp::socket& _socket) : m_ServerId(serverId), m_Socket(std::move(_socket))
	{
		cout << "[ConServerSession] Session created. ServerId:" << m_ServerId << endl;
	}

	~ConServerSession()
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

