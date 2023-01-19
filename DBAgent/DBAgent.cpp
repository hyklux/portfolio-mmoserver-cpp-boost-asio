#include "pch.h"
#include "DBAgent.h"
#include "IServer.h"
#include "Types.h"
#include "DBConn.h"

#include "Protocol.pb.h"

int CreateServerInstance(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[DBAgent] Creating zone server instance..." << endl;

	DBAgent* server = new DBAgent();
	if (nullptr == server)
	{
		return -1;
	}

	server->OnCreate(pServerContainer, pServer);

	cout << "[DBAgent] Zone server instance created." << endl;

	return 0;
}

int DBAgent::AddRef(void)
{
	return ++m_refs;
}

int DBAgent::ReleaseRef(void)
{
	if (--m_refs == 0)
	{
		delete this;
	}

	return m_refs;
}

int DBAgent::OnCreate(IServerContainer* pServerContainer, IServer*& pServer)
{
	cout << "[DBAgent] OnCreate" << endl;

	if (pServerContainer == nullptr)
	{
		return -1;
	}

	m_pServerContainer = pServerContainer;

	pServer = static_cast<IServer*>(this);
	pServer->AddRef();

	return 0;
}

int DBAgent::OnLoad()
{
	cout << "[DBAgent] OnLoad" << endl;

	//Connect
	if (0 != ConnectToDB())
	{
		return -1;
	}

	//Create table
	if (0 != InitDBTable())
	{
		return -1;
	}

	return 0;
}

int DBAgent::OnStart()
{
	cout << "[DBAgent] OnStart" << endl;

	return 0;
}

int DBAgent::OnUnload()
{
	cout << "[DBAgent] OnUnload" << endl;

	return 0;
}

int DBAgent::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[DBAgent] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_LOGIN:
		Handle_C_LOGIN(msg, session);
		break;
	default:
		break;
	}

	return 0;
}

int DBAgent::Handle_C_LOGIN(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[DBAgent] Handle_C_LOGIN" << endl;

	//패킷 분해
	Protocol::C_LOGIN pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[UserServer] " << pkt.username() << " logging in..." << endl;

	if (!ExistsUserInDB(pkt.username()))
	{
		CreateUserToDB(pkt.username());
	}

	cout << "[UserServer] " << pkt.username() << " login success." << endl;

	return static_cast<uint16_t>(ERRORTYPE::NONE_ERROR);
}

bool DBAgent::ExistsUserInDB(std::string userName)
{
	m_DbConn.Unbind();

	SQLLEN len = 0;
	m_DbConn.BindParam(1, SQL_C_WCHAR, SQL_WCHAR, sizeof(userName), &userName, &len);

	auto query = L"SELECT name FROM [dbo].[User] WHERE name = (?)";

	int outId = 0;
	SQLLEN outIdLen = 0;
	m_DbConn.BindCol(1, SQL_C_LONG, sizeof(outId), &outId, &outIdLen);

	bool queryResult = m_DbConn.Execute(query);
	if (!queryResult)
	{
		cout << "[DBAgent] User search failed." << endl;
		return false;
	}

	cout << "[DBAgent] User search success." << endl;

	m_DbConn.Fetch();
	return (outId != 0);
}

int DBAgent::CreateUserToDB(std::string userName)
{
	m_DbConn.Unbind();

	SQLLEN len = 0;

	m_UserIdx++;
	int queryUserId = m_UserIdx;
	std::wstring usernameWStr = std::wstring(userName.begin(), userName.end());
	const WCHAR* queryUserName = usernameWStr.c_str();

	m_DbConn.BindParam(1, &queryUserId, &len);
	m_DbConn.BindParam(2, queryUserName, &len);
	auto query = L"SET IDENTITY_INSERT [dbo].[User] ON INSERT INTO [dbo].[User]([id], [name]) VALUES(?, ?) SET IDENTITY_INSERT [dbo].[User] OFF";

	bool queryResult = m_DbConn.Execute(query);
	if (!queryResult)
	{
		cout << "[DBAgent] User creation failed." << endl;
		return -1;
	}

	cout << "[DBAgent] User creation success." << endl;
	return 0;
}

int DBAgent::SetConnector()
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

int DBAgent::ConnectToDB()
{
	cout << "[DBAgent] Connecting to DB..." << endl;

	bool connResult = m_DbConn.Connect(L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=UserDB;Trusted_Connection=Yes;");
	if (!connResult)
	{
		cout << "[DBAgent] DB connection error." << endl;
		return -1;
	}

	cout << "[DBAgent] DB connection success." << endl;
	return 0;
}

int DBAgent::InitDBTable()
{
	cout << "[DBAgent] Initializing DB table..." << endl;

	auto query = L"DROP TABLE IF EXISTS [dbo].[User]; CREATE TABLE [dbo].[User] ([id] INT NOT NULL PRIMARY KEY IDENTITY, [name] VARCHAR(100) NULL);";

	bool createTableResult = m_DbConn.Execute(query);
	if (!createTableResult)
	{
		cout << "[DBAgent] DB create table failed." << endl;
		return -1;
	}

	cout << "[DBAgent] DB create table success." << endl;
	return 0;
}