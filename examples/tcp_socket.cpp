
#include "tcp_socket.h"
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

using tcp_server_t = tcp_socket_t<cfg_ip_type, socket_type_e::server>;
using tcp_client_t = tcp_socket_t<cfg_ip_type, socket_type_e::client>;

void accepted_client_func (tcp_client_t accepted_client) {

  while (true) {
    char buf[1000];
    int res = accepted_client.recv (buf, 1000);
    if (res == tcp_server_t::error_timeout) {
      // nothing
    }
    else if (res == tcp_server_t::error_unreachable) {
      // nothing
    }
    else if (res == 0) {
      // closed by client
      return;
    }
    else if (res > 0) {
      // success
      std::this_thread::sleep_for (std::chrono::seconds (2));
      printf ("%s\n", buf);
      accepted_client.send ("answer\0", 7);
    }
  }
  printf ("shutdown accepted_client\n");
}

void server_func () {
  tcp_server_t sock (tcp_server_t::log_e::debug);
  if (sock.open (cfg_server) == tcp_server_t::no_error) {
    cfg_addr_t accepted_client_addr;

    std::vector<std::thread> accepted_clients;

    while (true) {

      tcp_client_t accepted_client = sock.accept (accepted_client_addr);
      if (accepted_client.state == tcp_client_t::state_opened) {
        std::thread accepted_thread = std::thread (accepted_client_func, std::move (accepted_client));
        accepted_clients.emplace_back (std::move (accepted_thread));
      }

    }
  }
}

void client_func () {
  tcp_client_t sock (tcp_client_t::log_e::debug);
  if (sock.open (cfg_client) == tcp_client_t::no_error) {

    for (size_t i = 0; i < 5; i++) {
      std::this_thread::sleep_for (std::chrono::seconds (4));
      if (sock.send ("hello\0", 6) >= 0) {
        char buf[1000];
        int  res = sock.recv (buf, 1000);
        if      (res == tcp_client_t::error_timeout) {
          // nothing
        }
        else if (res == tcp_client_t::error_unreachable) {
          // nothing
        }
        else if (res == 0) {
          // ???
          printf ("recv zero\n");
        }
        else if (res > 0) {
          // success
          printf ("%s\n", buf);
        }
      }
    }

  }
  printf ("shutdown client\n");
}

int main () {

  std::thread server (server_func);
  std::thread client (client_func);

  server.join ();
  //client.join ();

  return 0;

}
