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

void NetGameSession::RegisterReceive()
{
	auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), m_Msg.GetLength()), [this, self](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			if (m_Msg.DecodeHeader())
			{
				m_pNetworkServer->DispatchClientMsg(EUserServer, m_Msg, self);
			}

			RegisterReceive();
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

void NetGameSession::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		std::cout << "[NetGameSession] Send to client success." << std::endl;
	}
}

void NetGameSession::OnReceive(error_code error, size_t bytes_transferred)
{
	cout << "=======================================" << endl;

	if (error)
	{
		cout << error.message() << endl;
	}
	else
	{
		// Read the data received
		boost::asio::streambuf::const_buffers_type bufs = streamBuf.data();

		// Convert to string
		std::string msgStr(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + bytes_transferred);

		if (bytes_transferred > 0)
		{
			cout << "Session " << m_SessionId << " received msg. Size :" << bytes_transferred << endl;

			//m_pNetworkServer->DispatchClientMsg();

			// Consume it after
			streamBuf.consume(bytes_transferred);
		}

		cout << "=======================================" << endl;

		RegisterReceive();
	}
}