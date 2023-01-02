#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include "IServer.h"
#include "IServerContainer.h"
#include "NetGameSession.h"

using namespace std;

extern "C" __declspec(dllexport) int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer);

class NetworkServer : public IServer
{
private:
	const char* m_IpAddr = "127.0.0.1";
	const int m_PortNo = 10000;
	const int m_ThreadCnt = 5;

	std::atomic_int m_refs = 0;
	IServerContainer* m_pServerContainer;
	IServer* m_pConnectorServer;

	std::shared_ptr<boost::asio::io_service::work> m_Work;
	boost::asio::io_service m_IOService;
	boost::asio::ip::tcp::acceptor m_Acceptor;
	boost::asio::ip::tcp::socket m_Socket;
	std::vector<std::shared_ptr<boost::asio::ip::tcp::acceptor>> m_Acceptors;
	std::vector<boost::asio::ip::tcp::socket> m_AcceptorSockets;
	std::vector<std::unique_ptr<std::thread>> m_ThreadList;

	uint32_t m_SessionIdIdx = 0;
	std::vector<std::shared_ptr<NetGameSession>> m_GameSessions;
public:
	NetworkServer() : m_Acceptor(m_IOService), m_Socket(m_IOService)
	{
	}

	~NetworkServer()
	{

	}

	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;

	virtual int OnCreate(IServerContainer* pServerContainer, IServer*& pServer) override;
	virtual int OnLoad() override;
	virtual int OnStart() override;
	virtual int OnUnload() override;

	void DispatchClientMsg(uint16_t targetServer, NetMsg msg);

private:
	virtual int SetConnector() override;

	bool InitIOService();
	bool InitThreads();
	bool StartTcpServer();
	void RegisterAccept();
	void OnAccept(const boost::system::error_code& error, const boost::asio::ip::tcp::socket& socket);
};
