cmake_minimum_required(VERSION 3.0)

project(offsetAllocator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
   offsetAllocator.cpp
   offsetAllocator.hpp
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC .)

Include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/external")

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(OffsetAllocatorTests offsetAllocatorTests.cpp)
target_link_libraries(OffsetAllocatorTests PRIVATE ${PROJECT_NAME} Catch2::Catch2WithMain)
