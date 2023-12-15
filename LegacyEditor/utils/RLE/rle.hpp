#pragma once

#include "LegacyEditor/utils/processor.hpp"


static void RLE_decompress(u8* dataIn, const u32 sizeIn, u8* dataOut, u32& sizeOut) {
    DataManager in(dataIn, sizeIn);
    DataManager out(dataOut, sizeOut);

    while (in.getPosition() < sizeIn) {
        const u8 b = in.readInt8();
        if (b != 255) {
            out.writeInt8(b);
        } else {
            const u8 b2 = in.readInt8();
            u8 value = 255;
            if (b2 >= 3) {
                value = in.readInt8();
            }
            for (int j = 0; j <= static_cast<int>(b2); j++) {
                out.writeInt8(value);
            }
        }
    }
    sizeOut = out.getPosition();
}


static void RLE_compress(const u8* dataIn, const u32 sizeIn, u8* dataOut, u32& sizeOut) {
    u32 dataIndex = 0;
    sizeOut = 0;

    while (dataIndex < sizeIn) {
        const u8 value = dataIn[dataIndex];
        u32 count = 1;

        // Count the run of the same byte
        while (dataIndex + count < sizeIn && dataIn[dataIndex + count] == value && count < 256) {
            count++;
        }

        if (value == 255) {
            if (count < 4) {
                dataOut[sizeOut++] = 255;
                dataOut[sizeOut++] = count - 1;
            } else {
                dataOut[sizeOut++] = 255;
                dataOut[sizeOut++] = count - 1;
                dataOut[sizeOut++] = value;
            }
        } else {
            if (count >= 4) {
                // If run length is 4 or more, encode it
                dataOut[sizeOut++] = 255;
                dataOut[sizeOut++] = count - 1;
                dataOut[sizeOut++] = value;
            } else {
                // Else write the bytes directly
                for (u32 i = 0; i < count; ++i) {
                    dataOut[sizeOut++] = value;
                }
            }
        }

        dataIndex += count;
    }
}