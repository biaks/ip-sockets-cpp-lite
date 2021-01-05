
cmake_minimum_required(VERSION 3.12)
project(ip-sockets-cpp-lite)

option(IP_SOCKETS_CPP_LITE_BUILD_EXAMPLES "Build with examples" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

if(WIN32) 

  # it prevent create Debug/ and Release folders in Visual Studio
  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin )
  endforeach()

  set (INSTALL_PATH_BIN "${PROJECT_SOURCE_DIR}/installed/bin/")

else() # not WIN32

  set (INSTALL_PATH_BIN "bin/")

endif()

# include all header files from current directory

file(GLOB_RECURSE IP_SOCKETS_CPP_LITE_HEADERS *.h)

# it is header only target

add_library               (${PROJECT_NAME}     INTERFACE)
target_sources            (${PROJECT_NAME}     INTERFACE ${IP_SOCKETS_CPP_LITE_HEADERS})
target_include_directories(${PROJECT_NAME}     INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_custom_target         (${PROJECT_NAME}-ide SOURCES   ${IP_SOCKETS_CPP_LITE_HEADERS})

## If this project is added as a subdirectory of another project, export the
## include path to the parent scope. When this is the top-level project
## (used standalone), don't use PARENT_SCOPE to avoid warnings.

if(NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(${PROJECT_NAME}_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
else()
  set(${PROJECT_NAME}_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# include examples if the option is enabled

if(IP_SOCKETS_CPP_LITE_BUILD_EXAMPLES)
  add_subdirectory("./examples")
endif()
