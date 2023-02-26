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
(도식화 그림)
- 서버는 다수의 세부 모듈로 구성되어 있습니다.
- 각 모듈은 기능적으로 분리되어 있습니다.
- 실제 서비스에서는 같은 모듈이라도 수용 가능한 접속 유저 수에 따라 여러 개로 분리될 수 있습니다.
- 각 모듈은 Connector 모듈을 통해 다른 모듈과 통신합니다.


# Server Container
- 서버가 시작되면 config.json 파일에 정의된 user, zone, chat 등의 모듈을 로드합니다. 로드할 모듈은 해당 서버의 구성에 따라 달라질 수 있습니다. 예를 들어 어떤 서버에는 zone 모듈이 없다거나 어떤 서버에는 dbagent 모듈이 없을 수 있습니다.
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
- 각 모듈은 개별 DLL 파일로 만들어져 있어 모듈 로드시 DLL 파일을 로드하기 됩니다.
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
		HMODULE handle = LoadLibrary(executable.c_str());
		if (nullptr == handle)
		{
			cout << "DLL load error. GetLastError :" << GetLastError() << std::endl;
			continue;
		}

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
- Server Container는 해당 서버에 업로드된 모든 모듈을 관리하는 컨테이너로 모든 서버 모듈에 대한 참조를 갖고 있습니다.


# Network 모듈
- Network 모듈은 클라이언트와의 통신을 담당하는 모듈입니다.
- 클라이언트가 접속하면 NetGameSession 클래스를 생성하여 이 객체를 통해 각 클라이언트와 통신하게 됩니다.
- Boost Asio 네트워크 라이브러리를 사용하여 네트워크 통신을 구현하였습니다.


# User 모듈
- User 모듈은 클라이언트로부터 온 요청을 실제로 실행하기 시작하는 시작점이 되는 모듈입니다.
- 클라이언트부터 온 패킷을 분해하여 수행해야 할 요청을 해석한 후 User 모듈의 JobQueue에 Job의 형태로 담아줍니다.
- User 모듈의 WorkerThread에서 JobQueue에 쌓인 Job을 하나씩 수행하게 됩니다.
- User 모듈은 유저들의 주요 정보를 처리하는 역할도 합니다.
- 유저 주요 정보는 반드시 User 모듈에서 참조하여야 하며, 다른 모듈에서 유저 정보가 변경되었을 시 User 모듈로 패킷을 보내 유저 정보를 최신화해야 합니다.

# DBAgent 모듈
- DBAgent 모듈은 DB에 대한 요청을 처리하는 모듈입니다.
- 모든 DB에 대한 요청은 DBAgent 모듈을 통해 수행됩니다.
- 모듈 로드 시 DB와 연결을 설정하며, DB 처리에 대한 요청을 받으면 쿼리에 파라미터로 값을 연동하여 쿼리를 실행합니다.
- 로그인 요청 시 신규 유저이면 DB에 유저 데이터를 생성하도록 구현해 보았습니다.


# Zone 모듈
- 게임에서 월드(맵)이 존재하는 모듈입니다.
- 월드에서 플레이어가 다른 플레이어 또는 NPC와 인터랙션합니다.
- Zone 모듈 실행 시 NPC를 생성하고, 유저가 게임에 입장하며 Player 객체를 생성하도록 했습니다.
- Zone 모듈에는 Tick 함수가 존재합니다.
- Tick 함수는 약속된 주기(1초에 60번)로 반복적으로 실행하는 함수로 월드, 플레이어, NPC 등의 상태를 계속적으로 업데이트하고 이를 모든 유저에게 브로드캐스팅 하는 역할을 합니다.
- Tick 함수에서 Player, Monster의 Update 함수를 호출하도록 하여 player와 Monster 클래스는 Update 함수룰 통해 매 프레임 처리해야 할 작업을 가능하게 하였습니다.


# Chat 모듈
