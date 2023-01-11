#pragma once

#include <string>

class CPlayer
{
private:
	int32_t m_PlayerId;
	std::string m_PlayerName;
	
	float m_PosX;
	float m_PosY;
	float m_PosZ;

public:
	CPlayer(int32_t playeId, std::string playerName) : m_PlayerId(playeId), m_PlayerName(playerName), m_PosX(0.0f), m_PosY(0.0f), m_PosZ(0.0f)
	{

	}
};

