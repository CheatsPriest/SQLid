#pragma once

#include "MemoryMap.h"
#include "InfoLoader.h"
#include <memory>

class Tabble {
private:
    static std::string makeStorageName(const std::string& base) {
        return base + "_storage.db";
    }

    static std::string makeLoaderName(const std::string& base) {
        return base + "_info.db";
    }

    std::string baseName;
    std::string nameForStorage;
    std::string nameForLoader;
    InfoLoader loader;
    



public:

    MemoryMap storage;

    Tabble(std::string baseName_)
        : baseName(std::move(baseName_))
        , nameForStorage(makeStorageName(baseName))
        , nameForLoader(makeLoaderName(baseName))
        , loader(nameForLoader)
        , storage(nameForStorage, loader.getCurrentInfo()->lineLength)
    {
    }

    Tabble(const Tabble&) = delete;
    Tabble& operator=(const Tabble&) = delete;
    Tabble(Tabble&&) = default;
    Tabble& operator=(Tabble&&) = default;

    std::shared_ptr<TabbleInfo> getInfo() {
        return loader.getCurrentInfo();
    }

};