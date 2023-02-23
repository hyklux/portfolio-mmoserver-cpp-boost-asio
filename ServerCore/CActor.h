#pragma once
class CActor
{
private:
	int32_t m_EntityId;

	float m_PosX;
	float m_PosY;

public:
	CActor(int32_t entityId) : m_EntityId(entityId), m_PosX(0.0f), m_PosY(0.0f)
	{

	}

	void SetPosition(float posX, float posY)
	{
		m_PosX = posX;
		m_PosY = posY;
	}
};

