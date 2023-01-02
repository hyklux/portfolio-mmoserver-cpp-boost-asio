#include "pch.h"
#include "NetTcpServer.h"
#include "NetGameSession.h"

void NetTcpServer::RegisterAccept()
{
	std::cout << "Waiting for client connection..." << std::endl;

	//m_pSession = new NetGameSession(m_io_service);
	//m_acceptor.async_accept(m_pSession->Socket(), boost::bind(&NetTcpServer::OnAccept, this, m_pSession, boost::asio::placeholders::error));
}

void NetTcpServer::OnAccept(NetGameSession* pSession, const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Client connection success." << std::endl;
		std::cout << std::endl;
		pSession->RegisterReceive();
	}
}
