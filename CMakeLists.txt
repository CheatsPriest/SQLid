cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("SQLid")

# Найти Boost с компонентом JSON
set(BOOST_ROOT "C:/boost_1_89_0")
set(Boost_DIR "C:/boost_1_89_0/lib/cmake/Boost-1.89.0")
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS json)

# Добавить исполняемый файл
add_executable (SQLid 
    "src/SQLid.cpp" 
    "src/SQLid.h"   
    "src/files/MemoryMap.h" 
    "src/files/MemoryMap.cpp" 
    "src/files/MemoryMapWindows.h" 
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
)

# Директории include
target_include_directories(SQLid PRIVATE src)

# Линковка с Boost
target_link_libraries(SQLid PRIVATE 
    Boost::json
    ws2_32  # Для сетевых функций Windows, если используешь
)

# Стандарт C++
set_property(TARGET SQLid PROPERTY CXX_STANDARD 23)

# Подавить некоторые предупреждения для MSVC
if (MSVC)
    target_compile_options(SQLid PRIVATE /wd4267 /wd4018 /wd4715)
endif()