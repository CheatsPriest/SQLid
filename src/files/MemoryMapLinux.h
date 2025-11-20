#pragma once


#include <string>
#include <stdexcept>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

class MemoryMapLinux {
private:
    int fd = -1;
    void* mappedData = nullptr;
    size_t fileSize = 0;
    size_t lineLength = 0;

public:
    size_t getMaxLine() const {
        return fileSize / lineLength;
    }

    bool expandFile() {
        // 1. Закрыть mapping
        if (mappedData) {
            munmap(mappedData, fileSize);
        }

        // 2. Увеличить размер файла с учетом lineLength
        size_t newFileSize = fileSize * 2;
        if (ftruncate(fd, newFileSize) == -1) {
            throw std::runtime_error("ftruncate failed");
        }

        // 3. Пересоздать mapping
        mappedData = mmap(nullptr, newFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mappedData == MAP_FAILED) {
            throw std::runtime_error("mmap failed");
        }

        fileSize = newFileSize;
        std::cout << "Expanded by " << fileSize << std::endl;

        return mappedData != MAP_FAILED;
    }

    MemoryMapLinux(const std::string& filename, size_t lineLength)
        : lineLength(lineLength) {
        openFile(filename);
    }

    ~MemoryMapLinux() {
        close();
    }

    // Открыть файл
    bool openFile(const std::string& filename) {
        close(); // Закрыть предыдущий

        // Открываем файл
        fd = open(filename.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd == -1) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        // Получаем размер файла
        struct stat st;
        if (fstat(fd, &st) == -1) {
            close();
            throw std::runtime_error("fstat failed");
        }
        fileSize = st.st_size;

        // Если файл новый/пустой - устанавливаем начальный размер
        if (fileSize <= lineLength) {
            fileSize = lineLength * 100; // Начальный размер: 100 строк
            if (ftruncate(fd, fileSize) == -1) {
                close();
                throw std::runtime_error("ftruncate failed");
            }
        }

        // Создаем memory mapping
        mappedData = mmap(nullptr, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mappedData == MAP_FAILED) {
            close();
            throw std::runtime_error("mmap failed");
        }

        return true;
    }

    // Найти указатель на строку по номеру
    char* findLine(size_t lineNumber) {
        if (!mappedData) return nullptr;

        char* data = static_cast<char*>(mappedData);
        char* current = data + lineNumber * lineLength;
        return (current < data + fileSize) ? current : nullptr;
    }

    void writeText(size_t lineNumber, size_t offset, std::string_view value) {
        char* lineStart = findLine(lineNumber) + offset;

        if (value.length() > lineLength - offset) {
            throw std::runtime_error("Text is too long for the field");
        }

        memcpy(lineStart, value.data(), value.length());
    }

    std::string_view readText(size_t lineNumber, size_t offset, size_t size) {
        char* lineStart = findLine(lineNumber) + offset;
        if (!lineStart) return std::string_view();

        size_t actual_length = 0;
        while (actual_length < size && lineStart[actual_length] != '\0') {
            actual_length++;
        }

        return std::string_view(lineStart, actual_length);
    }

    template<typename T>
    void writeNumber(size_t lineNumber, size_t offset, T value) {
        static_assert(std::is_arithmetic_v<T>, "T must be numeric");

        char* lineStart = findLine(lineNumber) + offset;

        if (sizeof(value) > lineLength - offset) {
            throw std::runtime_error("Number is too long for the field");
        }

        memcpy(lineStart, &value, sizeof(T));
    }

    template<typename T>
    T readNumber(size_t lineNumber, size_t offset = 0) {
        T value;
        char* lineStart = findLine(lineNumber) + offset;
        memcpy(&value, lineStart, sizeof(T));
        return value;
    }

    // Закрыть файл
    void close() {
        if (mappedData && mappedData != MAP_FAILED) {
            munmap(mappedData, fileSize);
            mappedData = nullptr;
        }
        if (fd != -1) {
            ::close(fd);
            fd = -1;
        }
        fileSize = 0;
    }

    // Получить размер файла
    size_t getFileSize() const { return fileSize; }

    // Получить количество строк
    size_t getLineCount() const {
        if (!mappedData) return 0;

        size_t count = 0;
        char* data = static_cast<char*>(mappedData);
        char* current = data;

        while (current < data + fileSize) {
            if (*current == '\n') count++;
            current++;
        }

        return count;
    }
};