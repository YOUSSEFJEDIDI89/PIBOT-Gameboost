cmake_minimum_required(VERSION 3.16)
project(pibot VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB PIBOT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(pibot ${PIBOT_SOURCES})

# libcurl اختياري: إن وُجد نستخدمه، وإلا نستخدم أمر curl تلقائياً
find_package(CURL QUIET)
if(CURL_FOUND)
    message(STATUS "libcurl found - using native HTTP")
    target_compile_definitions(pibot PRIVATE PIBOT_USE_LIBCURL)
    target_link_libraries(pibot PRIVATE CURL::libcurl)
else()
    message(STATUS "libcurl not found - falling back to curl binary")
endif()

install(TARGETS pibot RUNTIME DESTINATION bin)
