#ifndef ModuleCrtHelpers_h__
#define ModuleCrtHelpers_h__

#include "stdafx.h"
#include "Streams.h"

typedef enum
{
  INVALID_ARGUMENT = -1,
  READ_FAILED = -2,
  NOT_A_NUMBER = -3,
  NOT_A_POSITIVE_NUMBER = -4,
  NEGATIVE_NUMBER = -5,
  NUMBER_OVERFLOW = -6,
  NUMBER_UNDERFLOW = -7,
} StreamHelperErrorCode;

static inline int32_t ReadSignedNonNegativeInt32FromBytes(CFileStream* stream)
{
    if (stream == nullptr)
        return StreamHelperErrorCode::INVALID_ARGUMENT;

    unsigned char buffer[sizeof(int32_t)];
    if (!stream->ReadBuffer(&buffer, sizeof(int32_t)))
        return StreamHelperErrorCode::READ_FAILED;

    int32_t result = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
    if (result < 0)
        return StreamHelperErrorCode::NEGATIVE_NUMBER;

    return result;
}

static inline int32_t ReadSignedPositiveInt32FromBytes(CFileStream* stream)
{
    int32_t result = ReadSignedNonNegativeInt32FromBytes(stream);

    if (result == 0)
        return StreamHelperErrorCode::NOT_A_POSITIVE_NUMBER;

    return result;
}

static inline int64_t ReadSignedPositiveInt64FromHexString(CFileStream* stream)
{
    if (stream == nullptr)
        return StreamHelperErrorCode::INVALID_ARGUMENT;

    char buffer[sizeof(int64_t)];
    if (!stream->ReadBuffer(&buffer, sizeof(int64_t)))
        return StreamHelperErrorCode::READ_FAILED;

    char *dummy;
    int64_t result = std::strtoll(buffer, &dummy, 16);
    if (errno == ERANGE)
    {
        if (result == LLONG_MIN)
            return StreamHelperErrorCode::NUMBER_UNDERFLOW;
        if (result == LLONG_MAX)
            return StreamHelperErrorCode::NUMBER_OVERFLOW;
    }

    if (result == 0)
        return StreamHelperErrorCode::NOT_A_NUMBER;

    if (result < 0)
        return StreamHelperErrorCode::NOT_A_POSITIVE_NUMBER;

    return result;
}

#endif
