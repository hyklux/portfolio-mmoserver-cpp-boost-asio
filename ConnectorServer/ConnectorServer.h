#pragma once

#include <iostream>
#include <boost/asio.hpp>

#include "IServerModule.h"
#include "IServerContainer.h"

class ConServerSession;

using namespace std;

extern "C" __declspec(dllexport) int CreateServerInstance(IServerContainer * pServerContainer, IServerModule * &pServer);

class ConnectorServer : public IServerModule
{
private:
	const char* m_IpAddr = "127.0.0.1";
	const int m_ListenPortNo = 10001;
	const int m_ThreadCnt = 1;

	std::atomic_int m_refs = 0;
	IServerContainer* m_pServerContainer;
	IServerModule* m_pConnectorServer;

	std::shared_ptr<boost::asio::io_service::work> m_Work;
	boost::asio::io_service m_IOService;
	boost::asio::ip::tcp::acceptor m_Acceptor;
	boost::asio::ip::tcp::socket m_Socket;
	std::vector<std::shared_ptr<boost::asio::ip::tcp::acceptor>> m_Acceptors;
	std::vector<boost::asio::ip::tcp::socket> m_AcceptorSockets;
	std::vector<std::unique_ptr<std::thread>> m_ThreadList;

	uint32_t m_SessionIdIdx = 0;
	std::vector<ConServerSession*> m_ServerSessions;
public:
	ConnectorServer() : m_Acceptor(m_IOService), m_Socket(m_IOService)
	{
	}

	~ConnectorServer()
	{

	}

	virtual int AddRef(void) override;
	virtual int ReleaseRef(void) override;

	virtual int OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer) override;
	virtual int OnLoad() override;
	virtual int OnStart() override;
	virtual int OnUnload() override;
	virtual void DispatchMsgToServer(uint16_t targetServer, NetMsg msg, const std::shared_ptr<NetGameSession>& session) override;

private:
	bool InitIOService();
	bool InitThreads();
	bool StartTcpServer();
	void RegisterAccept();
};

