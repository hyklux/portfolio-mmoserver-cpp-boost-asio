#pragma once

struct NetHeader
{
	uint16_t size;
	uint16_t id; // 프로토콜ID (ex. 1=로그인, 2=이동요청)
};