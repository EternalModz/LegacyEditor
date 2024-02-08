#include "ChunkManager.hpp"

#include "LegacyEditor/utils/PS3_DEFLATE/deflateUsage.hpp"
#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/libs/tinf/tinf.h"
#include "LegacyEditor/libs/zlib-1.2.12/zlib.h"

#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/LCE/Chunk/chunkData.hpp"


#include "LegacyEditor/LCE/Chunk/v10.hpp"
#include "LegacyEditor/LCE/Chunk/v11.hpp"
#include "LegacyEditor/LCE/Chunk/v12.hpp"
#include "LegacyEditor/LCE/Chunk/v13.hpp"


namespace editor {


    enum CHUNK_HEADER : i16 {
        HEADER_NBT = 0x0a00,
        V_8 = 0x0008,
        V_9 = 0x0009,
        V_11 = 0x000B,
        V_12 = 0x000C,
        V_13 = 0x000D,
    };


    ChunkManager::ChunkManager() {
        chunkData = new chunk::ChunkData();
    }


    ChunkManager::~ChunkManager() {
        delete chunkData;
    }


    MU void ChunkManager::readChunk(MU const CONSOLE console) const {
        auto managerIn = DataManager(data, size);
        managerIn.seekStart();

        chunkData->lastVersion = managerIn.readInt16();

        switch(chunkData->lastVersion) {
            case HEADER_NBT:
                chunkData->lastVersion = V_11;
                chunk::ChunkV10(chunkData, &managerIn).readChunk();
                break;
            case V_8:
            case V_9:
            case V_11: {
                chunk::ChunkV11(chunkData, &managerIn).readChunk();
                break;
            }
            case V_12: {
                chunk::ChunkV12(chunkData, &managerIn).readChunk();
                break;
            }
            case V_13: {
                // chunk::ChunkV13(chunkData, &managerIn).readChunk();
                break;
            }
            default:;
        }
    }


    MU void ChunkManager::writeChunk(MU CONSOLE console) {
        Data outBuffer(CHUNK_BUFFER_SIZE);
        memset(outBuffer.data, 0, CHUNK_BUFFER_SIZE);
        auto managerOut = DataManager(outBuffer);

        managerOut.seekStart();
        switch (chunkData->lastVersion) {
            case HEADER_NBT: {
                chunk::ChunkV10(chunkData, &managerOut).writeChunk();
                break;
            }
            case V_8:
            case V_9:
            case V_11: {
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV11(chunkData, &managerOut).writeChunk();
                break;
            }
            case V_12: {
                managerOut.writeInt16(chunkData->lastVersion);
                chunk::ChunkV12(chunkData, &managerOut).writeChunk();
                break;
            }
            case V_13: {
                printf("ChunkManager::writeChunk v13 forbidden\n");
                exit(-1);
            }
            default:;
        }

        const Data outData(managerOut.getPosition());
        memcpy(outData.data, outBuffer.data, outData.size);
        outBuffer.deallocate();
        deallocate();

        data = outData.data;
        size = outData.size;
        fileData.setDecSize(size);
    }


    // TODO: rewrite to return status
    void ChunkManager::ensureDecompress(const CONSOLE console) {
        if (fileData.getCompressed() == 0U
            || console == CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return;
        }
        fileData.setCompressed(0U);

        u32 dec_size_copy = fileData.getDecSize();
        Data decompData(fileData.getDecSize());

        int result;
        switch (console) {
            case CONSOLE::XBOX360:
                dec_size_copy = XDecompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::RPCS3:
            case CONSOLE::PS3:
                result = tinf_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::SWITCH:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            case CONSOLE::PS4:
                result = tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::XBOX1:
                break;
            default:
                break;
        }

        deallocate();

        if (fileData.getRLE() != 0U) {
            allocate(fileData.getDecSize());
            RLE_decompress(decompData.start(), decompData.size, start(), dec_size_copy);
            decompData.deallocate();
        } else {
            data = decompData.data;
            size = dec_size_copy;
            decompData.reset();
        }
    }


    // TODO: rewrite to return status
    void ChunkManager::ensureCompressed(const CONSOLE console) {
        if (fileData.getCompressed() != 0U
            || console == CONSOLE::NONE
            || data == nullptr
            || size == 0) {
            return;
        }
        fileData.setCompressed(1U);
        fileData.setDecSize(size);

        if (fileData.getRLE() != 0U) {
            Data rleBuffer(size);
            RLE_compress(data, size, rleBuffer.data, rleBuffer.size);
            deallocate();
            data = rleBuffer.data;
            size = rleBuffer.size;
            rleBuffer.reset();
        }

        // allocate memory and recompress
        auto *const comp_ptr = new u8[size];
        uLongf comp_size = size;

        // TODO: Does it work for vita?
        switch (console) {
            case CONSOLE::XBOX360:
                // TODO: leaks memory
                // XCompress(comp_ptr, comp_size, data_ptr, data_size);
                break;
            case CONSOLE::PS3:
                // TODO: leaks memory
                // tinf_compress(comp_ptr, comp_size, data_ptr, data_size);
                break;

            case CONSOLE::RPCS3: {
                if (const int status = compress(comp_ptr, &comp_size, data, size); status != 0) {
                    printf("error has occurred compressing chunk\n");
                }

                deallocate();
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            }

            case CONSOLE::SWITCH:
            case CONSOLE::PS4:
            case CONSOLE::WIIU:
            case CONSOLE::VITA: {
                if (const int status = compress(comp_ptr, &comp_size, data, size); status != 0) {
                    printf("error has occurred compressing chunk\n");
                }

                deallocate();
                data = comp_ptr;
                size = comp_size;
                comp_size = 0;
                break;
            }
            default:
                break;
        }

    }


}