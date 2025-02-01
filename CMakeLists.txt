cmake_minimum_required(VERSION 3.10)
project(MyCrowApp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find asio header
find_path(ASIO_INCLUDE_DIR asio.hpp PATHS /usr/include /usr/local/include third_party/asio/include)

if(NOT ASIO_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find asio.hpp")
endif()

# Download and compile crow_all.hpp
add_custom_command(
OUTPUT ${CMAKE_SOURCE_DIR}/include/crow_all.hpp  
COMMAND make compile_crow               
WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
COMMENT "Generating crow_all.hpp"
)

# Add a custom target to make sure the crow_all.hpp is generated before the executable
add_custom_target(
	generate_crow
	DEPENDS ${CMAKE_SOURCE_DIR}/include/crow_all.hpp 
)

# Set sources and headers files
set(SOURCES src/api.cpp src/routes.cpp)
set(HEADERS src/routes.hpp include/crow_all.hpp)

# Add executable
add_executable(MyCrowApp ${SOURCES} ${HEADERS})

# Include directories PRIVATE means only this target can see the include directories
target_include_directories(MyCrowApp PRIVATE include ${ASIO_INCLUDE_DIR}) 

# Link libraries
target_link_libraries(MyCrowApp)
