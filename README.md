# portfolio-mmoserver-cpp-boost-asio
MMO server framework using C++ Boost Asio library


# Introduction
This is a MMO server framework using C++ Boost Asio library.


# Implemntation
:heavy_check_mark: Architecture


:heavy_check_mark: Server Container


:heavy_check_mark: Network module


:heavy_check_mark: User module


:heavy_check_mark: DBAgent module


:heavy_check_mark: Zone module


:heavy_check_mark: Chat module


# Architecture
![게임 서버 구조도](https://user-images.githubusercontent.com/96270683/221395601-73432038-1ac3-4c96-b880-fa8f73992428.png)
- The server consists of a number of sub-modules, each functionally separated.
- In actual live service, even the same module(say User module) can be exist more than one according to the number of users that can be accommodated.
- Each module communicates with other modules through the Coordinator or Connector module, depending on the configuration.


# Server Container
- When the server starts, it loads the modules defined in the config.json file, such as user, zone, and chat.
- The modules to be loaded may depend on the configuration of your server.
- For example, some servers may not have a zone module or may not have a dbagent module.
``` json
    {
      "name": "ZoneServer",
      "use": "true",
      "servicetype": "3",
      "executable": "../.././DLLs/ZoneServer.dll",
      "config": "../.././Config/ZoneServer.json",
      "log": {
        "loglevel": "0",
        "console": "false"
      }
    },
    {
      "name": "ChatServer",
      "use": "true",
      "servicetype": "4",
      "executable": "../.././DLLs/ChatServer.dll",
      "config": "../.././Config/ChatServer.json",
      "log": {
        "loglevel": "0",
        "console": "false"
      }
    },
    {
      "name": "UserServer",
      "use": "true",
      "servicetype": "2",
      "executable": "../.././DLLs/UserServer.dll",
      "config": "../.././Config/UserServer.json",
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
		auto serverPair = m_ServerMap.emplace(name, new IServer());

		auto executable = server.second.get<std::string>("executable");
		
		//Load DLL files
		HMODULE handle = LoadLibrary(executable.c_str());
		if (nullptr == handle)
		{
			cout << "DLL load error. GetLastError :" << GetLastError() << std::endl;
			continue;
		}
		
		//Execute the loaded module
		typedef int(*CREATE_SERVER_FUNC)(IServerContainer* pServerContainer, IServer*& pServer);
		CREATE_SERVER_FUNC funcPtr = (CREATE_SERVER_FUNC)::GetProcAddress(handle, "CreateServerInstance");
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
- The Server Container is the container that manages all modules uploaded to that server and holds references to all server modules.


# Network module
- Network module is a module responsible for communication with clients.
- Network communication was implemented using the Boost Asio network library.
- When a client connects, it creates a NetGameSession class.
``` c++
bool NetworkServer::StartTcpServer()
{
	cout << "[NetworkServer] StartTcpServer" << endl;

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

void NetworkServer::RegisterAccept()
{
	m_Acceptor.async_accept([this](boost::system::error_code error, boost::asio::ip::tcp::socket socket)
	{
		cout << "[NetworkServer] OnAccept" << endl;

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
- Each user performs packet communication through the session object of the server connected to his/her client.
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
				m_pNetworkServer->DispatchClientMsg(EUserServer, m_Msg, self);
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
- The User module is the starting point for actually executing requests from clients.
- After analyzing the request to be performed by disassembling the packet from the client, it is put in the JobQueue of the User module in the form of a Job.
``` c++
//Put received messages in JobQueue.
int UserServer::HandleMsg(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[UserServer] HandleMsg. PktId:" << msg.GetPktId() << endl;

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
- User main data must be referenced in the User module.
- When user information is changed in another module, the user data must be updated by sending a packet to the User module.

# DBAgent module
- The DBAgent module handles requests to the DB.
- All DB requests are made through the DBAgent module.
- When the module is loaded, it establishes a connection with the DB.
``` c++
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
- When requesting a login, if it is a new user, I implemented it to create user data in the DB.
``` c++
int DBAgent::CreateUserToDB(std::string userName)
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


# Zone module
- A module in the game where the world (map) exists, in which the player interacts with other players or NPCs.
- When the Zone module is executed, an NPC is created, and a Player object is created when the user enters the game.
``` c++
void ZoneServer::CreateNPCs()
{
	cout << "[ZoneServer] CreateNPCs" << endl;

	//Create NPC object
	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<CMonster> monster(new CMonster(1000 + i, "Monster" + i));
		monster->SetPosition(i, 0);
		m_MonsterList.push_back(monster);
	}
}

EResultType ZoneServer::Handle_C_ENTER_GAME(NetMsg msg)
{
	cout << "[ZoneServer] Handle_C_ENTER_GAME" << endl;

	//Disassemble packet
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return EResultType::PKT_ERROR;
	}

	std::string playerName = "Player" + to_string(pkt.playerid());

	cout << "[ZoneServer] " << playerName << " entering game..." << endl;

	std::shared_ptr<CPlayer> player(new CPlayer(pkt.playerid(), playerName));
	m_PlayerList.push_back(player);

	cout << "[ZoneServer] " << playerName << " enter game success." << endl;

	return EResultType::SUCCESS;
}
```
- The Zone module has a Tick function.
- The Tick function is a function that is repeatedly executed at the promised cycle and continuously updates the state of the world, players, NPCs, etc.
``` c++
void ZoneServer::RunTick()
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
void ZoneServer::Tick(float deltaTime)
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
- When a user enters the game, it is added to the user session list.
``` c++
EResultType ChatServer::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatServer] Handle_C_ENTER_GAME" << endl;

	//Disassemble packet
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return EResultType::PKT_ERROR;
	}

	cout << "[ChatServer] Player" << to_string(pkt.playerid()) << " has entered chat room." << endl;

	//Add user to session list
	m_UserSessionList.push_back(session);

	return EResultType::SUCCESS;
}
```
- When a specific user sends a chat message, it is broadcasted to other users.
``` c++
EResultType ChatServer::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatServer] Handle_C_CHAT" << endl;

	//Disassemble packet
	Protocol::C_CHAT pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return EResultType::PKT_ERROR;
	}

	cout << "[ChatServer] [Player" << to_string(pkt.playerid()) << "] " << pkt.msg() << endl;

	std::string broadcastMsgStr = "[Player" + to_string(pkt.playerid()) + "] : " + pkt.msg();
	BroadCastAll(broadcastMsgStr);

	return EResultType::SUCCESS;
}

void ChatServer::BroadCastAll(std::string broadcastMsgStr)
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
