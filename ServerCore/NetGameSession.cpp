#include "pch.h"
#include "NetGameSession.h"
#include "NetHeader.h"

void NetGameSession::SendMsgToClient(NetMsg msg)
{
	if (m_Socket.is_open() == false)
	{
		return;
	}

	RegisterSend(msg);
}

void NetGameSession::RegisterSend(NetMsg msg)
{
	boost::asio::async_write(m_Socket, boost::asio::buffer(msg.GetData(), msg.GetLength()), [&](error_code error, std::size_t bytes_transferred)
	{
		std::cout << "[NetGameSession] All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});
}

void NetGameSession::RegisterReceiveHeader()
{
	auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), NetMsg::HEADER_LENGTH), [this, self](boost::system::error_code error, std::size_t /*length*/)
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

void NetGameSession::RegisterReceiveBody()
{
	auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetBody(), m_Msg.GetBodyLength()), [this, self](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			m_pNetworkServer->DispatchClientMsg(EUserModule, m_Msg, self);
			RegisterReceiveHeader();
		}
		else
		{
			Disconnect();
		}
	});
}

void NetGameSession::Disconnect()
{
	std::cout << "[NetGameSession] Disconnect with client" << endl;

	if (m_Socket.is_open() == false)
	{
		return;
	}

	m_Socket.close();

	m_SessionId = 0;
	m_PlayerId = 0;
}