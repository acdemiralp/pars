##################################################    Project     ##################################################
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project               (pars VERSION 1.0 LANGUAGES CXX)
list                  (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set_property          (GLOBAL PROPERTY USE_FOLDERS ON)
set                   (CMAKE_CXX_STANDARD 17)
set                   (CMAKE_CXX_VISIBILITY_PRESET hidden)
set                   (CMAKE_VISIBILITY_INLINES_HIDDEN 1)

include               (set_max_warning_level)
set_max_warning_level ()

##################################################    Options     ##################################################
option(BUILD_SHARED_LIBS "Build shared (dynamic) libraries." OFF)
option(BUILD_TESTS "Build tests." OFF)

##################################################    Sources     ##################################################
file(GLOB_RECURSE PROJECT_HEADERS include/*.h include/*.hpp)
file(GLOB_RECURSE PROJECT_SOURCES source/*.c source/*.cpp)
file(GLOB_RECURSE PROJECT_PROTOBUFS proto/*.proto)
file(GLOB_RECURSE PROJECT_CMAKE_UTILS cmake/*.cmake)
file(GLOB_RECURSE PROJECT_MISC *.md *.txt)
set (PROJECT_EXPORT_HPP include/${PROJECT_NAME}/export.hpp)
set (PROJECT_FILES 
  ${PROJECT_HEADERS} 
  ${PROJECT_SOURCES} 
  ${PROJECT_PROTOBUFS} 
  ${PROJECT_CMAKE_UTILS} 
  ${PROJECT_MISC} 
  ${PROJECT_EXPORT_HPP})
  
include            (assign_source_group)
assign_source_group(${PROJECT_FILES})

##################################################  Dependencies  ##################################################
include(import_library)

if            ((CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  list        (APPEND PROJECT_COMPILE_OPTIONS -msse -msse2)
elseif        (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  list        (APPEND PROJECT_COMPILE_OPTIONS /QxSSE /QxSSE2)
elseif        (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  list        (APPEND PROJECT_COMPILE_OPTIONS /arch:SSE /arch:SSE2)
endif         ()

list          (APPEND PROJECT_LIBRARIES pa)

find_package  (MPI REQUIRED)
import_library(MPI_C_INCLUDE_DIRS MPI_C_LIBRARIES)

find_package  (OSPRay REQUIRED)
import_library(OSPRAY_INCLUDE_DIRS OSPRAY_LIBRARIES)

find_package  (Protobuf REQUIRED)
import_library(PROTOBUF_INCLUDE_DIRS PROTOBUF_LIBRARIES)
list          (APPEND PROJECT_COMPILE_DEFINITIONS -DPROTOBUF_USE_DLLS)

find_package  (ZeroMQ REQUIRED)
import_library(ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARIES)

##################################################    Prebuild    ##################################################
if   (PROJECT_PROTOBUFS)
  protobuf_generate_cpp(PROTOBUF_AUTOGEN_SOURCES PROTOBUF_AUTOGEN_HEADERS ${PROJECT_PROTOBUFS})
  assign_source_group  (${PROTOBUF_AUTOGEN_SOURCES} ${PROTOBUF_AUTOGEN_HEADERS})
  list                 (APPEND PROJECT_FILES ${PROTOBUF_AUTOGEN_SOURCES} ${PROTOBUF_AUTOGEN_HEADERS})
endif()

##################################################    Targets     ##################################################
add_library(${PROJECT_NAME} ${PROJECT_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/external>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
  $<INSTALL_INTERFACE:include/external> PRIVATE source)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries     (${PROJECT_NAME} PUBLIC ${PROJECT_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
target_compile_options    (${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_OPTIONS})
set_target_properties     (${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

if(NOT BUILD_SHARED_LIBS)
  string               (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS -D${PROJECT_NAME_UPPER}_STATIC)
endif()

##################################################    Postbuild   ##################################################
include               (GenerateExportHeader)
string                (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
generate_export_header(${PROJECT_NAME} 
  EXPORT_FILE_NAME     ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/export.hpp
  EXPORT_MACRO_NAME    ${PROJECT_NAME_UPPER}_EXPORT
  STATIC_DEFINE        ${PROJECT_NAME_UPPER}_STATIC
)

##################################################    Testing     ##################################################
if(BUILD_TESTS)
  enable_testing     ()
  set                (TEST_MAIN_NAME catch_main)
  set                (TEST_MAIN_SOURCES tests/catch/catch.hpp tests/catch/main.cpp)
  add_library        (${TEST_MAIN_NAME} OBJECT ${TEST_MAIN_SOURCES})
  set_property       (TARGET ${TEST_MAIN_NAME} PROPERTY FOLDER tests/catch)
  assign_source_group(${TEST_MAIN_SOURCES})

  file(GLOB PROJECT_TEST_CPPS tests/*.cpp)
  foreach(_SOURCE ${PROJECT_TEST_CPPS})
    get_filename_component(_NAME ${_SOURCE} NAME_WE)
    add_executable        (${_NAME} ${_SOURCE} $<TARGET_OBJECTS:${TEST_MAIN_NAME}>)
    target_link_libraries (${_NAME} ${PROJECT_NAME})
    add_test              (${_NAME} ${_NAME})
    set_property          (TARGET ${_NAME} PROPERTY FOLDER tests)
    assign_source_group   (${_SOURCE})
  endforeach()
endif()

##################################################  Installation  ##################################################
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-config
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(EXPORT  ${PROJECT_NAME}-config DESTINATION cmake)
export (TARGETS ${PROJECT_NAME}        FILE        ${PROJECT_NAME}-config.cmake)
