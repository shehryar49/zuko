cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)
project(plutonium)
set(CMAKE_CXX_STANDARD 11)
add_executable(plutonium plutonium.cpp)
target_include_directories(plutonium PUBLIC include)
target_link_libraries(plutonium -ldl)
message("BUILD TYPE = ${CMAKE_BUILD_TYPE}")
#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")
# Set optimization flags for release build
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #For MSVC we stick to the default debug and release flags
else()
    #for Clang or GCC 
    if(CMAKE_BUILD_TYPE STREQUAL "Release") 
      set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -w")
    else()
      set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
    endif()
endif()
#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")


# Add modules
set(CMAKE_SHARED_LIBRARY_PREFIX "")
# math
add_library(math SHARED modules/math/math.cpp modules/math/math.h)
target_include_directories(math PUBLIC include)
set_target_properties(math PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# json
add_library(json SHARED modules/json/json.cpp modules/json/json.h)
target_include_directories(json PUBLIC include)
set_target_properties(json PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# regex
add_library(regex SHARED modules/regex/regex.cpp modules/regex/regex.h)
target_include_directories(regex PUBLIC include)
set_target_properties(regex PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# datetime
add_library(datetime SHARED modules/datetime/datetime.cpp modules/datetime/datetime.h)
target_include_directories(datetime PUBLIC include)
set_target_properties(datetime PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)

# linux specific modules
add_library(process SHARED modules/process/process.cpp modules/process/process.h)
target_include_directories(process PUBLIC include)
set_target_properties(process PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)

# windows specific modules
if(WIN32)
  # conio
  add_library(conio SHARED modules/conio/conio.cpp modules/conio/conio.h)
  target_include_directories(conio PUBLIC include)
  set_target_properties(conio PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
  # console
  add_library(console SHARED modules/console/console.cpp modules/console/console.h)
  target_include_directories(console PUBLIC include)
  set_target_properties(console PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  
endif()

# Third party library bindings

# libcurl
find_package(CURL REQUIRED) 
add_library(libcurl SHARED modules/libcurl/libcurl.cpp modules/libcurl/libcurl.h)
target_include_directories(libcurl PUBLIC include)
target_link_libraries(libcurl ${CURL_LIBRARIES})
set_target_properties(libcurl PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  
