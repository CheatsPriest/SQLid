#pragma once

#include "MemoryMap.h"
#include "InfoLoader.h"
#include <memory>
#include "parser/Conditions.h"
#include <stack>

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
    MemoryMap storage;

    std::mutex stack_mtx;
    std::stack<size_t> stack_freeId;
    std::atomic<size_t> freeId = 0;

    void analyzeHoles() {
        std::unique_lock lock(stack_mtx);

        size_t maxLine = storage.getMaxLines();
        size_t ind = 0;
        while (storage.isActive(ind) and ind < maxLine) {
            ++ind;
        }
        if (ind == maxLine)storage.expand();

        maxLine = storage.getMaxLines() - 1;

        while (true) {
            if (storage.isActive(maxLine) or maxLine==ind) {
                if(storage.isActive(ind))freeId = maxLine + 1;
                else {
                    freeId = ind;
                }
                break;
            }
            maxLine--;
        }

        for (; ind < maxLine; ++ind) {
            if (!storage.isActive(ind)) {
                stack_freeId.push(ind);
            }
        }
    }

    size_t allocateId() {
        
        {
            
            std::lock_guard lock(stack_mtx);
            if (!stack_freeId.empty()) {
                size_t id = stack_freeId.top();
                stack_freeId.pop();
                return id;
            }
        }


        size_t cur = freeId.load(std::memory_order_acquire);

        freeId++;

        return cur;
    }

public:

   

    Tabble(std::string baseName_)
        : baseName(std::move(baseName_))
        , nameForStorage(makeStorageName(baseName))
        , nameForLoader(makeLoaderName(baseName))
        , loader(nameForLoader)
        , storage(nameForStorage, loader.getCurrentInfo()->lineLength)
    {

        analyzeHoles();

    }

    Tabble(const Tabble&) = delete;
    Tabble& operator=(const Tabble&) = delete;
    Tabble(Tabble&&) = default;
    Tabble& operator=(Tabble&&) = default;

    MemoryMap& operator()() {
        return storage;
    }

    std::shared_ptr<TabbleInfo> getInfo() {
        return loader.getCurrentInfo();
    }

    size_t insert(const std::vector<variant_types>& values, const std::vector<Column>& columns) {
        size_t cur = allocateId(); // Берём ID из free list

        storage.activate(cur);

        size_t current_offset = 1; // Начинаем после флага активности

        for (size_t i = 0; i < values.size(); ++i) {
            auto& value = values[i];
            auto& column = columns[i];
            

            std::visit([this, cur, offset = column.offset](auto&& val) {
                storage.writeInfo(cur, offset, std::forward<decltype(val)>(val));
                }, value);

            // Помечаем запись как активную
            
        }
        return cur;
    }

};