#pragma once
#include <windows.h>
#include <string>
#include <stdexcept>
#include <iostream>

class MemoryMapWindows {
private:
	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hMap = NULL;
	void* mappedData = nullptr;
	size_t fileSize = 0;


	size_t lineLength = 0;

	
	

public:
	size_t getMaxLine() const {
		return fileSize / lineLength;
	}
	bool expandFile() {
		// 1. Закрыть mapping
		UnmapViewOfFile(mappedData);
		CloseHandle(hMap);

		// 2. Увеличить размер файла с учетом lineLength
		size_t newFileSize = fileSize * 2;
		SetFilePointer(hFile, newFileSize, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);

		// 3. Пересоздать mapping
		hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, newFileSize, NULL);
		mappedData = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, newFileSize);

		fileSize = newFileSize; // Не забыть обновить!

		std::cout << "Expanded by " << fileSize << std::endl;

		return mappedData != nullptr;
	}

	MemoryMapWindows(const std::string& filename, size_t lineLength)
		: lineLength(lineLength) {
		openFile(filename);
	}

	~MemoryMapWindows() {
		close();
	}

	// Открыть файл
	bool openFile(const std::string& filename) {
		close(); // Закрыть предыдущий

		// Открываем файл
		hFile = CreateFileA(filename.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,  // Открыть если существует, иначе создать
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Cannot open file: " + filename);
		}

		// Получаем размер файла
		fileSize = GetFileSize(hFile, NULL);

		// Если файл новый/пустой - устанавливаем начальный размер
		if (fileSize <= lineLength) {
			fileSize = lineLength * 100; // Начальный размер: 100 строк
			SetFilePointer(hFile, fileSize, NULL, FILE_BEGIN);
			SetEndOfFile(hFile);
		}

		// Создаем memory mapping
		hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
		if (!hMap) {
			CloseHandle(hFile);
			throw std::runtime_error("CreateFileMapping failed");
		}

		// Отображаем в память
		mappedData = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, fileSize);
		if (!mappedData) {
			CloseHandle(hMap);
			CloseHandle(hFile);
			throw std::runtime_error("MapViewOfFile failed");
		}




		return true;
	}

	// Найти указатель на строку по номеру (O(n) - потом оптимизируешь)
	char* findLine(size_t lineNumber) {

		if (!mappedData) return nullptr;

		char* data = static_cast<char*>(mappedData);
		char* current = data + lineNumber * lineLength;
		size_t currentLine = 0;



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



		//size_t maxLength = min(lineLength, fileSize - (lineStart - static_cast<char*>(mappedData)));
		//size_t actualLength = strnlen(lineStart, maxLength);

		return std::string_view(lineStart, size); 
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

	// Чтение любого числа
	template<typename T>
	T readNumber(size_t lineNumber, size_t offset = 0) {
		static_assert(std::is_arithmetic_v<T>, "T must be numeric");
		T value;
		char* lineStart = findLine(lineNumber) + offset;
		memcpy(&value, lineStart, sizeof(T));
		return value;
	}

	// Добавить строку в коне

	// Закрыть файл
	void close() {
		if (mappedData) {
			UnmapViewOfFile(mappedData);
			mappedData = nullptr;
		}
		if (hMap) {
			CloseHandle(hMap);
			hMap = NULL;
		}
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}
		fileSize = 0;
	}

	// Получить размер файла
	size_t getFileSize() const { return fileSize; }

	// Получить количество строк (O(n) - потом оптимизируешь)
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