#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/file.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "ConsoleParser.hpp"
#include "LegacyEditor/utils/dataManager.hpp"

#include <functional>
#include <map>
#include <set>
#include <vector>





class FileList : public std::vector<File*> {
public:
    // ~FileList() {
    //     removeAll();
    // }

    void removeAll() {
        for (File* file : *this) {
            delete[] file->data.data;
        }
        clear();
    }
};


class FileListing {
public:
    std::vector<FileList*> dimFileLists = {&region_nether, &region_overworld, &region_end};
    std::vector<FileList*> entityFileLists = {&entity_overworld, &entity_nether, &entity_end};

    CONSOLE console = CONSOLE::NONE;
    std::vector<File> allFiles;

    // pointers
    FileList region_overworld;
    FileList region_nether;
    FileList region_end;

    FileList entity_overworld;
    FileList entity_nether;
    FileList entity_end;

    FileList maps;
    FileList structures;
    FileList players;

    File* largeMapDataMappings{};
    File* level{};
    File* grf{};
    File* village{};

    // data
    i32 oldestVersion{};
    i32 currentVersion{};

    FileListing() = default;
    explicit FileListing(ConsoleParser& consoleParser) : console(consoleParser.console) {
        read(consoleParser);
    }
    explicit FileListing(ConsoleParser* consoleParser) : console(consoleParser->console) {
        read(*consoleParser);
    }

    void read(Data& dataIn);
    Data write(CONSOLE consoleOut);
    void saveToFolder(const std::string& folder);

    void convertRegions(CONSOLE consoleOut);

    void deleteAllChunks();


    void deallocate();

    void clearPointers();
    void updatePointers();




    void removeFileTypes(std::set<FileType> typesToRemove) {
        allFiles.erase(
                std::remove_if(
                        allFiles.begin(),
                        allFiles.end(),
                        [&typesToRemove](File& file) {
                            bool to_del = typesToRemove.count(file.fileType) > 0;
                            if (to_del) {
                                delete[] file.data.data;
                                file.data.data = nullptr;
                            }
                            return to_del;
                        }
                        ),
                allFiles.end()
        );
        for (const auto& fileType : typesToRemove) {
            if (clearActionsRemove.count(fileType)) {
                clearActionsRemove[fileType]();
            }
        }
    }


    void addFiles(std::vector<File> filesIn) {
        allFiles.insert(allFiles.end(), filesIn.begin(), filesIn.end());
        updatePointers();
    }



    std::vector<File> collectFiles(FileType fileType);



private:
    /// For use in removeFileTypes
    std::map<FileType, std::function<void()>> clearActionsDelete = {
        {FileType::STRUCTURE, [this]() { structures.removeAll(); }},
        {FileType::MAP, [this]() { maps.removeAll(); }},
        {FileType::PLAYER, [this]() { players.removeAll(); }},
        {FileType::REGION_NETHER, [this]() { region_nether.removeAll(); }},
        {FileType::REGION_OVERWORLD, [this]() { region_overworld.removeAll(); }},
        {FileType::REGION_END, [this]() { region_end.removeAll(); }},
        {FileType::ENTITY_NETHER, [this]() { entity_nether.removeAll(); }},
        {FileType::ENTITY_OVERWORLD, [this]() { entity_overworld.removeAll(); }},
        {FileType::ENTITY_END, [this]() { entity_end.removeAll(); }},
        {FileType::VILLAGE, [this]() {
                 delete[] village->data.data;
                 village = nullptr; }},
        {FileType::DATA_MAPPING, [this]() {
                 delete[] largeMapDataMappings->data.data;
                 largeMapDataMappings = nullptr;
         }},
        {FileType::LEVEL, [this]() {
             delete[] level->data.data;
             level = nullptr;
         }},
        {FileType::GRF, [this]() {
             delete[] grf->data.data;
             grf = nullptr;
         }},
    };


    std::map<FileType, std::function<void()>> clearActionsRemove = {
            {FileType::STRUCTURE, [this]() { structures.clear(); }},
            {FileType::MAP, [this]() { maps.clear(); }},
            {FileType::PLAYER, [this]() { players.clear(); }},
            {FileType::REGION_NETHER, [this]() { region_nether.clear(); }},
            {FileType::REGION_OVERWORLD, [this]() { region_overworld.clear(); }},
            {FileType::REGION_END, [this]() { region_end.clear(); }},
            {FileType::ENTITY_NETHER, [this]() { entity_nether.clear(); }},
            {FileType::ENTITY_OVERWORLD, [this]() { entity_overworld.clear(); }},
            {FileType::ENTITY_END, [this]() { entity_end.clear(); }},
            {FileType::VILLAGE, [this]() { village = nullptr; }},
            {FileType::DATA_MAPPING, [this]() {
                 largeMapDataMappings = nullptr;
             }},
            {FileType::LEVEL, [this]() {
                 level = nullptr;
             }},
            {FileType::GRF, [this]() {
                 grf = nullptr;
             }},
    };
};