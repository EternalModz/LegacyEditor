#include "RegionManager.hpp"


ChunkManager* RegionManager::getChunk(int x, int z) {
    int index = x + z * 32;
    if (index > CHUNK_COUNT) return nullptr;
    return &chunks[index];
}


ChunkManager* RegionManager::getChunk(int index) {
    if (index > CHUNK_COUNT) return nullptr;
    return &chunks[index];
}


ChunkManager* RegionManager::getNonEmptyChunk() {
    for (auto & chunk : chunks) {
       if (chunk.size != 0) {
           return &chunk;
       }
    }
    return nullptr;
}



void RegionManager::read(Data* dataIn) {
    totalSectors = (dataIn->size + SECTOR_SIZE - 1) / SECTOR_SIZE;

    // step 0: copying data from file
    DataManager managerIn(dataIn);
    if (console == CONSOLE::VITA)
        managerIn.setLittleEndian();

    for (ChunkManager &chunk: chunks) {
        u32 val = managerIn.readInt32();
        chunk.sectors = val & 0xff;
        chunk.location = val >> 8;
    }


    // step 2: read timestamps [1024]
    for (ChunkManager& chunk : chunks) {
        chunk.timestamp = managerIn.readInt32();
    }

    // step 3: read chunk size, decompressed size
    int count = 0;
    for (ChunkManager& chunk : chunks) {
        if (chunk.sectors == 0) continue;

        if (chunk.location + chunk.sectors > totalSectors) {
            printf("[%u] chunk sector[%u, %u] end goes outside file...\n", totalSectors, chunk.location, chunk.sectors);
            throw std::runtime_error("debug here");
        }


        // read chunk info
        managerIn.seek(SECTOR_SIZE * chunk.location);
        count++;

        // allocates memory for the chunk
        chunk.size = managerIn.readInt32();
        chunk.rleFlag = chunk.size >> 31;
        chunk.size &= 0x3FFFFFFF;
        chunk.allocate(chunk.size);

        // set chunk's decompressed size attribute
        switch (console) {
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
                chunk.dec_size = managerIn.readInt32();
                chunk.dec_size = managerIn.readInt32();
                break;
            case CONSOLE::XBOX360:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            default:
                chunk.dec_size = managerIn.readInt32();
                break;
        }

        // each chunk gets its own memory
        memcpy(chunk.start(), managerIn.ptr, chunk.size);

    }


}


void RegionManager::read(File* fileIn) {
    read(&fileIn->data);
}


Data RegionManager::write(CONSOLE consoleIn) {
    // step 1: make sure all chunks are compressed correctly
    // step 2: recalculate sectorCount of each chunk
    // step 3: calculate chunk offsets for each chunk
    int total_sectors = 2;
    for (ChunkManager& chunk : chunks) {
        if (chunk.sectors == 0) {
            chunk.location = 0;
        } else {
            chunk.ensure_compressed(consoleIn);
            u8 chunk_sectors = (chunk.size + SECTOR_SIZE - 1) / SECTOR_SIZE + 1;
            chunk.location = total_sectors;
            total_sectors += chunk_sectors;
        }
    }

    // step 4: allocate memory and create buffer
    u32 data_size = total_sectors * SECTOR_SIZE;
    Data dataOut = Data(data_size);
    DataManager managerOut(dataOut);

    if (consoleIn == CONSOLE::VITA) {
        managerOut.setLittleEndian();
    }

    // step 5: write each chunk offset
    for (ChunkManager &chunk: chunks) {
        u32 val = chunk.sectors | chunk.location << 8;
        managerOut.writeInt32(val);
    }

    // step 6: write each chunk timestamp
    for (const ChunkManager& chunk : chunks) {
        managerOut.writeInt32(chunk.timestamp);
    }

    // step 7: seek to each location, write chunk attr's, then chunk data
    // make sure the pointer is a multiple of SECTOR_SIZE
    for (const ChunkManager& chunk : chunks) {
        if (chunk.sectors == 0) continue;

        managerOut.seek(chunk.location * SECTOR_SIZE);

        u32 size = chunk.size;
        if (chunk.rleFlag) {
            u32 mask = 0x00FFFFFF;
            size &= mask;
            size |= (0xC0 << 24);
        }
        managerOut.writeInt32(size);

        switch (console) {
            case CONSOLE::PS3:
            case CONSOLE::RPCS3:
                managerOut.writeInt32(chunk.dec_size);
                managerOut.writeInt32(chunk.dec_size);
                break;
            case CONSOLE::XBOX360:
            case CONSOLE::WIIU:
            case CONSOLE::VITA:
            default:
                managerOut.writeInt32(chunk.dec_size);
                break;
        }
        managerOut.writeBytes(chunk.start(), chunk.size);
    }

    return dataOut;
}