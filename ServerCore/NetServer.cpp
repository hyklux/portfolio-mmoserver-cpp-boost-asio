#include "pch.h"
#include "NetServer.h"
#include "NetSession.h"

void NetServer::RegisterAccept()
{
	std::cout << "Waiting for client connection..." << std::endl;

	m_pSession = new NetSession(m_io_service);
	m_acceptor.async_accept(m_pSession->Socket(), boost::bind(&NetServer::OnAccept, this, m_pSession, boost::asio::placeholders::error));
}

void NetServer::OnAccept(NetSession* pSession, const boost::system::error_code& error)
{
	if (!error)
	{
		std::cout << "Client connection success." << std::endl;
		pSession->RegisterReceive();
	}
}