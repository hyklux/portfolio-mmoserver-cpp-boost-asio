#pragma once

#include <cstdint>
#include <limits>

namespace pods
{
    using Bool = uint8_t;
    using Size = uint32_t;
    using Version = uint32_t;

    static constexpr Bool False = 0;
    static constexpr Bool True = 1;

    static constexpr Version NoVersion = 0;

	//static constexpr Size MaxSize = std::numeric_limits<Size>::max() - 1;
	static constexpr Size MaxSize = (std::numeric_limits<Size>::max)() - 1;
}


// 255 이하의 크기를 가지는 std::string. length 는 byte 로 처리.
struct SmallString
{
	std::string		str_small;

	SmallString() {}
	SmallString(const std::string& _str) : str_small(_str) {}
};

struct SmallBinaryChunk
{
	void Set(uint8_t* data_p, uint8_t size_p) {
		data.resize(size_p);
		uint8_t* vec_data = data.data();
		memcpy(vec_data, data_p, size_p);
	}
	bool Copy(uint8_t* data_p, uint8_t size_p) {
		if (size_p > data.size()) {
			return false;
		}
		uint8_t* vec_data = data.data();
		memcpy(data_p, vec_data, size_p);
		return true;
	}
	uint8_t* GetPtr() {
		return data.data();
	}
	//bool GetSize(uint8_t& chunk_size) {
	//	if (data.size() < sizeof(uint8_t)) {
	//		return false;
	//	}
	//	uint8_t* read_size = data.data();
	//	chunk_size = *read_size;
	//	return true;
	//}

	std::vector<uint8_t>	data;
};

template <class VecElement>
class SmallVector
{
public:
	std::vector< VecElement>	vec;
};