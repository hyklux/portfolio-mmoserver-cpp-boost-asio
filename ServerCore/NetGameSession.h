#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <memory>
#include <atomic>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "NetMsg.h"
#include "IServerModule.h"

using namespace std;

class NetMsg;

using _read_buffer_t = std::basic_string<char>;
using _read_stream_t = boost::asio::basic_streambuf<std::allocator<char>>;
using _error_stream_t = std::basic_stringstream<char>;

class NetGameSession : public std::enable_shared_from_this<NetGameSession>
{
private:
	IServerModule* m_pNetworkServer;
	uint32_t m_SessionId;
	uint64_t m_PlayerId;

	boost::asio::ip::tcp::socket m_Socket;
	boost::asio::streambuf streamBuf;
	NetMsg m_Msg;
	bool m_ReceivingData = false;

	_read_stream_t m_ReadStreamBuf;
	_read_buffer_t m_ReadBuf;

public:
	NetGameSession(IServerModule* pNetworkServer, uint32_t sessionId, boost::asio::ip::tcp::socket& _socket) : m_pNetworkServer(pNetworkServer), m_SessionId(sessionId), m_PlayerId(0), m_Socket(std::move(_socket))
	{
		cout << "[NetGameSession] Session created. SessionId:" << m_SessionId << endl;
	}

	~NetGameSession()
	{

	}

	void Start()
	{
		RegisterReceiveHeader();
	}

	boost::asio::ip::tcp::socket& Socket()
	{
		return m_Socket;
	}

	uint32_t GetSessionId()
	{
		return m_SessionId;
	}

	uint64_t GetPlayerId()
	{
		return m_PlayerId;
	}

	void SendMsgToClient(NetMsg msg);

	void RegisterSend(NetMsg msg);

	void RegisterReceiveHeader();

	void RegisterReceiveBody();

	void Disconnect();

	void Disassemble(int nLen, const unsigned char* pchData, int* pnTotalLen, int* pnHdLen, int* pnDataLen);

	void ParseMsg(int nLen, const unsigned char* pchData, const std::shared_ptr<NetGameSession>& session);
};
