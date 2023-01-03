#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "NetHeader.h"

class NetMsg
{
public:
	enum { BUFFER_SIZE = 128 };

private:
	char m_Data[BUFFER_SIZE];
	uint16_t m_HeaderLength;
	uint16_t m_BodyLength;
	uint16_t m_PktId = -1;

public:
	NetMsg() : m_HeaderLength(sizeof(NetHeader)), m_BodyLength(0)
	{

	}

	~NetMsg()
	{

	}

	uint16_t GetPktId() const
	{
		return m_PktId;
	}

	uint16_t GetLength() const
	{
		return BUFFER_SIZE;
	}

	const char* GetData() const
	{
		return m_Data;
	}

	char* GetData()
	{
		return m_Data;
	}

	const char* GetBody() const
	{
		return m_Data + m_HeaderLength;
	}

	char* GetBody()
	{
		return m_Data + m_HeaderLength;
	}

	uint16_t GetBodyLength() const
	{
		return m_BodyLength;
	}

	void SetBodyLength(uint16_t new_length)
	{
		m_BodyLength = new_length;

		if (m_BodyLength > BUFFER_SIZE - m_HeaderLength)
		{
			m_BodyLength = BUFFER_SIZE - m_HeaderLength;
		}
	}

	bool DecodeHeader()
	{
		NetHeader* header = reinterpret_cast<NetHeader*>(GetData());
		m_PktId = header->id;
		m_BodyLength = static_cast<uint16_t>(header->size);
		if (m_BodyLength > BUFFER_SIZE - m_HeaderLength)
		{
			m_BodyLength = 0;
			return false;
		}

		return true;
	}

	void EncodeHeader(NetHeader netHeader)
	{
		std::memcpy(m_Data, &netHeader, sizeof(NetHeader));
	}

	template<typename T>
	void MakeBuffer(T& pkt, uint16_t pktId)
	{
		const uint16_t dataSize = static_cast<uint16_t>(pkt.ByteSizeLong());

		NetHeader* header = reinterpret_cast<NetHeader*>(GetData());
		header->size = dataSize;
		header->id = pktId;
		pkt.SerializeToArray(GetBody(), dataSize);
	}
};