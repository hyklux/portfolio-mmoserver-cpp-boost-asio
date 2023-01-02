#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

class ConServerSession;

class ConTcpServer
{
private:
	const unsigned short portNo = 7777;
	boost::asio::ip::tcp::acceptor m_acceptor;
	boost::asio::io_service& m_io_service;
	ConServerSession* m_pSession;

public:
	ConTcpServer(boost::asio::io_service& io_service)
		: m_io_service(io_service), m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), portNo)), m_pSession(nullptr)
	{
		RegisterAccept();
	}

	~ConTcpServer()
	{
		{
			delete m_pSession;
		}
	}

	void RegisterAccept();

private:

	void OnAccept(ConServerSession* pSession, const boost::system::error_code& error);
};

