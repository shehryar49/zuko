cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)
project(zuko)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable( 
  zuko 
  zuko.cpp 
  src/vm.cpp 
  src/parser.cpp 
  src/lexer.c
  src/builtinfunc.cpp 
  src/utility.cpp 
  src/overflow.cpp 
  src/convo.cpp  
  src/compiler.cpp 
  src/repl.cpp 
  src/zobject.c 
  src/zdict.c 
  src/zlist.c 
  src/strmap.c 
  src/zbytearray.c 
  src/module.c 
  src/klass.c 
  src/klassobject.c  
  src/dis.cpp 
  src/refgraph.c  
  src/zuko-src.c 
  src/lntable.c 
  src/str-vec.c 
  src/token.c 
  src/dyn-str.c 
  src/token-vector.c
)
include_directories(include/) # even modules use this (include/zobject.h)
if(UNIX)
  target_link_libraries(zuko ${CMAKE_DL_LIBS})
  target_link_libraries(zuko -lreadline) # GNU readline
endif()

if(MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "/MT")
  set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
endif()

add_executable(test_all tests/test_all.cpp)
set_target_properties(test_all PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests/)
message("BUILD TYPE = ${CMAKE_BUILD_TYPE}")
#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")

#message("${CMAKE_CXX_FLAGS_RELEASE}")
#message("${CMAKE_CXX_FLAGS_DEBUG}")
include_directories(include/)


# static library (extension api) for linking to modules

add_library(zapi STATIC src/zobject.c src/zdict.c src/zlist.c src/zapi.c src/strmap.c src/zbytearray.c src/module.c src/klass.c src/klassobject.c)
set_property(TARGET zapi PROPERTY POSITION_INDEPENDENT_CODE ON)
link_directories(${CMAKE_SOURCE_DIR})
# Add modules

set(CMAKE_SHARED_LIBRARY_PREFIX "")

# math
add_library(math SHARED modules/math/math.cpp modules/math/math.h)
set_target_properties(math PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)

target_link_libraries(math zapi)

# json
add_library(json SHARED modules/json/json.cpp modules/json/json.h)
set_target_properties(json PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(json zapi)
# regex
add_library(regex SHARED modules/regex/regex.cpp modules/regex/regex.h)
set_target_properties(regex PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(regex zapi)
# datetime
add_library(datetime SHARED modules/datetime/datetime.cpp modules/datetime/datetime.h)
set_target_properties(datetime PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(datetime zapi)
# bignum
add_library(bignum SHARED modules/bignum/bignumWrapper.cpp modules/bignum/bignum.cpp modules/bignum/bignum.h)
set_target_properties(bignum PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(bignum zapi)
# cgi
add_library(cgi SHARED modules/cgi/cgi.cpp modules/cgi/cgi.h modules/cgi/multipart.cpp modules/cgi/cgi-utils.cpp)
set_target_properties(cgi PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(cgi zapi)
# socket
add_library(socket SHARED modules/socket/socket.cpp modules/socket/socket.h)
set_target_properties(socket PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(socket zapi)
# base64
add_library(base64 SHARED modules/base64/base64.cpp modules/base64/base64.h)
set_target_properties(base64 PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
target_link_libraries(base64 zapi)

# linux specific modules
if(UNIX)
  add_library(process SHARED modules/process/process.cpp modules/process/process.h)
  set_target_properties(process PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
  target_link_libraries(process zapi)
endif()
# windows specific modules
if(WIN32)
  # conio
  # add_library(conio SHARED modules/conio/conio.cpp modules/conio/conio.h)
  # set_target_properties(conio PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)
  # console
  # add_library(console SHARED modules/console/console.cpp modules/console/console.h)
  # set_target_properties(console PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  
endif()

# Third party library bindings

# libcurl
find_package(CURL REQUIRED) 
add_library(libcurl SHARED modules/libcurl/libcurl.cpp modules/libcurl/libcurl.h modules/libcurl/curl.h modules/libcurl/curl.cpp modules/libcurl/mime.h modules/libcurl/mime.cpp modules/libcurl/mimepart.h modules/libcurl/mimepart.cpp modules/libcurl/curl_slist.cpp)
target_include_directories(libcurl PUBLIC include)
target_link_libraries(libcurl ${CURL_LIBRARIES})
set_target_properties(libcurl PROPERTIES LIBRARY_OUTPUT_DIRECTORY modules/)  
target_link_libraries(libcurl zapi)
