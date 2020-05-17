
add_executable       (ip "ip.cpp")
target_link_libraries(ip ip-sockets-cpp-lite)

add_executable       (udp_socket "udp_socket.cpp")
target_link_libraries(udp_socket  ip-sockets-cpp-lite)

add_executable       (tcp_socket "tcp_socket.cpp")
target_link_libraries(tcp_socket  ip-sockets-cpp-lite)
