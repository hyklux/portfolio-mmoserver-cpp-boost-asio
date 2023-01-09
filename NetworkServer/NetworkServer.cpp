#include "pch.h"
#include "NetworkServer.h"
#include "NetTcpServer.h"


int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[NetworkServer] Creating network server instance..." << endl;

	NetworkServer* server = new NetworkServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[NetworkServer] Network server instance created." << endl;

	return 0;
}

int NetworkServer::AddRef(void)
{
	return ++m_refs;
}

int NetworkServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int NetworkServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[NetworkServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int NetworkServer::OnLoad()
{
	cout << "[NetworkServer] OnLoad" << endl;

	InitIOService();
	InitThreads();

	return 0;
}

int NetworkServer::OnStart()
{
	cout << "[NetworkServer] OnStart" << endl;

	StartTcpServer();

	return 0;
}

int NetworkServer::OnUnload()
{
	cout << "[NetworkServer] OnUnload" << endl;

	// 작업이 없으면 io_service가 종료하도록
	m_Work->get_io_context().restart();

	// 쓰레드가 종료하기를 기다린다
	for (const auto& itr : m_ThreadList)
	{
		if (itr->joinable())
		{
			itr->join();
		}
	}

	// io_service 완전 종료
	m_IOService.stop();

	return 0;
}

int NetworkServer::SetConnector()
{
	if (!m_pServerContainer)
	{
		return -1;
	}

	void* pContainerPtr = m_pServerContainer->GetConnectorServer();
	if (pContainerPtr)
	{
		m_pConnectorServer = static_cast<IServer*>(pContainerPtr);
	}

	return m_pConnectorServer ? 0 : -1;
}

bool NetworkServer::InitIOService()
{
	cout << "[NetworkServer] InitIOService" << endl;

	try
	{
		m_Work = std::make_unique<boost::asio::io_service::work>(m_IOService);
		m_Work->get_io_context().restart();
	}
	catch (std::bad_weak_ptr& e)
	{
		return false;
	}
	catch (std::exception& e)
	{
		return false;
	}

	return true;
}

bool NetworkServer::InitThreads()
{
	cout << "[NetworkServer] InitThreads Cnt:" << m_ThreadCnt << endl;

	try
	{
		for (int i = 0; i < m_ThreadCnt; ++i)
		{
			auto thr = std::make_unique<std::thread>([this]()
			{
				m_IOService.run();
			});
			m_ThreadList.push_back(std::move(thr));
		}
	}
	catch (std::bad_weak_ptr& e)
	{
		return false;
	}
	catch (std::exception& e)
	{
		return false;
	}

	return true;
}

bool NetworkServer::StartTcpServer()
{
	cout << "[NetworkServer] StartTcpServer" << endl;

	try
	{
		boost::asio::ip::tcp::endpoint endpoint_;

		endpoint_.address(boost::asio::ip::address::from_string(m_IpAddr));
		endpoint_.port(m_PortNo);

		//m_Acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(m_IOService);
		m_Acceptor.open(endpoint_.protocol());

		boost::asio::socket_base::reuse_address option(true);
		m_Acceptor.set_option(option);
		m_Acceptor.bind(endpoint_);
		m_Acceptor.listen();

		//m_Acceptors.push_back(std::move(m_Acceptor));

		//boost::asio::ip::tcp::socket acceptorSocket(m_IOService);
		//m_AcceptorSockets.push_back(acceptorSocket);

		RegisterAccept();
	}
	catch (std::bad_weak_ptr& e)
	{
		return false;
	}
	catch (std::exception& e)
	{
		return false;
	}

	return true;
}

void NetworkServer::RegisterAccept()
{
	m_Acceptor.async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
	{
		cout << "[NetworkServer] OnAccept" << endl;

		if (!error)
		{
			std::cout << "Client connection success." << std::endl;

			std::shared_ptr<NetGameSession> newSession = std::make_shared<NetGameSession>(this, ++m_SessionIdIdx, socket);
			newSession->RegisterReceive();
			//m_GameSessions.push_back(std::move(newSession));
			//m_GameSessions.push_back(new NetGameSession(++m_SessionIdIdx, socket));

			IServer* userServer = GetUserServer();
			if (userServer)
			{
				userServer->CreateUserConnection(newSession);
			}
		}

		RegisterAccept();
	});
}

void NetworkServer::OnAccept(const boost::system::error_code& error, const boost::asio::ip::tcp::socket& socket)
{
}

IServer* NetworkServer::GetUserServer()
{
	void* pContainerPtr = m_pServerContainer->GetUserServer();
	if (pContainerPtr)
	{
		return static_cast<IServer*>(pContainerPtr);
	}

	return nullptr;
}

void NetworkServer::DispatchClientMsg(uint16_t targetServer, NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[NetworkServer] DispatchClientMsg" << endl;

	IServer* pTargetServer = static_cast<IServer*>(m_pServerContainer->GetTargetServer(targetServer));

	if (pTargetServer)
	{
		pTargetServer->HandleMsg(msg, session);
	}
}

