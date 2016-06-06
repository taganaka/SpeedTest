cmake_minimum_required(VERSION 2.6)
project(SpeedTest)

set (SpeedTest_VERSION_MAJOR 1)
set (SpeedTest_VERSION_MINOR 0)
set (SpeedTest_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
set (SpeedTest_SYSTEM ${CMAKE_SYSTEM})

set (SpeedTest_AUTHOR "Francesco Laurita <francesco.laurita@gmail.com>")
set (SpeedTest_USER_AGENT "Mozilla/5.0 ${CMAKE_SYSTEM}; U; ${CMAKE_SYSTEM_PROCESSOR}; en-us (KHTML, like Gecko) SpeedTest++/${SpeedTest_VERSION_MAJOR}.${SpeedTest_VERSION_MINOR}")
set (SpeedTest_SERVER_LIST_URL "http://www.speedtest.net/speedtest-servers-static.php")
set (SpeedTest_IP_INFO_API_URL "http://speedtest.ookla.com/api/ipaddress.php")
set (SpeedTest_API_URL "http://www.speedtest.net/api/api.php")
set (SpeedTest_API_REFERER "http://c.speedtest.net/flash/speedtest.swf")
set (SpeedTest_API_KEY "297aae72")


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-result")

set(SOURCE_FILES
        main.cpp
        SpeedTest.cpp
        SpeedTest.h
        SpeedTestClient.cpp
        SpeedTestClient.h
        TestConfigTemplate.h
        MD5Util.cpp MD5Util.h)

configure_file (
        "${PROJECT_SOURCE_DIR}/SpeedTestConfig.h.in"
        "${PROJECT_BINARY_DIR}/SpeedTestConfig.h"
)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(SpeedTest ${SOURCE_FILES})

INCLUDE (CheckIncludeFiles)
find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

if (NOT (APPLE))
    find_package(OpenSSL REQUIRED)
else()
    CHECK_INCLUDE_FILES("CommonCrypto/CommonDigest.h" HAVE_COMMON_DIGEST_H)
endif()

include_directories(${CURL_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(SpeedTest ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES} -lssl -lcrypto)

install(TARGETS SpeedTest RUNTIME DESTINATION bin)