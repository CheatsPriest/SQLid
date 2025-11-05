# CMakeList.txt: проект CMake для SQLid; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SQLid")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (SQLid "src/SQLid.cpp" "src/SQLid.h"   "src/files/MemoryMap.h" "src/files/MemoryMap.cpp" "src/files/MemoryMapWindows.h" "src/files/Tabble.h" "src/files/InfoLoader.h" "src/parser/RequestParser.h" "src/parser/Query.h" "src/parser/Conditions.h" "src/supported_variants.h" "src/parser/Conditions.cpp" "src/data_base/DataBase.h"  "src/result/Result.h" "src/files/Optimizer.h" "src/data_base/Executor.h"  "src/system/system.h")

target_include_directories(SQLid PRIVATE src)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET SQLid PROPERTY CXX_STANDARD 23)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
