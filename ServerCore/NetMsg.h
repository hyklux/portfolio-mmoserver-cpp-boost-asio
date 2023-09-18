#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <stdio.h>

#include "NetHeader.h"

class NetMsg
{
public:
	enum { HEADER_LENGTH = 4 };
	enum { MAX_BODY_LENGTH = 512 };

private:
	char m_Data[HEADER_LENGTH + MAX_BODY_LENGTH];
	uint16_t m_BodyLength;
	uint16_t m_PktId = -1;

public:
	NetMsg() : m_BodyLength(0)
	{

	}

	~NetMsg()
	{

	}

	uint16_t GetPktId() const
	{
		return m_PktId;
	}

	const char* GetData() const
	{
		return m_Data;
	}

	char* GetData()
	{
		return m_Data;
	}

	uint16_t GetLength() const
	{
		return HEADER_LENGTH + m_BodyLength;
	}

	const char* GetBody() const
	{
		return m_Data + HEADER_LENGTH;
	}

	char* GetBody()
	{
		return m_Data + HEADER_LENGTH;
	}

	uint16_t GetBodyLength() const
	{
		return m_BodyLength;
	}

	void SetBodyLength(uint16_t new_length)
	{
		m_BodyLength = new_length;

		if (m_BodyLength > MAX_BODY_LENGTH)
		{
			m_BodyLength = MAX_BODY_LENGTH;
		}
	}

	bool DecodeHeader()
	{
		NetHeader* header = reinterpret_cast<NetHeader*>(m_Data);
		m_PktId = header->id;
		m_BodyLength = header->size;

		if (m_BodyLength > MAX_BODY_LENGTH)
		{
			m_BodyLength = 0;
			return false;
		}
		return true;
	}

	void EncodeHeader(uint16_t dataBodySize, uint16_t pktId)
	{
		NetHeader* header = reinterpret_cast<NetHeader*>(m_Data);
		header->size = dataBodySize;
		header->id = pktId;
	}

	template<typename T>
	void MakeBuffer(T& pkt, uint16_t pktId)
	{
		const uint16_t dataBodySize = static_cast<uint16_t>(pkt.ByteSizeLong());
		EncodeHeader(dataBodySize, pktId);
		m_BodyLength = dataBodySize;
		pkt.SerializeToArray(GetBody(), dataBodySize);
	}
};