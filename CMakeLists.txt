cmake_minimum_required(VERSION 3.5)
project(SpeedTest)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES main.cpp SpeedTest.cpp SpeedTest.h SpeedTestClient.cpp SpeedTestClient.h)
add_executable(SpeedTest ${SOURCE_FILES})

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

include_directories(${CURL_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR})
target_link_libraries(SpeedTest ${CURL_LIBRARIES} ${LIBXML2_LIBRARIES})