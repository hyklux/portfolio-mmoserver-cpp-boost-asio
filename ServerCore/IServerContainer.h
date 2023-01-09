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
	virtual void* GetUserServer() { return nullptr; }
	virtual void* GetTargetServer(uint16_t targetServer) { return nullptr; }
};