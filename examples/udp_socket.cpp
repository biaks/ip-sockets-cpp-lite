
#include "udp_socket.h"
#include <thread>
#include <chrono>

#if 1
static const ip_type_e cfg_ip_type = v4;
using                  cfg_addr_t  = addr4_t;
static const addr4_t   cfg_server  = "127.0.0.1:2000";
static const addr4_t   cfg_client  = "127.0.0.1:2000";
#else
static const ip_type_e cfg_ip_type = v6;
using                  cfg_addr_t  = addr6_t;
static const addr6_t   cfg_server  = "[::1]:2000";
static const addr6_t   cfg_client  = "[::1]:2000";
#endif

void server_func () {
  using udp_server_t = udp_socket_t<cfg_ip_type, socket_type_e::server>;
  udp_server_t sock (udp_server_t::log_e::debug);
  if (sock.open (cfg_server) == udp_server_t::no_error) {
    cfg_addr_t client_addr;
    char   buf[1000];
    while (true) {
      int res = sock.recvfrom (buf, 1000, client_addr);
      if      (res == udp_server_t::error_timeout) {
        // nothing
      }
      else if (res == udp_server_t::error_unreachable) {
        // nothing
      }
      else if (res >= 0) {
        // success
        printf ("%s\n", buf);
        sock.sendto ("answer\0", 7, client_addr);
      }
    }
  }
}

void client_func () {
  using udp_client_t = udp_socket_t<cfg_ip_type, socket_type_e::client>;
  udp_client_t sock (udp_client_t::log_e::debug);
  if (sock.open (cfg_client) == udp_client_t::no_error) {

    for (size_t i = 0; i < 2; i++) {
      std::this_thread::sleep_for (std::chrono::seconds (1));
      if (sock.send ("hello\0", 6) >= 0) {
        char buf[1000];
        int  res = sock.recv (buf, 1000);
        if      (res == udp_client_t::error_timeout) {
          // nothing
        }
        else if (res == udp_client_t::error_unreachable) {
          // nothing
        }
        else if (res >= 0) {
          // success
          printf ("%s\n", buf);
        }
      }
    }

  }
}

int main () {

  std::thread server (server_func);
  std::thread client (client_func);

  server.join ();
  client.join ();

  return 0;

}
