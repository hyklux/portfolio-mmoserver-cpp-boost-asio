#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"

#include "ThreadSafeQueue.h"
#include "NetClient.h"
#include "NetMsg.h"
#include "Protocol.pb.h"

#include <thread>

const char SERVER_IP[] = "127.0.0.1";
const unsigned short PORT_NUMBER = 10000;

enum : uint16_t
{
	MSG_C_LOGIN = 1000,
	MSG_S_LOGIN = 1001,
	MSG_C_ENTER_GAME = 1002,
	MSG_S_ENTER_GAME = 1003,
	MSG_C_CHAT = 1004,
	MSG_S_CHAT = 1005,
};

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override
	{
	}

	virtual void OnRecvPacket(BYTE* buffer, int32 len) override
	{
		PacketSessionRef session = GetPacketSessionRef();
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);

		// TODO : packetId 대역 체크
		ServerPacketHandler::HandlePacket(session, buffer, len);
	}

	virtual void OnSend(int32 len) override
	{
		//cout << "OnSend Len = " << len << endl;
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}
};

int main()
{
	bool m_IsServerOn = false;
	int m_ThreadCnt = 1;
	std::vector<std::unique_ptr<std::thread>> m_ThreadList;

	this_thread::sleep_for(1s);

	//[boost asio 초기화]
	boost::asio::io_service io_service;
	std::shared_ptr<boost::asio::io_service::work> worker;

	//before you call run() of the io_service yourIOService
	worker = std::make_shared<boost::asio::io_service::work>(io_service);
	worker->get_io_context().restart();

	//[서버 연결]
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(SERVER_IP), PORT_NUMBER);

	NetClient netClient1(io_service);
	netClient1.Connect(endpoint);
	io_service.run();

	while (!netClient1.IsConnected())
	{
		continue;
	}

	m_IsServerOn = true;

	//로그인 
	
	// Seed the random number generator with the current time
	std::srand(std::time(nullptr));

	// Generate a random number between 0 and 99
	int randomPlayerId = std::rand() % 100;

	Protocol::C_LOGIN loginPkt;
	loginPkt.set_username("Player" + randomPlayerId);

	cout << "[Client] " << loginPkt.username() << " requesting login..." << endl;
	NetMsg loginMsg;
	loginMsg.MakeBuffer(loginPkt, MSG_C_LOGIN);
	netClient1.SendMsgToServer(loginMsg);

	while (!netClient1.IsLoggedIn())
	{
		continue;
	}

	this_thread::sleep_for(1s);

	//게임 진입
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerid(randomPlayerId);

	cout << "[Client] Player" << enterGamePkt.playerid() << " requesting enter game..." << endl;
	NetMsg enterGameMsg;
	enterGameMsg.MakeBuffer(enterGamePkt, MSG_C_ENTER_GAME);
	netClient1.SendMsgToServer(enterGameMsg);

	while (!netClient1.HasEnteredGame())
	{
		continue;
	}

	int cnt = 0;
	while (cnt < 10)
	{
		this_thread::sleep_for(1s);
		
		cout << "[Client] Writing message..." << endl;
		Protocol::C_CHAT chatPkt;
		chatPkt.set_playerid(randomPlayerId);
		chatPkt.set_msg("Blah blah blah...");

		NetMsg chatMsg;
		chatMsg.MakeBuffer(chatPkt, MSG_C_CHAT);
		netClient1.SendMsgToServer(chatMsg);

		cnt++;
	}

	while (m_IsServerOn)
	{
		continue;
	}

	netClient1.Disconnect();

	return 0;
}