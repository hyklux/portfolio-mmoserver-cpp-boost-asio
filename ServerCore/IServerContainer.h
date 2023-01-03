#pragma once

#include <string>

#include "IServer.h"
#include "NetMsg.h"

class IServerContainer
{
public:
	virtual int AddRef(void) { return 0; }
	virtual int ReleaseRef(void) { return 0; }

	virtual void* GetConnectorServer() { return nullptr; }
	virtual int DispatchMsg(uint16_t targetServer, NetMsg msg) { return -1; }
};