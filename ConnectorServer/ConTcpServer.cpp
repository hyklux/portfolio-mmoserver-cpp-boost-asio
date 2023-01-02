#include "pch.h"
#include "ConTcpServer.h"
#include "ConServerSession.h"

void ConTcpServer::RegisterAccept()
{
	std::cout << "Waiting for other server connection..." << std::endl;

	//m_pSession = new ConServerSession();
	//m_acceptor.async_accept(m_pSession->Socket(), boost::bind(&ConTcpServer::OnAccept, this, m_pSession, boost::asio::placeholders::error));
}

void ConTcpServer::OnAccept(ConServerSession* pSession, const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Client connection success." << std::endl;
		std::cout << std::endl;
		pSession->RegisterReceive();
	}
}