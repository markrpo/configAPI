cmake_minimum_required(VERSION 3.10)
project(configapi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# check if clang is used and set the stdlib to libc++ (libc++ is the C++ standard library for clang and by default clang uses libstdc++)
# To compile with cmake use the following command: cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
# In build directory
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()

# Find libpahoC 
find_path(NLOHMANN_JSON_INCLUDE_DIR
	NAMES json.hpp
	PATHS /usr/include/nlohmann /usr/local/include/nlohmann
	)

find_path(SQLITE3_INCLUDE_DIR
	NAMES sqlite3.h
	PATHS /usr/include /usr/local/include
	)

find_library(SQLITE3_LIBRARIES
	NAMES sqlite3
	PATHS /usr/lib /usr/local/lib
	)

find_path(PUGIXML_INCLUDE_DIR
	NAMES pugixml.hpp
	PATHS /usr/include /usr/local/include
	)

find_library(PUGIXML_LIBRARIES
	NAMES pugixml
	PATHS /usr/lib /usr/local/lib
	)

# Check if all libraries are found
if (NLOHMANN_JSON_INCLUDE_DIR)
	message(STATUS "Found nlohmann_json")
	message(STATUS "Include dir: ${NLOHMANN_JSON_INCLUDE_DIR}")
else()
	message(STATUS "Include dir: ${NLOHMANN_JSON_INCLUDE_DIR}")
	message(FATAL_ERROR "nlohmann_json not found")
endif()

if (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
	message(STATUS "Found sqlite3")
	message(STATUS "Include dir: ${SQLITE3_INCLUDE_DIR}")
	message(STATUS "Library: ${SQLITE3_LIBRARIES}")
else()
	message(STATUS "Include dir: ${SQLITE3_INCLUDE_DIR}")
	message(STATUS "Library: ${SQLITE3_LIBRARIES}")
	message(FATAL_ERROR "sqlite3 not found")
endif()

if (PUGIXML_INCLUDE_DIR AND PUGIXML_LIBRARIES)
	message(STATUS "Found pugixml")
	message(STATUS "Include dir: ${PUGIXML_INCLUDE_DIR}")
	message(STATUS "Library: ${PUGIXML_LIBRARIES}")
else()
	message(STATUS "Include dir: ${PUGIXML_INCLUDE_DIR}")
	message(STATUS "Library: ${PUGIXML_LIBRARIES}")
	message(FATAL_ERROR "pugixml not found")
endif()

# Set sources and headers files
set(SOURCES src/json_xml.cpp src/config.cpp)
set(HEADERS src/config.hpp)

# Add executable
add_executable(configapi ${SOURCES} ${HEADERS})

# Include directories PRIVATE means only this target can see the include directories
target_include_directories(configapi PRIVATE include ${NLOHMANN_JSON_INCLUDE_DIR} ${SQLITE3_INCLUDE_DIR} ${PUGIXML_INCLUDE_DIR}) 

# Link libraries
target_link_libraries(configapi PRIVATE  ${NLOHMANN_JSON_LIBRARIES} ${SQLITE3_LIBRARIES} ${PUGIXML_LIBRARIES})
