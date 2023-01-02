#include "pch.h"
#include "ConServerSession.h"

void ConServerSession::RegisterSend()
{

}

void ConServerSession::RegisterReceive()
{
	boost::asio::async_read(m_Socket, streamBuf, boost::asio::transfer_exactly(128), [&](error_code error, std::size_t bytes_transferred)
	{
		OnReceive(error, bytes_transferred);
	});
}

void ConServerSession::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{

}

void ConServerSession::OnReceive(error_code error, size_t bytes_transferred)
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
			cout << "Session " << static_cast<int>(m_ServerId) << " received msg. Size :" << bytes_transferred << endl;

			/*
			Protocol::C_LOGIN loginPkt;
			int ret = loginPkt.ParseFromString(msgStr);
			if (ret == 0)
			{
				cout << "Msg content : " << loginPkt.username() << endl;
			}
			*/

			// Consume it after
			streamBuf.consume(bytes_transferred);
		}

		cout << "=======================================" << endl;

		RegisterReceive();
	}
}
