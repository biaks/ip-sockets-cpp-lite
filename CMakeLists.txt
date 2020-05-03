
cmake_minimum_required(VERSION 3.2)
project(ip-sockets-cpp-lite)

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

# include examples if the option is enabled

add_subdirectory("./examples")
