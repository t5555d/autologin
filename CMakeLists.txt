cmake_minimum_required(VERSION 3.15)

project(AutoLogin)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.rc)
file(GLOB_RECURSE HEADERS src/*.h   etc/*.reg)
file(GLOB_RECURSE EXPORTS src/*.def)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} ${EXPORTS})

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ASSEMBLYMODULE:secur32.lib")

target_link_libraries(${PROJECT_NAME}
    Credui
    Secur32
    Shlwapi
    Ws2_32
    Iphlpapi
)
