#pragma once

#include "LegacyEditor/utils/processor.hpp"


namespace editor {

    class LCEFile;

    namespace map {


        MU void saveMapToPng(const LCEFile* map,
                             const std::string& path,
                             const std::string& filename = "map_0.png");
    }
}

