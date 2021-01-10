
if(NOT WIN32)
  find_package(Threads)
  target_link_libraries(${PROJECT_NAME} INTERFACE ${CMAKE_THREAD_LIBS_INIT})
endif()

add_executable       (ip_address "ip_address.cpp")
target_link_libraries(ip ip-sockets-cpp-lite)

add_executable       (udp_socket "udp_socket.cpp")
target_link_libraries(udp_socket  ip-sockets-cpp-lite)

add_executable       (tcp_socket "tcp_socket.cpp")
target_link_libraries(tcp_socket  ip-sockets-cpp-lite)

add_executable       (resolve_host "resolve_host.cpp")
target_link_libraries(resolve_host  ip-sockets-cpp-lite)
