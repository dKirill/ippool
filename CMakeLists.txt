cmake_minimum_required(VERSION 3.7)
project(ippool CXX)

set(CMAKE_CXX_STANDARD 17)

# set flags
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wenum-compare -Wfloat-equal -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wno-unused-function")

# create static lib
set(SOURCES 
	src/ippool.cpp)
set(HEADERS 
	include/ippool.hpp)

add_library(ippool STATIC ${SOURCES} ${HEADERS})
target_include_directories(ippool PUBLIC include)

set_target_properties(ippool PROPERTIES PUBLIC_HEADER ${HEADERS})

# Prepare "Catch" unit test library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/extern/Catch2/single_include)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# Add test executable
set(TEST_SOURCES 
	tests/Test.cpp)
	
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch ippool)

# install
install(TARGETS ippool
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)
