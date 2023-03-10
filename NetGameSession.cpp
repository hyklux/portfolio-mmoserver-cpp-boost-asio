#include "pch.h"
#include "NetGameSession.h"
#include "NetHeader.h"
#include "NetworkServer.h"

void NetGameSession::RegisterReceive()
{
	auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), m_Msg.GetLength()), [this, self](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			if (m_Msg.DecodeHeader())
			{
				m_pNetworkServer->DispatchClientMsg(EUserServer, m_Msg);
			}

			RegisterReceive();
		}
		else
		{
			//khy todo : disconnect
		}
	});
}

void NetGameSession::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		std::cout << "Send to client success." << std::endl;
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