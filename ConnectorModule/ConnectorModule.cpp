#include "pch.h"
#include "ConnectorModule.h"
#include "ConServerSession.h"

int CreateServerModuleInstance(IServerContainer * pServerContainer, IServerModule * &pServer)
{
	cout << "[ConnectorModule] Creating connector module instance..." << endl;

	ConnectorModule* server = new ConnectorModule();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[ConnectorModule] Connector module instance created." << endl;

	return 0;
}

int ConnectorModule::AddRef(void)
{
	return ++m_refs;
}

int ConnectorModule::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int ConnectorModule::OnCreate(IServerContainer* pServerContainer, IServerModule*& pServer)
{
	cout << "[ConnectorModule] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServerModule*>(this);
	pServer->AddRef();

	return 0;
}

int ConnectorModule::OnLoad()
{
	cout << "[ConnectorModule] OnLoad" << endl;

	InitIOService();
	InitThreads();

	return 0;
}

int ConnectorModule::OnStart()
{
	cout << "[ConnectorModule] OnStart" << endl;

	StartTcpServer();

	return 0;
}

int ConnectorModule::OnUnload()
{
	cout << "[ConnectorModule] OnUnload" << endl;

	m_Work->get_io_context().restart();
	m_IOService.stop();

	return 0;
}

void ConnectorModule::DispatchMsgToServer(uint16_t targetServer, NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	IServerModule* pTargetServer = static_cast<IServerModule*>(m_pServerContainer->GetTargetModule(targetServer));

	if (pTargetServer)
	{
		pTargetServer->HandleMsg(msg, session);
	}
}

bool ConnectorModule::InitIOService()
{
	cout << "[ConnectorModule] InitIOService" << endl;

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

bool ConnectorModule::InitThreads()
{
	cout << "[ConnectorModule] InitThreads Cnt:" << m_ThreadCnt << endl;

	try
	{
		for (int nI = 0; nI < m_ThreadCnt; ++nI)
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

bool ConnectorModule::StartTcpServer()
{
	cout << "[ConnectorModule] StartTcpServer" << endl;

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

void ConnectorModule::RegisterAccept()
{
	m_Acceptor.async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
	{
		cout << "[ConnectorModule] OnAccept" << endl;

		if (!error)
		{
			std::cout << "Server connection success." << std::endl;

			ConServerSession* newSession = new ConServerSession(1, socket);
			newSession->RegisterReceive();
			m_ServerSessions.push_back(std::move(newSession));
		}

		RegisterAccept();
	});
}