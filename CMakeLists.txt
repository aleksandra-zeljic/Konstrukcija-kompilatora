cmake_minimum_required(VERSION 3.13.4)
project(OurAggressiveDCE)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "Putanja do LLVM instalacije")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Pronadjen LLVM ${LLVM_PACKAGE_VERSION}")

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(OurAggressiveDCE SHARED
  OurAggressiveDCEPass.cpp
  OurPostDominatorTree.cpp
  )

target_link_libraries(OurAggressiveDCE
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")
