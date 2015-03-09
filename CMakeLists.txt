cmake_minimum_required(VERSION 2.8.4)
project(uvpp-http)

include(cmake/FindLibUV.cmake)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

add_subdirectory(thirdparty/http-parser)

FILE(GLOB INCLUDE_FILES "http/*.hpp")

INCLUDE_DIRECTORIES(
		"thirdparty/uvpp/"
		"thirdparty/http-parser/"
        ${INCLUDE_FILES}       
        ${LIBUV_INCLUDE_DIRS}
)


SET(LIBS ${LIBS} )


add_executable(server "webserver.cpp")

target_link_libraries (server http-parser ${LIBUV_LIBRARIES})
