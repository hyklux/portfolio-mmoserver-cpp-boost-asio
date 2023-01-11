#include "pch.h"
#include "ConnectorServer.h"
#include "ConServerSession.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ConnectorServer] Creating connector server instance..." << endl;

	ConnectorServer* server = new ConnectorServer();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ConnectorServer] Connector server instance created." << endl;

	return 0;
}

int ConnectorServer::AddRef(void)
{
	return ++m_refs;
}

int ConnectorServer::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ConnectorServer::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[ConnectorServer] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int ConnectorServer::OnLoad()
{
	cout << "[ConnectorServer] OnLoad" << endl;

	InitIOService();
	InitThreads();

	return 0;
}

int ConnectorServer::OnStart()
{
	cout << "[ConnectorServer] OnStart" << endl;

	StartTcpServer();

	return 0;
}

int ConnectorServer::OnUnload()
{
	cout << "[ConnectorServer] OnUnload" << endl;

	m_Work->get_io_context().restart();
	m_IOService.stop();
	m_ThreadGroup.join();

	return 0;
}

void ConnectorServer::DispatchMsgToServer(uint16_t targetServer, NetMsg msg)
{
	IServer* pTargetServer = static_cast<IServer*>(m_pServerContainer->GetTargetServer(targetServer));

	if (pTargetServer)
	{
		pTargetServer->HandleMsg(msg, nullptr);
	}
}

bool ConnectorServer::InitIOService()
{
	cout << "[ConnectorServer] InitIOService" << endl;

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

bool ConnectorServer::InitThreads()
{
	cout << "[ConnectorServer] InitThreads Cnt:" << m_ThreadCnt << endl;

	try
	{
		for (int nI = 0; nI < m_ThreadCnt; ++nI)
		{
			/*
			m_ThreadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &m_IOService));
			auto thr = std::make_unique<std::thread>([this]()
			{
				m_IOService.run();
			});
			m_ThreadList.push_back(std::move(thr));
			*/
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

bool ConnectorServer::StartTcpServer()
{
	cout << "[ConnectorServer] StartTcpServer" << endl;

	try
	{
		boost::asio::ip::tcp::endpoint endpoint_;

		endpoint_.address(boost::asio::ip::address::from_string(m_IpAddr));
		endpoint_.port(m_ListenPortNo);

		m_Acceptor.open(endpoint_.protocol());

		boost::asio::socket_base::reuse_address option(true);
		m_Acceptor.set_option(option);
		m_Acceptor.bind(endpoint_);
		m_Acceptor.listen();

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

void ConnectorServer::RegisterAccept()
{
	m_Acceptor.async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
	{
		cout << "[ConnectorServer] OnAccept" << endl;

		if (!error)
		{
			std::cout << "Server connection success." << std::endl;

			ConServerSession* newSession = new ConServerSession(1 , socket);
			newSession->RegisterReceive();
			m_ServerSessions.push_back(std::move(newSession));
		}

		RegisterAccept();
	});
}
