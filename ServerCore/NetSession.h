#pragma once

#include <SDKDDKVer.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>

using namespace std;

class NetSession
{
private:

public:
	NetSession()
	{
	}

	~NetSession()
	{
	}

	virtual void Send() = 0;
	virtual void Receive() = 0;
};