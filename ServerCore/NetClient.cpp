#include "pch.h"
#include "NetClient.h"
#include "NetMsg.h"
#include "IServerModule.h"

#include "Protocol.pb.h"

void NetClient::Connect(boost::asio::ip::tcp::endpoint& endpoint)
{
	std::cout << "[NetClient] Connecting to server(" << endpoint.address() << "," << endpoint.port() << ")..." << std::endl;

	m_Socket.async_connect(endpoint, boost::bind(&NetClient::OnConnect, this, boost::asio::placeholders::error));
}

void NetClient::OnConnect(const boost::system::error_code& error)
{
	if (error)
	{
		std::cout << "[NetClient] Connection failed : " << error.message() << std::endl;
		m_Socket.close();
		m_IsConnected = false;
	}
	else
	{
		std::cout << "[NetClient] Connected to server." << std::endl;
		m_IsConnected = true;
		Start();
	}
}

void NetClient::Disconnect()
{
	std::cout << "[NetClient] Disconnect from server." << endl;

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

void NetClient::StartIOService()
{
	m_io_service.restart();
	m_io_service.run();
}

void NetClient::StopIOService()
{
	m_io_service.stop();
}

void NetClient::RegisterSend(std::string msgStr)
{
	std::cout << "RegisterSend msg:" << msgStr << endl;

	boost::asio::streambuf b;
	//b.prepare(128);
	b.prepare(msgStr.length());
	std::ostream os(&b);
	os << msgStr;
	//b.commit(128);
	b.commit(msgStr.length());

	boost::asio::async_write(m_Socket, b, boost::asio::transfer_exactly(msgStr.length()), [&](error_code error, std::size_t bytes_transferred)
	{
		std::cout << "[NetClient] All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});
}

void NetClient::RegisterSend(NetMsg msg)
{
	boost::asio::async_write(m_Socket, boost::asio::buffer(msg.GetData(), msg.GetLength()), [&](error_code error, std::size_t bytes_transferred)
	{
		std::cout << "[NetClient] All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});
}

void NetClient::RegisterReceiveHeader()
{
	//auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), NetMsg::HEADER_LENGTH), [this](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			if (m_Msg.DecodeHeader())
			{
				RegisterReceiveBody();
			}
			else
			{
				Disconnect();
			}
		}
		else
		{
			Disconnect();
		}
	});
}

void NetClient::RegisterReceiveBody()
{
	//auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetBody(), m_Msg.GetBodyLength()), [this](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			HandleMsg(m_Msg);
			RegisterReceiveHeader();
		}
		else
		{
			Disconnect();
		}
	});
}

void NetClient::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{
	std::cout << "[NetClient] Send success!" << std::endl;
}

void NetClient::OnReceive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cout << "[NetClient] Connection with server failed." << std::endl;
		}
		else
		{
			std::cout << "[NetClient] Error No: " << error.value() << " Error Message: " << error.message() << std::endl;
		}
	}
}

int NetClient::HandleMsg(const NetMsg msg)
{
	cout << "[NetClient] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_S_LOGIN:
		Handle_S_LOGIN(msg);
		break;
	case MSG_S_ENTER_GAME:
		Handle_S_ENTER_GAME(msg);
		break;
	case MSG_S_CHAT:
		Handle_S_CHAT(msg);
		break;
	default:
		break;
	}

	return 0;
}

uint16_t NetClient::Handle_S_LOGIN(const NetMsg msg)
{
	//패킷 분해
	Protocol::S_LOGIN pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	//로그인 결과에 따른 처리
	m_IsLoggedIn = pkt.success();
	if (m_IsLoggedIn)
	{
		cout << "[NetClient] Login success!" << endl;
	}
	else
	{
		cout << "[NetClient] Login failed." << endl;
	}

	return 0;
}

uint16_t NetClient::Handle_S_ENTER_GAME(const NetMsg msg)
{
	//패킷 분해
	Protocol::S_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	//게임 진입 결과에 따른 처리
	m_HasEnteredGame = pkt.success();
	if (m_HasEnteredGame)
	{
		cout << "[NetClient] Enter game success!" << endl;
	}
	else
	{
		cout << "[NetClient] Enter game failed." << endl;
	}

	return 0;
}

uint16_t NetClient::Handle_S_CHAT(const NetMsg msg)
{
	//패킷 분해
	Protocol::S_CHAT pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	//채팅 메세지 출력
	std::string chatMsg = pkt.msg();
	cout << "[NetClient] " << chatMsg << endl;

	return 0;
}
