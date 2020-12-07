# CMakeList.txt : CMake project for utility, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} /MTd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} /MT /O2 /Ob2 /DNDEBUG")
    if (NOT CMAKE_BUILD_TYPE)
		message("not defined Build Type:auto define it to Debug" )
		set (CMAKE_BUILD_TYPE Debug)
	endif (NOT CMAKE_BUILD_TYPE)
    if (CMAKE_BUILD_TYPE MATCHES "Debug" )
	    set(LIBRARY_OUTPUT_PATH ./dist/lib/windows)
    elseif (CMAKE_BUILD_TYPE MATCHES "Release" )
        set(LIBRARY_OUTPUT_PATH ./dist/lib/windows)
    elseif (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
        set(LIBRARY_OUTPUT_PATH ./dist/lib/windows/MinSizeRel)
    elseif (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
        set(LIBRARY_OUTPUT_PATH ./dist/lib/windows/RelWithDebInfo)
    else()
	    message("unkown defined Build Type:same handle with Debug" )
	    set(LIBRARY_OUTPUT_PATH ./dist/lib/windows)
	endif (CMAKE_BUILD_TYPE MATCHES "Debug")
elseif(UNIX)
	if (NOT CMAKE_BUILD_TYPE)
		message("not defined Build Type:auto define it to Debug" )
		set (CMAKE_BUILD_TYPE Debug)
	endif (NOT CMAKE_BUILD_TYPE)

	if (CMAKE_BUILD_TYPE MATCHES "Debug" )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -fPIC ")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g -ggdb -fPIC ")
		set(LIBRARY_OUTPUT_PATH ./dist/lib/linux/Debug)
	elseif (CMAKE_BUILD_TYPE MATCHES "Release" )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -DNDEBUG -fPIC ")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -DNDEBUG -fPIC ")
		set(LIBRARY_OUTPUT_PATH ./dist/lib/linux/Release)
    elseif (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -Wall -fPIC ")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -Wall -fPIC ")
		set(LIBRARY_OUTPUT_PATH ./dist/lib/linux/MinSizeRel)
	elseif (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fPIC ")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2 -fPIC ")
		set(LIBRARY_OUTPUT_PATH ./dist/lib/linux/RelWithDebInfo)
	else()
		message("unkown defined Build Type:same handle with Debug" )
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -fPIC ")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -Wall -g -ggdb -fPIC ")
		set(LIBRARY_OUTPUT_PATH ./dist/lib/linux/Debug)
	endif (CMAKE_BUILD_TYPE MATCHES "Debug")
endif(WIN32)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (utility
VERSION 1.2020.1116.0
DESCRIPTION 公共帮助类
LANGUAGES CXX)
if(UNIX)
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif(UNIX)

include(vVersion.cmake)

set(thirdparty ../thirdparty)

include_directories( ${thirdparty}/hiredis/include)
if(UNIX)
	include_directories( ${thirdparty}/libevent/include)
endif(UNIX)

include_directories(${PROJECT_SOURCE_DIR}/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)

if(WIN32)
	link_directories( ${thirdparty}/hiredis/lib/windows)
elseif(UNIX)
	link_directories( ${thirdparty}/libevent/lib)
	link_directories( ${thirdparty}/hiredis/lib/linux)
endif(WIN32)

if(BUILD_SHARED_LIBS)
	add_library(${CMAKE_PROJECT_NAME} SHARED ${SRC_LIST})
else()
	add_library (${CMAKE_PROJECT_NAME} STATIC ${SRC_LIST})
endif(BUILD_SHARED_LIBS)

if(WIN32)
	target_link_libraries(${CMAKE_PROJECT_NAME}
hiredis)
elseif(UNIX)
	target_link_libraries(${CMAKE_PROJECT_NAME}
rt
pthread
hiredis
event)
endif(WIN32)

aux_source_directory(${PROJECT_SOURCE_DIR}/test test_LIST)

add_executable (${CMAKE_PROJECT_NAME}_test ${test_LIST})
target_link_libraries(${CMAKE_PROJECT_NAME}_test ${CMAKE_PROJECT_NAME})
# TODO: Add tests and install targets if needed.
#cmake 编译命令
#linux
#release   cmake -DCMAKE_BUILD_TYPE=Release ..
#debug     cmake -DCMAKE_BUILD_TYPE=Debug ..

#windows
#debug
#cmake -DCMAKE_BUILD_TYPE=Debug ..
#cmake --build .\ --config Debug
#release
#cmake -DCMAKE_BUILD_TYPE=Release ..
#cmake --build .\ --config Release