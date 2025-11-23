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
    
    //To Do: Change it to lock-free stack
    std::mutex stack_mtx;
    std::stack<size_t> stack_freeId;

    std::atomic<size_t> freeId = 0;

    std::atomic<size_t> activePlaces;

    void analyzeHoles() {
        std::unique_lock lock(stack_mtx);

        size_t maxLine = storage.getMaxLines();
        size_t ind = 0;
        while (ind < maxLine and storage.isActive(ind)) {
            ++ind;
        }
        if (ind == maxLine)storage.expand();

        maxLine = storage.getMaxLines();

        for (size_t i = 0; i < maxLine; ++i) {
            if (storage.isActive(i)) {
                freeId.store(i+1);
                activePlaces.fetch_add(1);
            }
        }
        for (size_t i = 0; i < freeId; ++i) {
            if (!storage.isActive(i)) {
                stack_freeId.push(i);
            }
        }
        std::cout << "First free " << freeId << " " << stack_freeId.size()<<" "<<activePlaces << std::endl;
    }

    void pushInStack(size_t ind) {
        std::lock_guard<std::mutex> lock(stack_mtx);
        stack_freeId.push(ind);
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
        if (cur >= storage.getMaxLines()) {
            storage.expand();
        }

        activePlaces.fetch_add(1, std::memory_order_release);
        freeId.fetch_add(1, std::memory_order_release);

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
    MemoryMap& getStoarge() {
        return storage;
    }


    size_t getActivePlaces() const {
        return activePlaces.load(std::memory_order_acquire);
    }
    size_t getMaxActiveLine() const {
        return freeId.load(std::memory_order_acquire);
    }

    std::shared_ptr<TabbleInfo> getInfo() {
        return loader.getCurrentInfo();
    }

    void update(const std::vector<variant_types>& values, const std::vector<Column>& columns, size_t cur, size_t indexValues=0) {
      
     
        for (size_t i = 0; i < columns.size(); ++i) {
            auto& value = values[i+indexValues];
            auto& column = columns[i];


            std::visit([this, cur, offset = column.offset](auto&& val) {
                storage.writeInfo(cur, offset, std::forward<decltype(val)>(val));
                }, value);

            // Помечаем запись как активную

        }
        
    }

    size_t insert(const std::vector<variant_types>& values, const std::vector<Column>& columns, size_t index) {
        size_t cur = allocateId(); // Берём ID из free list

        storage.activate(cur);

        size_t current_offset = 1; // Начинаем после флага активности

        update(values, columns, cur, index);
        activePlaces.fetch_add(1, std::memory_order_acq_rel);
        return cur;
    }

    size_t erase(size_t line) {
        storage.deactivate(line);
        pushInStack(line);
        activePlaces.fetch_sub(1, std::memory_order_acq_rel);//I have no idea what I have to set here
        return line;
    }

   

    variant_types readNumber(size_t line, size_t offset, Type type) {
        switch (type) {
        case Type::INT32: return storage.readNumber<int32_t>(line, offset);
        case Type::INT64: return storage.readNumber<int64_t>(line, offset);
        case Type::DOUBLE: return storage.readNumber<double>(line, offset);
        case Type::FLOAT: return storage.readNumber<float>(line, offset);
        case Type::BOOL: return storage.readNumber<bool>(line, offset);
        };
    }
    std::string_view readText(size_t line, size_t offset, size_t size=0) {
        if (size == 0) {
            return storage.readText(line, offset, 128);
        }
        else {
            return storage.readText(line, offset, size);
        }
    }

};