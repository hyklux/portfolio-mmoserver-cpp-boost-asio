# portfolio-mmoserver-cpp-boost-asio
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크


# 소개
C++ Boost asio 라이브러리 기반 MMO 서버 프레임워크입니다.


# 기능
:heavy_check_mark: 서버 아키텍처


:heavy_check_mark: Server Container


:heavy_check_mark: Network 모듈


:heavy_check_mark: User 모듈


:heavy_check_mark: DBAgent 모듈


:heavy_check_mark: Zone 모듈


:heavy_check_mark: Chat 모듈


# 서버 아키텍처
![게임 서버 구조도](https://user-images.githubusercontent.com/96270683/221395601-73432038-1ac3-4c96-b880-fa8f73992428.png)
- 서버는 다수의 세부 모듈로 구성되어 있으며 각 모듈은 기능적으로 분리되어 있습니다.
- 실제 서비스에서는 같은 모듈이라도 수용 가능한 접속 유저 수에 따라 여러 개로 분리될 수 있습니다.
- 각 모듈은 Connector 모듈을 통해 다른 모듈과 통신합니다.


# Server Container
- 서버가 시작되면 config.json 파일에 정의된 user, zone, chat 등의 모듈을 로드합니다.
- 로드할 모듈은 해당 서버의 구성에 따라 달라질 수 있습니다. 예를 들어 어떤 서버에는 zone 모듈이 없거나 dbagent 모듈이 없을 수 있습니다.
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
- 각 모듈은 개별 DLL 파일로 만들어져 있어 모듈 로드시 DLL 파일을 로드하게 됩니다.
``` c++
class ServerContainer
{
	//...(중략)
 
	auto servers = props.get_child("servers");
	for (const auto& server : servers)
 	{
		auto name = server.second.get<std::string>("name");
		auto serverPair = m_ServerMap.emplace(name, new IServer());

		auto executable = server.second.get<std::string>("executable");
		
		//DLL 파일 로드하기
		HMODULE handle = LoadLibrary(executable.c_str());
		if (nullptr == handle)
		{
			cout << "DLL load error. GetLastError :" << GetLastError() << std::endl;
			continue;
		}
		
		//로드한 모듈 실행
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
  
	//...(중략)
}
```
 ![서버 모듈 로드](https://user-images.githubusercontent.com/96270683/221408977-60f10220-00cd-4dc3-a9bf-61efd9b04be6.PNG)
- Server Container는 해당 서버에 업로드된 모든 모듈을 관리하는 컨테이너로 모든 서버 모듈에 대한 참조를 갖고 있습니다.


# Network 모듈
- Network 모듈은 클라이언트와의 통신을 담당하는 모듈입니다.
- 네트워크 통신은 Boost Asio 네트워크 라이브러리를 사용하여 구현했습니다.
- 클라이언트가 접속하면 NetGameSession 클래스를 생성합니다.
``` c++
bool NetworkServer::StartTcpServer()
{
	cout << "[NetworkServer] StartTcpServer" << endl;

	try
	{
		//엔드포인트 설정
		boost::asio::ip::tcp::endpoint endpoint_;
		endpoint_.address(boost::asio::ip::address::from_string(m_IpAddr));
		endpoint_.port(m_PortNo);

		//서버 리슨 시작
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

		//클라이언트 소켓과 연결되면 게임 세션 클래스 생성
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
(사진) 세션 
- 각 유저는 자신의 클라이언트와 연결된 서버의 세션 객체를 통해 패킷 통신을 하게 됩니다.
``` c++
//클라이언트에게 Send 처리
void NetGameSession::RegisterSend(NetMsg msg)
{
	boost::asio::async_write(m_Socket, boost::asio::buffer(msg.GetData(), msg.GetLength()), [&](error_code error, std::size_t bytes_transferred)
	{
		std::cout << "[NetGameSession] All data has been sent. Bytes transferred:" << bytes_transferred << std::endl;
	});
}

//클라이언트로부터 Receive 처리
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

# User 모듈
- User 모듈은 클라이언트로부터 온 요청을 실제로 실행하기 시작하는 시작점이 되는 모듈입니다.
- 클라이언트부터 온 패킷을 분해하여 수행해야 할 요청을 해석한 후 User 모듈의 JobQueue에 Job의 형태로 담아줍니다.
``` c++
//JobQueue에 넣어준다.
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
- User 모듈의 WorkerThread에서 JobQueue에 쌓인 Job을 하나씩 수행하게 됩니다.
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

		// 맨 앞의 job 을 뺀다.
		std::function<void()> job = std::move(m_JobQueue.front());
		m_JobQueue.pop();
		lock.unlock();

		cout << "[ThreadPool] Pushing job out of queue..." << endl;

		// 해당 job 을 수행한다
		job();
	}
}
```
- User 모듈은 유저들의 주요 데이터 담당하는 역할도 합니다. 유저 주요 데이터는 반드시 User 모듈에서 참조하여야 하며, 다른 모듈에서 유저 정보가 변경되었을 시 User 모듈로 패킷을 보내 유저 데이터를 최신화해야 합니다.

# DBAgent 모듈
- DBAgent 모듈은 DB에 대한 요청을 처리하는 모듈입니다. 모든 DB에 대한 요청은 DBAgent 모듈을 통해 수행됩니다.
- 모듈 로드 시 DB와 연결을 설정합니다.
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
- DB 처리에 대한 요청을 받으면 쿼리에 파라미터로 값을 연동하여 쿼리를 실행합니다. 로그인 요청 시 신규 유저이면 DB에 유저 데이터를 생성하도록 구현해 보았습니다.
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

# Zone 모듈
- 게임에서 월드(맵)이 존재하는 모듈로 이 월드에서 플레이어가 다른 플레이어 또는 NPC와 인터랙션합니다.
- Zone 모듈 실행 시 NPC를 생성하고, 유저가 게임에 입장하며 Player 객체를 생성하도록 했습니다.
``` c++
void ZoneServer::CreateNPCs()
{
	cout << "[ZoneServer] CreateNPCs" << endl;

	//NPC 객체 생성
	for (int i = 0; i < 5; i++)
	{
		std::shared_ptr<CMonster> monster(new CMonster(1000 + i, "Monster" + i));
		monster->SetPosition(i, 0);
		m_MonsterList.push_back(monster);
	}
}

int ZoneServer::Handle_C_ENTER_GAME(NetMsg msg)
{
	cout << "[ZoneServer] Handle_C_ENTER_GAME" << endl;

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	std::string playerName = "Player" + to_string(pkt.playerid());

	cout << "[ZoneServer] " << playerName << " entering game..." << endl;

	//플레이어 객체 생성
	std::shared_ptr<CPlayer> player(new CPlayer(pkt.playerid(), playerName));
	m_PlayerList.push_back(player);

	cout << "[ZoneServer] " << playerName << " enter game success." << endl;

	return 0;
}
```
- Zone 모듈에는 Tick 함수가 존재합니다. Tick 함수는 약속된 주기(1초에 60번)로 반복적으로 실행하는 함수로 월드, 플레이어, NPC 등의 상태를 계속적으로 업데이트합니다.
``` c++
void ZoneServer::RunTick()
{
	m_CanTick = true;

	while (m_CanTick)
	{
		// 마지막 틱으로 부터 경과한 시간(deltaTime) 계산
		std::clock_t currentTime = std::clock();
		float deltaTime = ((float)(currentTime - lastTickTime)) / CLOCKS_PER_SEC;

		// deltaTime이 기준 프레임 시간 이상 되었는지 확인
		if (deltaTime >= (1.0f / TICK_RATE)) 
		{
			//Tick함수 실행
			Tick(deltaTime);

			lastTickTime = currentTime;
		}
	}
}

//매 프레임 처리해야 할 작업 수행
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


# Chat 모듈
- 채팅 메세지를 처리하는 모듈입니다.
- 유저가 게임에 진입하면 유저 세션 리스트에 추가해 줍니다.
``` c++
int ChatServer::Handle_C_ENTER_GAME(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatServer] Handle_C_ENTER_GAME" << endl;

	//패킷 분해
	Protocol::C_ENTER_GAME pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatServer] Player" << to_string(pkt.playerid()) << " has entered chat room." << endl;

	//세션 리스트에 유저 추가
	m_UserSessionList.push_back(session);

	return 0;
}
```
- 특정 유저가 채팅 메세지를 전송하면 다른 유저에게 브로드 캐스팅합니다.
``` c++
int ChatServer::Handle_C_CHAT(const NetMsg msg, const std::shared_ptr<NetGameSession>& session)
{
	cout << "[ChatServer] Handle_C_CHAT" << endl;

	//패킷 분해
	Protocol::C_CHAT pkt;
	if (false == ParsePkt(pkt, msg))
	{
		return static_cast<uint16_t>(ERRORTYPE::PKT_ERROR);
	}

	cout << "[ChatServer] [Player" << to_string(pkt.playerid()) << "] " << pkt.msg() << endl;

	std::string broadcastMsgStr = "[Player" + to_string(pkt.playerid()) + "] : " + pkt.msg();
	BroadCastAll(broadcastMsgStr);

	return 0;
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
(사진) 채팅 메세지 처리
