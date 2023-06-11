# portfolio-mmoserver-cpp-boost-asio
MMO server framework using C++ Boost Asio library


# Introduction
This is a MMO server framework using C++ Boost Asio library.


# Implmentation
:heavy_check_mark: Architecture


:heavy_check_mark: Server Container


:heavy_check_mark: Network module


:heavy_check_mark: User module


:heavy_check_mark: DBAgent module


:heavy_check_mark: Zone module


:heavy_check_mark: Chat module


# Architecture
![mmo_server_architecture2](https://github.com/hyklux/portfolio-mmoserver-cpp-boost-asio/assets/96270683/a1d02e0c-3c42-43b9-971e-7b873b141d86)
- The server consists of a number of sub-modules, each module defined to do certain tasks.
- In an actual live service, even the same module(say User module) can exist more than one according to the number of users that can be accommodated for each one.
- Each module communicates with other modules through Connector module.


# Server Container
- When the server starts, it loads the modules defined in the config.json file, such as user, zone, chat and etc.
- The modules to be loaded may depend on the configuration of your server.
- For example, some servers may not have a zone module or may not have a DBAgent module.
``` json
    {
      "name": "ZoneModule",
      "use": "true",
      "servicetype": "3",
      "executable": "../.././DLLs/ZoneModule.dll",
      "config": "../.././Config/ZoneModule.json",
      "log": {
        "loglevel": "0",
        "console": "false"
      }
    },
    {
      "name": "ChatModule",
      "use": "true",
      "servicetype": "4",
      "executable": "../.././DLLs/ChatModule.dll",
      "config": "../.././Config/ChatModule.json",
      "log": {
        "loglevel": "0",
        "console": "false"
      }
    },
    {
      "name": "UserModule",
      "use": "true",
      "servicetype": "2",
      "executable": "../.././DLLs/UserModule.dll",
      "config": "../.././Config/UserModule.json",
      "log": {
        "loglevel": "0",
        "console": "false"
      }
    }
```
- Each module is made of an individual DLL file, so the DLL file is loaded when the module is loaded.
``` c++
class ServerContainer
{
	//...(omitted)
 
	auto servers = props.get_child("servers");
	for (const auto& server : servers)
	{
		auto name = server.second.get<std::string>("name");
		auto serverPair = m_ServerMap.emplace(name, new IServerModule());

		auto executable = server.second.get<std::string>("executable");
		HMODULE handle = LoadLibrary(executable.c_str());
		if (nullptr == handle)
		{
			cout << "DLL load error. GetLastError :" << GetLastError() << std::endl;
			continue;
		}

		typedef int(*CREATE_SERVER_FUNC)(IServerContainer* pServerContainer, IServerModule*& pServer);
		CREATE_SERVER_FUNC funcPtr = (CREATE_SERVER_FUNC)::GetProcAddress(handle, "CreateServerModuleInstance");
		int ret = (*funcPtr)((IServerContainer*)this, serverPair.first->second);
		if (ret != 0)
		{
			cout << "Server create error." << endl;
			FreeLibrary(handle);
			continue;
		}
	}
  
	//...(omitted)
}
```
 ![서버 모듈 로드](https://user-images.githubusercontent.com/96270683/221408977-60f10220-00cd-4dc3-a9bf-61efd9b04be6.PNG)
![mmo_portfolio_server1](https://github.com/hyklux/portfolio-mmoserver-cpp-boost-asio/assets/96270683/9a037ff0-f536-4bc8-aa63-1673c4f60a22)
- ServerContainer is the container that manages all modules uploaded to that server and holds references to all server modules.


# Network module
- Network module is a module responsible for communicating with clients.
- Network communication was implemented using the Boost Asio network library.
- When a client connects, it creates a NetGameSession instance.
``` c++
bool NetworkModule::StartTcpServer()
{
	cout << "[NetworkModule] StartTcpServer" << endl;

	try
	{
		//Set endpoint
		boost::asio::ip::tcp::endpoint endpoint_;
		endpoint_.address(boost::asio::ip::address::from_string(m_IpAddr));
		endpoint_.port(m_PortNo);

		//Starts listening for connection request
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

void NetworkModule::RegisterAccept()
{
	m_Acceptor.async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
	{
		cout << "[NetworkModule] OnAccept" << endl;

		//Create game session class once connected with client socket
		if (!error)
		{
			std::cout << "Client connection success." << std::endl;

			std::shared_ptr<NetGameSession> newSession = std::make_shared<NetGameSession>(this, ++m_SessionIdIdx, socket);
			newSession->RegisterReceive();
		}

		RegisterAccept();
	});
}
```
![서버 세션 생성](https://user-images.githubusercontent.com/96270683/221409174-abcb4489-1e0e-43a7-93e6-bfb788c4846b.PNG)
![mmo_portfolio_server2](https://github.com/hyklux/portfolio-mmoserver-cpp-boost-asio/assets/96270683/eb371985-e9ab-429a-b3e1-f355c8d08fbc)
- Each user performs packet communication through the session instance of the server connected to his/her client.
``` c++
//Send message to client
void NetGameSession::RegisterSend(NetMsg msg)
{
	boost::asio::async_write(m_Socket, boost::asio::buffer(msg.GetData(), msg.GetLength()), [&](error_code error, std::size_t bytes_transferred)
	{
		std::cout << "[NetGameSession] All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});
}

//Receive message from client
void NetGameSession::RegisterReceive()
{
	auto self(shared_from_this());

	boost::asio::async_read(m_Socket, boost::asio::buffer(m_Msg.GetData(), m_Msg.GetLength()), [this, self](boost::system::error_code error, std::size_t /*length*/)
	{
		if (!error)
		{
			if (m_Msg.DecodeHeader())
			{
				m_pNetworkModule->DispatchClientMsg(EUserServer, m_Msg, self);
			}

			RegisterReceive();
		}
		else
		{
			Disconnect();
		}
	});
}
```

# User module
- The User module is the starting point for executing requests from clients.
- After analyzing the request to be performed by disassembling the packet from the client, it is put in the JobQueue of the User module in the form of a Job instance.
``` c++
//Put received messages in JobQueue.
int UserModule::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserModule] HandleMsg. PktId:" << msg.GetPktId() << endl;

	switch (msg.GetPktId())
	{
	case MSG_C_LOGIN:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_LOGIN(msg, session);
		});
		break;
	case MSG_C_ENTER_GAME:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_ENTER_GAME(msg, session);
		});
		break;
	case MSG_C_CHAT:
		m_ThreadPool.EnqueueJob([this, msg, session]() {
			this->Handle_C_CHAT(msg, session);
		});
		break;
	default:
		break;
	}

	return 0;
}
```
- In the WorkerThread of the User module, Jobs accumulated in the JobQueue are executed one by one by each thread.
``` c++
void ThreadPool::WorkerThread() 
{
	while (true) 
	{
		std::unique_lock<std::mutex> lock(m_JobQueueMutex);
		m_JobQueueCV.wait(lock, [this]() 
		{ 
			return !this->m_JobQueue.empty() || m_StopAll; 
		});
		
		if (m_StopAll && this->m_JobQueue.empty()) 
		{
			return;
		}

		std::function<void()> job = std::move(m_JobQueue.front());
		m_JobQueue.pop();
		lock.unlock();

		cout << "[ThreadPool] Pushing job out of queue..." << endl;

		job();
	}
}
```
- The User module is also responsible for the main data of users.
- User module holds the current data of each user connected to the server.
- When user data is changed in another module, the user data in the User module must be updated by sending a packet to the User module.

# DBAgent module
- The DBAgent module handles requests to the DB.
- All DB requests are made through the DBAgent module.
- When the module is loaded, it establishes a connection with the DB.
``` c++
int DBAgentModule::ConnectToDB()
{
	cout << "[DBAgentModule] Connecting to DB..." << endl;

	bool connResult = m_DbConn.Connect(L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=UserDB;Trusted_Connection=Yes;");
	if (!connResult)
	{
		cout << "[DBAgentModule] DB connection error." << endl;
		return -1;
	}

	cout << "[DBAgentModule] DB connection success." << endl;
	return 0;
}

bool DBConn::Connect(const WCHAR* connectionString)
{
	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_environment) != SQL_SUCCESS)
	{
		return false;
	}

	if (::SQLSetEnvAttr(_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
	{
		return false;
	}

	if (::SQLAllocHandle(SQL_HANDLE_DBC, _environment, &_connection) != SQL_SUCCESS)
	{
		return false;
	}

	WCHAR stringBuffer[MAX_PATH] = { 0 };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0 };
	SQLSMALLINT resultStringLen = 0;

	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT & resultStringLen,
		SQL_DRIVER_NOPROMPT
	);

	if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS)
	{
		return false;
	}

	return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}
```
- When a request for DB processing is received, the query is executed by linking values to the query as parameters.
- When requesting a login, if it is a new user, new user data is created in the DB.
``` c++
int DBAgentModule::CreateUserToDB(std::string userName)
{
	m_DbConn.Unbind();

	SQLLEN len = 0;

	m_UserId++;
	int userIdParam = m_UserId;
	std::wstring usernameWStr = std::wstring(userName.begin(), userName.end());
	const WCHAR* userNameParam = usernameWStr.c_str();

	m_DbConn.BindParam(1, &userIdParam, &len);
	m_DbConn.BindParam(2, userNameParam, &len);

	auto query = L"INSERT INTO [dbo].[User]([userid], [username]) VALUES(?, ?)";
	bool queryResult = m_DbConn.Execute(query);
	if (!queryResult)
	{
		cout << "[DBAgent] User creation failed." << endl;
		return -1;
	}

	cout << "[DBAgent] User creation success." << endl;
	return 0;
}

bool DBConn::Execute(const WCHAR* query)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		return true;

	HandleError(ret);
	return false;
}
```
![서버 DB에 유저 생성](https://user-images.githubusercontent.com/96270683/221409525-69297530-1fa6-49e7-aa19-4f8ddeb63eca.PNG)
![mmo_portfolio_server3](https://github.com/hyklux/portfolio-mmoserver-cpp-boost-asio/assets/96270683/f088ad35-fd5e-4377-9d7d-c6adb881e7b6)


# Zone module
- Zone module is where the actual game world (map or level) exists, in which the player interacts with other players or NPCs.
- When the Zone module is loaded, an NPC is created, and a player instance is created when the user enters the game.
``` c++
void ZoneModule::CreateNPCs()
{
	cout << "[ZoneModule] CreateNPCs" << endl;

	//Create NPC object
	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<CMonster> monster(new CMonster(1000 + i, "Monster" + i));
		monster->SetPosition(i, 0);
		m_MonsterList.push_back(monster);
	}
}

int ZoneModule::Handle_C_ENTER_GAME(NetMsg msg)
{
	cout << "[ZoneModule] Handle_C_ENTER_GAME" << endl;

	//Disassemble packet
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	std::string playerName = "Player" + to_string(pkt.playerid());

	cout << "[ZoneModule] " << playerName << " entering game..." << endl;

	std::shared_ptr<CPlayer> player(new CPlayer(pkt.playerid(), playerName));
	m_PlayerList.push_back(player);

	cout << "[ZoneModule] " << playerName << " enter game success." << endl;

	return 0;
}
```
- The Zone module has a Tick function.
- The Tick function is a function that is repeatedly executed at the promised cycle and continuously updates the state of the world, players, NPCs and etc.
``` c++
void ZoneModule::RunTick()
{
	m_CanTick = true;

	while (m_CanTick)
	{
		// Calculate the time since the last tick (deltaTime)
		std::clock_t currentTime = std::clock();
		float deltaTime = ((float)(currentTime - lastTickTime)) / CLOCKS_PER_SEC;

		// Check if deltaTime is greater than base frame time
		if (deltaTime >= (1.0f / TICK_RATE)) 
		{
			Tick(deltaTime);

			lastTickTime = currentTime;
		}
	}
}

//Do what needs to be done every frame
void ZoneModule::Tick(float deltaTime)
{
	for (auto player : m_PlayerList)
	{
		player->Update(deltaTime);
	}

	for (auto monster : m_MonsterList)
	{
		monster->Update(deltaTime);
	}
}
```


# Chat module
- This module handles chat messages.
- When a user enters the game, it is added to the user session list in the chat module.
``` c++
int ChatModule::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatModule] Handle_C_ENTER_GAME" << endl;

	//Disassemble packet
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatModule] Player" << to_string(pkt.playerid()) << " has entered chat room." << endl;

	m_UserSessionList.push_back(session);

	return 0;
}
```
- When a specific user sends a chat message, it is broadcasted to other users.
``` c++
int ChatModule::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatModule] Handle_C_CHAT" << endl;

	//Disassemble packet
	Protocol::C_CHAT pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatModule] [Player" << to_string(pkt.playerid()+1) << "] " << pkt.msg() << endl;

	std::string broadcastMsgStr = "[Player" + to_string(pkt.playerid()+1) + "] : " + pkt.msg();
	BroadCastAll(broadcastMsgStr);

	return 0;
}

void ChatModule::BroadCastAll(std::string broadcastMsgStr)
{
	NetMsg broadcastMsg;
	Protocol::S_CHAT chatPkt;
	chatPkt.set_msg(broadcastMsgStr);
	broadcastMsg.MakeBuffer(chatPkt, MSG_S_CHAT);

	for (std::shared_ptr<NetGameSession> session : m_UserSessionList)
	{
		session->SendMsgToClient(broadcastMsg);
	}
}
```
![서버 채팅 메세지 처리](https://user-images.githubusercontent.com/96270683/221409793-0d137a61-d9f1-49e1-9881-45eb31387669.PNG)
