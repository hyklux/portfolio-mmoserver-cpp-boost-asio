#include "pch.h"
#include "NetClient.h"
#include "NetMsg.h"

void NetClient::Connect(boost::asio::ip::tcp::endpoint& endpoint)
{
	std::cout << "Connecting to server(" << endpoint.address() << "," << endpoint.port() << ")..." << std::endl;

	m_Socket.async_connect(endpoint, boost::bind(&NetClient::OnConnect, this, boost::asio::placeholders::error));

	m_io_service.restart();
	m_io_service.run();
}

void NetClient::OnConnect(const boost::system::error_code& error)
{
	if (error)
	{
		std::cout << "Connection failed : " << error.message() << std::endl;
		m_Socket.close();
		m_IsConnected = false;
	}
	else
	{
		std::cout << "Connected to server." << std::endl;
		m_IsConnected = true;
	}

	m_io_service.stop();
}

void NetClient::Disconnect()
{
	std::cout << "Disconnect from server." << endl;

	if (m_Socket.is_open() == false)
	{
		return;
	}

	m_Socket.close();
	m_IsConnected = false;
}

void NetClient::SendMsgToServer(std::string msgStr)
{
	if (m_IsConnected == false)
	{
		return;
	}

	if (m_Socket.is_open() == false)
	{
		return;
	}

	RegisterSend(msgStr);
}

void NetClient::SendMsgToServer(NetMsg msg)
{
	if (m_IsConnected == false)
	{
		return;
	}

	if (m_Socket.is_open() == false)
	{
		return;
	}

	RegisterSend(msg);
}

void NetClient::RegisterSend(std::string msgStr)
{
	std::cout << "RegisterSend msg:" << msgStr << endl;

	boost::asio::streambuf b;
	b.prepare(128);
	std::ostream os(&b);
	os << msgStr;
	b.commit(128);

	boost::asio::async_write(m_Socket, b, boost::asio::transfer_exactly(128), [&](error_code error, std::size_t bytes_transferred)
	{
		// All data has been sent
		std::cout << "All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});

	RegisterReceive();
}

void NetClient::RegisterSend(NetMsg msg)
{
	boost::asio::async_write(m_Socket, boost::asio::buffer(msg.GetData(), msg.GetLength()), [&](error_code error, std::size_t bytes_transferred)
	{
		// All data has been sent
		std::cout << "All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});

	m_io_service.restart();
	m_io_service.run();
}

void NetClient::RegisterReceive()
{
	//auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), m_Msg.GetLength()), [this](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			//khy todo : Handle 처리
		}
		else
		{
			//khy todo : disconnect
		}
	});
}

void NetClient::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{
	std::cout << "Send success!" << std::endl;
}

void NetClient::OnReceive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cout << "Connection with server failed." << std::endl;
		}
		else
		{
			std::cout << "Error No: " << error.value() << " Error Message: " << error.message() << std::endl;
		}
	}
	else
	{
	}
}
