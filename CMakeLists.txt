cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)
project(zuko)
set(CMAKE_CXX_STANDARD 11)
add_executable(zuko zuko.cpp)
include_directories(include/) # even modules use this (include/PltObject.h)
if(UNIX)
  target_link_libraries(zuko ${CMAKE_DL_LIBS})
  target_link_libraries(zuko -lreadline) # GNU readline
endif()

add_executable(test_all tests/test_all.cpp)
set_target_properties(test_all PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests/)
message("BUILD TYPE = ${CMAKE_BUILD_TYPE}")
#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")

#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")
include_directories(include/)

# Add modules
set(CMAKE_SHARED_LIBRARY_PREFIX "")
# math
add_library(math SHARED modules/math/math.cpp modules/math/math.h)
set_target_properties(math PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# json
add_library(json SHARED modules/json/json.cpp modules/json/json.h)
set_target_properties(json PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# regex
add_library(regex SHARED modules/regex/regex.cpp modules/regex/regex.h)
set_target_properties(regex PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# datetime
add_library(datetime SHARED modules/datetime/datetime.cpp modules/datetime/datetime.h)
set_target_properties(datetime PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# bignum
add_library(bignum SHARED modules/bignum/bignumWrapper.cpp modules/bignum/bignum.cpp modules/bignum/bignum.h)
set_target_properties(bignum PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# cgi
add_library(cgi SHARED modules/cgi/cgi.cpp modules/cgi/cgi.h)
set_target_properties(cgi PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
# socket
add_library(socket SHARED modules/socket/socket.cpp modules/socket/socket.h)
set_target_properties(socket PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)


# linux specific modules
if(UNIX)
  add_library(process SHARED modules/process/process.cpp modules/process/process.h)
  set_target_properties(process PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
endif()
# windows specific modules
if(WIN32)
  # conio
  add_library(conio SHARED modules/conio/conio.cpp modules/conio/conio.h)
  set_target_properties(conio PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
  # console
  add_library(console SHARED modules/console/console.cpp modules/console/console.h)
  set_target_properties(console PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  
endif()

# Third party library bindings

# libcurl
find_package(CURL REQUIRED) 
add_library(libcurl SHARED modules/libcurl/libcurl.cpp modules/libcurl/libcurl.h)
target_include_directories(libcurl PUBLIC include)
target_link_libraries(libcurl ${CURL_LIBRARIES})
set_target_properties(libcurl PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  

