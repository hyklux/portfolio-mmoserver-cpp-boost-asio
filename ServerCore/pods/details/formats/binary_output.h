﻿#pragma once

#include "../utils.h"

#include "../../errors.h"
#include "../../types.h"

namespace pods
{
    namespace details
    {
        template <class Storage>
        class BinaryOutput final
        {
        public:
            explicit BinaryOutput(Storage& storage) noexcept
                : storage_(storage)
            {
            }

            BinaryOutput(const BinaryOutput<Storage>&) = delete;
            BinaryOutput& operator=(const BinaryOutput<Storage>&) = delete;

            Error startSerialization() noexcept
            {
                return Error::NoError;
            }

            Error endSerialization()
            {
                storage_.flush();
                return Error::NoError;
            }

            Error saveName(const char*) noexcept
            {
                return Error::NoError;
            }

            Error startObject() noexcept
            {
                return Error::NoError;
            }

            Error endObject() noexcept
            {
                return Error::NoError;
            }

            Error startArray(Size size)
            {
                return storage_.put(size);
            }

			Error startSmallArray(uint8_t size)
			{
				return storage_.put(size);
			}

            Error endArray() noexcept
            {
                return Error::NoError;
            }

            Error startMap(Size size)
            {
                return startArray(size);
            }

            Error endMap() noexcept
            {
                return endArray();
            }

            template <class T>
            Error save(T value)
            {
                return storage_.put(value);
            }

            Error save(bool value)
            {
                const Bool n = value ? True : False;
                return storage_.put(n);
            }

            Error save(const std::string& value)
            {
                return saveBlob(value.c_str(), static_cast<Size>(value.size()));
            }

			Error save(const SmallString& value)
			{
                uint8_t size = (uint8_t)value.str_small.size();
				PODS_SAFE_CALL(storage_.put(size));
				if (size == 0) return Error::NoError;
				return storage_.put(value.str_small.c_str(), size);
				//return saveBlob(value.c_str(), static_cast<uint8_t>(value.size()));
			}

			Error save(const SmallBinaryChunk& value)
			{
				uint8_t size = (uint8_t)value.data.size();
				PODS_SAFE_CALL(storage_.put(size));
				if (size == 0) return Error::NoError;
				return storage_.put(value.data.data(), size);
			}

            template <class T>
            Error saveBlob(const T* data, Size size)
            {
                PODS_SAFE_CALL(storage_.put(size));
                return storage_.put(data, size);
            }

        private:
            Storage& storage_;
        };
    }
}
