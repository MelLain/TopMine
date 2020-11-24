cmake_minimum_required(VERSION 3.0.2)

project(TopMine)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost COMPONENTS
  filesystem
  thread
  system
  program_options
REQUIRED)

include_directories(${Boost_INCLUDE_DIRS}) 

set(CMAKE_CXX_COMPILER "g++")

include_directories(./include)
include_directories(.)

set(SOURCE_LIB
  src/batch.cc
  src/collection_processor.cc
  src/collocations_processor.cc
  src/heap.cc
  src/scoring_processor.cc
  src/spinlock.cc
  src/thread_safe_collocation_start_indices.cc
  src/thread_safe_counters.cc
  src/thread_safe_dictionary.cc
  src/token_counters_processor.cc
  src/topmine_impl.cc
  src/utils.cc
)

set(CMAKE_CXX_STANDARD 11)

set(CXX_STANDARD_REQUIRED)

add_library(topmine_lib STATIC ${SOURCE_LIB})

add_executable(topmine src/topmine.cc)

target_link_libraries(
  topmine
  topmine_lib
  ${Boost_LIBRARIES}
)

add_executable(topmine_tests tests/topmine_tests.cc)

target_link_libraries(
  topmine_tests
  topmine_lib
  gtest
  gtest_main
  ${Boost_LIBRARIES}
)
