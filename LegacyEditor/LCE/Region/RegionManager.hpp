#pragma once

#include "ChunkManager.hpp"

#include "LegacyEditor/LCE/MC/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


namespace editor {

    class File;

    class RegionManager {
        const u32 REGION_WIDTH = 32;
        const u32 SECTOR_BYTES = 4096;
        const u32 SECTOR_INTS = SECTOR_BYTES / 4;

    public:
        ChunkManager *chunks;
        u32 sectorBytes;
        u32 sectorInts;
        CONSOLE console = CONSOLE::NONE;

        explicit RegionManager(const CONSOLE consoleIn, u32 sectorBytes)
            : console(consoleIn) {
            sectorBytes =

            chunks = new ChunkManager[]
        }
        ~RegionManager() = default;

        /// FUNCTIONS

        MU ChunkManager* getChunk(int xIn, int zIn);
        MU ChunkManager* getChunk(int index);
        MU ChunkManager* getNonEmptyChunk();

        /// READ

        void read(const File* fileIn);
        void read(const Data* dataIn);

        /// WRITE

        Data write(CONSOLE consoleIn);
    };

}
