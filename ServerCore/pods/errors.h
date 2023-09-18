#pragma once

#include <cstdint>

namespace pods
{
    enum class Error : uint32_t
    {
        NoError,

        ArchiveVersionMismatch,
        MandatoryFieldMissed,
        OptionalFieldMissed,
        CorruptedArchive,
        NotEnoughMemory,
        UnexpectedEnd,
        SizeToLarge,
        WriteError,
        ReadError,
        Eof,

        Max,
        Begin = NoError,

        UnknownError = Max,
    };
}
USE_MAGIC_ENUM(pods::Error);

#define     __PODS_OPTIMZIE__