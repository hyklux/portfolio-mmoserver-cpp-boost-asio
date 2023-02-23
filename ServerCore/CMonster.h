#pragma once

#include <string>
#include "CActor.h"

class CMonster : public CActor
{
private:
	std::string m_MonsterName;
	uint8_t m_Level;

public:
	CMonster(int32_t entityId, std::string monsterName) : CActor(entityId), m_MonsterName(monsterName), m_Level(1)
	{

	}
};