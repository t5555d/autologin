cmake_minimum_required(VERSION 3.15)

project(AutoLogin)

set(SRC_DIR samplehardwareeventcredentialprovider)

file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp ${SRC_DIR}/*.rc)
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h   ${SRC_DIR}/*.reg)
file(GLOB_RECURSE EXPORTS ${SRC_DIR}/*.def)

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
