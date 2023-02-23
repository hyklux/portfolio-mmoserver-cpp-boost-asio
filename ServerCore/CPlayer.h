#pragma once

#include <string>
#include "CActor.h"

class CPlayer : public CActor
{
private:
	std::string m_PlayerName;
	uint8_t m_Level;
	uint64_t m_Exp;
	uint64_t m_Gold;

public:
	CPlayer(int32_t entityId, std::string playerName) : CActor(entityId), m_PlayerName(playerName), m_Level(1), m_Exp(0), m_Gold(0)
	{

	}
};