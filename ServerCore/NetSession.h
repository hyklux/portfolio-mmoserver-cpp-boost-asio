#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace std;

struct NetHeader;

class NetSession
{
private:
	boost::asio::ip::tcp::socket m_Socket;
	std::string m_WriteMessage;
	std::array<char, 128> m_ReceiveBuffer;
	boost::asio::streambuf stream_buffer;
	char m_Data[128];

public:
	NetSession(boost::asio::io_service& io_service)
		: m_Socket(io_service)
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

	void OnReceive(const boost::system::error_code& error, size_t bytes_transferred);
};