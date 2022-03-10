// PODstore
// Kyle J Burgess

#include "PODstore.h"
#include "PsFile.h"
#include "PsBytes.h"
#include "zlib.h"

#include <vector>
#include <cstring>
#include <iostream>

const char* fileName = "checksum_file.test.bin";

template<PsEndian endian, PsChecksum checksum>
bool testFile()
{
    File file(fileName, FM_READ);

    if (!file.is_open())
    {
        return false;
    }

    std::vector<uint8_t> buffer(1024);

    size_t bytesRead = 0;

    while (true)
    {
        size_t tmp = file.read(buffer.data(), 1024);
        bytesRead += tmp;

        if (tmp < 1024)
        {
            buffer.resize(bytesRead);
            break;
        }

        buffer.resize(bytesRead + 1024);
    }

    uint32_t c0 = 0x236534AAu, c1 = 0x236534AAu;

    std::vector<uint8_t> crcbuf(4);
    memcpy(crcbuf.data(), buffer.data() + buffer.size() - 4, 4);

    bool requiresByteSwap =
        (endian == PS_ENDIAN_LITTLE && is_big_endian()) ||
        (endian == PS_ENDIAN_BIG && is_little_endian());

    if (requiresByteSwap)
    {
        get_bytes<uint32_t, true>(c0, crcbuf, 0, 4);
    }
    else
    {
        get_bytes<uint32_t, false>(c0, crcbuf, 0, 4);
    }

    switch(checksum)
    {
        case PS_CHECKSUM_ADLER32:
            c1 = adler32(c1, buffer.data(), buffer.size() - 4);
            break;
        case PS_CHECKSUM_CRC32:
            c1 = crc32(c1, buffer.data(), buffer.size() - 4);
            break;
        default:
            break;
    }

    if (c0 != c1)
    {
        std::cout << "c0 = " << c0 << "\n";
        std::cout << "c1 = " << c1 << "\n";
        return false;
    }

    return true;
}

template<PsEndian endian, PsChecksum checksum>
bool test()
{
    uint8_t c8[20];
    uint8_t u8[6];
    uint16_t u16[7];
    uint32_t u32[8];
    uint64_t u64[9];
    int8_t i8[6];
    int16_t i16[7];
    int32_t i32[8];
    int64_t i64[9];
    float f32[11];
    double f64[12];

    auto serializer = psCreateContainer();

    psSetValues(psGetItem(serializer, "test string"), c8, 20, PS_ASCII_CHAR8);

    psSetValues(psGetItem(serializer, "UKeyA"), u8, 6, PS_UINT8);
    psSetValues(psGetItem(serializer, "UKeyB"), u16, 7, PS_UINT16);
    psSetValues(psGetItem(serializer, "UKeyC"), u32, 8, PS_UINT32);
    psSetValues(psGetItem(serializer, "UKeyD"), u64, 9, PS_UINT64);

    psSetValues(psGetItem(serializer, "KeyE"), i8, 6, PS_INT8);
    psSetValues(psGetItem(serializer, "KeyF"), i16, 7, PS_INT16);
    psSetValues(psGetItem(serializer, "KeyG"), i32, 8, PS_INT32);
    psSetValues(psGetItem(serializer, "KeyH"), i64, 9, PS_INT64);

    psSetValues(psGetItem(serializer, "FloatKeyI"), f32, 11, PS_FLOAT32);
    psSetValues(psGetItem(serializer, "DoubleKeyJ"), f64, 12, PS_FLOAT64);

    if (psSaveFile(serializer, fileName, PS_COMPRESSION_6, checksum, 0x236534AAu, endian) != PS_SUCCESS)
    {
        return false;
    }

    psDeleteContainer(serializer);

    if (!testFile<endian, checksum>())
    {
        return false;
    }

    return true;
}

int main()
{
    if (!test<PS_ENDIAN_NATIVE, PS_CHECKSUM_CRC32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_NATIVE << ", checksum = " << PS_CHECKSUM_CRC32 << "\n";
        return -1;
    }

    if (!test<PS_ENDIAN_LITTLE, PS_CHECKSUM_CRC32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_LITTLE << ", checksum = " << PS_CHECKSUM_CRC32 << "\n";
        return -1;
    }

    if (!test<PS_ENDIAN_BIG, PS_CHECKSUM_CRC32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_BIG << ", checksum = " << PS_CHECKSUM_CRC32 << "\n";
        return -1;
    }

    if (!test<PS_ENDIAN_NATIVE, PS_CHECKSUM_ADLER32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_NATIVE << ", checksum = " << PS_CHECKSUM_ADLER32 << "\n";
        return -1;
    }

    if (!test<PS_ENDIAN_LITTLE, PS_CHECKSUM_ADLER32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_LITTLE << ", checksum = " << PS_CHECKSUM_ADLER32 << "\n";
        return -1;
    }

    if (!test<PS_ENDIAN_BIG, PS_CHECKSUM_ADLER32>())
    {
        std::cout << "failed, endian = " << PS_ENDIAN_BIG << ", checksum = " << PS_CHECKSUM_ADLER32 << "\n";
        return -1;
    }

    return 0;
}
