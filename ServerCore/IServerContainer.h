#pragma once

#include <string>

#include "IServerModule.h"
#include "NetMsg.h"

class IServerContainer
{
public:
	virtual int AddRef(void) { return 0; }
	virtual int ReleaseRef(void) { return 0; }

	virtual void* GetConnectorModule() { return nullptr; }
	virtual void* GetUserModule() { return nullptr; }
	virtual void* GetTargetModule(uint16_t targetModule) { return nullptr; }
};