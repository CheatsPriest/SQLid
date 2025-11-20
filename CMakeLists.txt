cmake_minimum_required(VERSION 3.8)

project("SQLid")

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Автоопределение платформы
if(WIN32)
    message(STATUS "Building for Windows")
    set(SQLID_WINDOWS 1)
    
    # MSVC specific settings
    if(MSVC)
        if(POLICY CMP0141)
            cmake_policy(SET CMP0141 NEW)
            set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
        endif()
        
        # Подавить предупреждения для MSVC
        add_compile_options(/wd4267 /wd4018 /wd4715)
    endif()
    
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
    set(SQLID_LINUX 1)
endif()

# Поиск Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_DEBUG ON)  # Для отладки поиска Boost

if(SQLID_WINDOWS)
    # Windows-specific Boost paths
    set(BOOST_ROOT "C:/boost_1_89_0")
    set(Boost_DIR "C:/boost_1_89_0/lib/cmake/Boost-1.89.0")
    
    # Помоги CMake найти Boost
    set(BOOST_LIBRARYDIR "C:/boost_1_89_0/lib")
    set(BOOST_INCLUDEDIR "C:/boost_1_89_0")
endif()

# Ищем только JSON (system обычно подтягивается автоматически)
find_package(Boost REQUIRED COMPONENTS json)

# Исходные файлы
add_executable(SQLid 
    "src/SQLid.cpp" 
    "src/SQLid.h"   
    "src/files/MemoryMap.h" 
    "src/files/MemoryMap.cpp" 
    "src/files/Tabble.h" 
    "src/files/InfoLoader.h" 
    "src/parser/RequestParser.h" 
    "src/parser/Query.h" 
    "src/parser/Conditions.h" 
    "src/supported_variants.h" 
    "src/parser/Conditions.cpp" 
    "src/data_base/DataBase.h"  
    "src/result/Result.h" 
    "src/files/Optimizer.h" 
    "src/data_base/Executor.h"  
    "src/system/system.h" 
    "src/clients/Operator.h" 
    "src/clients/Client.h" 
    "src/clients/SyncServer.h" 
    "src/clients/Base.h" 
    "src/exceptions/Exceptions.h"
    "src/parser/CommandsParser.h" 
    "src/parser/Commands.h" 
    "src/data_base/CommandsExecutor.h"
    "include/SQLid_API.h"
)

# Платформо-специфичные файлы
if(SQLID_WINDOWS)
    target_sources(SQLid PRIVATE "src/files/MemoryMapWindows.h")
elseif(SQLID_LINUX)
    target_sources(SQLid PRIVATE "src/files/MemoryMapLinux.h")
endif()

# Директории include
target_include_directories(SQLid 
    PRIVATE src 
    PUBLIC include
    PUBLIC ${Boost_INCLUDE_DIRS}
)

# Линковка
target_link_libraries(SQLid PRIVATE 
    Boost::json
    # Boost::system обычно не нужен явно для json
)

# Платформо-специфичные библиотеки
if(SQLID_WINDOWS)
    target_link_libraries(SQLid PRIVATE ws2_32)
elseif(SQLID_LINUX)
    target_link_libraries(SQLid PRIVATE pthread)
endif()

# Для Linux в WSL - дополнительные флаги
if(SQLID_LINUX)
    target_compile_options(SQLid PRIVATE -pthread)
    target_link_options(SQLid PRIVATE -pthread)
endif()