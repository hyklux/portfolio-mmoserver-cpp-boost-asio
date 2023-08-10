#pragma once

class ClientPktDispatcher
{
public:
	static bool DispatchClientPkt(uint16_t pktId, std::string pktStr);
};

