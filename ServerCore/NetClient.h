#pragma once

#include <memory>

#include <SDKDDKVer.h>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "NetMsg.h"

class NetMsg;

class NetClient : public std::enable_shared_from_this<NetClient>
{
private:
	boost::asio::io_service& m_io_service;
	boost::asio::ip::tcp::socket m_Socket;

	std::string m_WriteMessage;
	bool m_IsConnected;

	NetMsg m_Msg;

public:
	NetClient(boost::asio::io_service& io_service) : m_io_service(io_service), m_Socket(io_service), m_IsConnected(false)
	{}

	bool IsConnected() const { return m_IsConnected; }

	void Connect(boost::asio::ip::tcp::endpoint& endpoint);

	void Disconnect();

	void SendMsgToServer(std::string msgStr);

	void SendMsgToServer(NetMsg msg);

private:
	void RegisterSend(std::string msgStr);

	void RegisterSend(NetMsg msg);

	void RegisterReceive();

	void OnConnect(const boost::system::error_code& error);

	void OnSend(const boost::system::error_code& error, size_t bytes_transferred);

	void OnReceive(const boost::system::error_code& error, size_t bytes_transferred);
};
