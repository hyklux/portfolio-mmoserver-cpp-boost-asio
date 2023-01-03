#include "pch.h"
#include "NetSession.h"
#include "NetHeader.h"

void NetSession::RegisterSend()
{
	/*
	char szMessage[128] = { 0, };
	sprintf_s(szMessage, 128 - 1, "Re:%s", strRecvMessage.c_str());
	m_WriteMessage = szMessage;
	*/

	boost::asio::async_write(m_Socket, boost::asio::buffer(m_Data), boost::bind(&NetSession::OnSend, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));

	RegisterReceive();
}

void NetSession::RegisterReceive()
{
	/*memset(&m_ReceiveBuffer, '\0', sizeof(m_ReceiveBuffer));
	m_Socket.async_read_some(boost::asio::buffer(m_ReceiveBuffer), boost::bind(&NetSession::OnReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));*/

	//memset(m_Data, 0, 128);
	//boost::asio::async_read(m_Socket, boost::asio::buffer(m_Data), boost::bind(&NetSession::OnReceive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void NetSession::OnSend(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (!error)
	{
		std::cout << "Send to client success." << std::endl;
	}
}

void NetSession::OnReceive(const boost::system::error_code& error, size_t bytes_transferred)
{
	if (error)
	{
		if (error == boost::asio::error::eof)
		{
			std::cout << "Disconnect with client." << std::endl;
		}
		else
		{
			std::cout << "Error No: " << error.value() << " Error Message: " << error.message() << std::endl;
		}
	}
	else
	{
		/*
		NetHeader* pktHeader = &m_Data[0];
		const uint16 pktSize = pktHeader->size;
		const uint16 id = pktHeader->id;

		if (id == PKT_C_CHAT)
		{
			Protocol::C_CHAT chatPkt;
			int ret = chatPkt.ParseFromArray(m_Data + sizeof(NetHeader), pktSize - sizeof(NetHeader));
			if (!ret)
			{
				cout << "Parse from packet failed." << endl;
			}

			cout << chatPkt.msg() << endl;
		}
		*/

		//RegisterSend();
	}
}