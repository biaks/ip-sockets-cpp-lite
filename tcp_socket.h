
/*
 * ip-sockets-cpp-lite — header-only C++ networking utilities
 * https://github.com/biaks/ip-sockets-cpp-lite
 * 
 * Copyright (c) 2021 biaks ianiskr@gmail.com
 * Licensed under the MIT License
 */

#pragma once

#include "udp_socket.h"

namespace ipsockets {

  template <ip_type_e Ip_type, socket_type_e Socket_type>
  struct tcp_socket_t : udp_socket_t<Ip_type, Socket_type> {

    using base_socket_t = udp_socket_t<Ip_type, Socket_type>;
    using typename base_socket_t::address_t;

    tcp_socket_t<Ip_type, socket_type_e::server>* parent = nullptr;
    std::vector<socket_t>                         accept_clients;

    tcp_socket_t (log_e log_level_ = log_e::info)
      : base_socket_t (log_level_) {
      this->type     = SOCK_STREAM;
      this->protocol = IPPROTO_TCP;
      this->tname    = std::string ("tcp<") + ((this->ip_type == v4) ? "ip4," : "ip6,")
                     + ((this->socket_type == socket_type_e::server) ? "server>" : "client>");
    }

    tcp_socket_t (const tcp_socket_t& socket) = delete;

    tcp_socket_t (tcp_socket_t&& other_socket) : base_socket_t (std::move(other_socket)) {
      parent               = other_socket.parent;
      other_socket.parent  = nullptr;
      accept_clients       = std::move (other_socket.accept_clients);
    }

    ~tcp_socket_t () {
      if (parent == nullptr)
        for (socket_t& accept_sock : accept_clients)
          closesocket (accept_sock);
      close ();
    }

    int open (const address_t& address, uint32_t timeout_ms = 1000, int max_incoming_queue = 1000) {

      if ( parent != nullptr ) return this->log_and_return ('<', "open", error_not_allowed);

      int result = base_socket_t::open (address, timeout_ms);

      if (this->socket_type == socket_type_e::server) {
        int res = listen (this->sock, max_incoming_queue); 

        if (res == SOCKET_ERROR) {
          this->log_and_return ('-', "open", this->_get_err (), "listen");
          closesocket (this->sock);
          return error_open_failed;
        }
      }

      return result;

    }

    int recv (char* buf, int buf_len) {

      // udp socket can receive a zero-length datagram,
      // tcp socket CANNOT receive data of 0 bytes, the value 0 indicates that
      // the connection was intentionally terminated by the other side
      // for linux   base_socket_t::recv received 0 and then we correct it to error_tcp_closed
      // for windows base_socket_t::recv received error_tcp_closed because it catch WSAECONNABORTED error code from OS
      int result = base_socket_t::recv (buf, buf_len);
      return (result == 0)
        ? this->log_and_return ('<', "recv", error_tcp_closed)
        : result;

    }

    /// @brief Closes the socket.
    /// Accepted sockets will be automatically closed, when the server object will distroyed.
    /// @return Error code or no_error on successful close.
    int close () {
      if (this->state == state_e::state_opened && parent)
        for (size_t i = 0; i < parent->accept_clients.size (); i++)
          if (parent->accept_clients[i] == this->sock)
            parent->accept_clients.erase (parent->accept_clients.begin () + i);
      return base_socket_t::close ();
    }

    template <socket_type_e SOCK = Socket_type, std::enable_if_t<SOCK == socket_type_e::server, bool> = true>
    tcp_socket_t<Ip_type, socket_type_e::client> accept (address_t& address_from, bool* success = nullptr) {

      using sockaddr_in_t = typename base_socket_t::sockaddr_in_t;

      tcp_socket_t<Ip_type, socket_type_e::client> result (this->log_level);

      int cerr = no_error;

      if (this->state       != state_e::state_opened) cerr = error_not_open;
      if (this->socket_type != socket_type_e::server) cerr = error_not_allowed;

      if (cerr != no_error) {
        this->log_and_return ('-', "accept", cerr);
        return result;
      }

      socklen_t     addr_len  = sizeof (sockaddr_in_t);
      sockaddr_in_t addr_from = {};

      #ifdef _WIN32 // WINDOWS OS
        // for windows ::accept will never be timeout and we should close sock connection to interrupt it or we should using poll()
        DWORD tv_ms  = 1000; // get SO_RCVTIMEO if set (DWORD on Windows), fallback to 1000 ms
        int   optlen = sizeof(tv_ms);
        if (getsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv_ms, &optlen) == SOCKET_ERROR)
          tv_ms = 1000;

        WSAPOLLFD pfd;
        pfd.fd     = this->sock;
        pfd.events = POLLRDNORM; // wait incoming data

        int rv = WSAPoll(&pfd, 1, (int)tv_ms);
        if (rv == 0) { // timeout
          result.sock = INVALID_SOCKET;
          cerr        = error_timeout;
        }
        else if (pfd.revents & POLLRDNORM) { // data ready
          result.sock = ::accept (this->sock, (sockaddr*)&addr_from, &addr_len);
          cerr        = this->_get_err ();
        }
        else { // error
          result.sock = INVALID_SOCKET;
          cerr        = this->_get_err ();
        }
      #else         // LINUX OS
        // for linux   ::accept will be timeout by value SO_RCVTIMEO wich we set by setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof (tv));
        result.sock = ::accept (this->sock, (sockaddr*)&addr_from, &addr_len);
        cerr = this->_get_err ();
      #endif

      if (result.sock == INVALID_SOCKET) {
        this->log_and_return ('-', "accept", cerr);
        address_from = {};
        if (success)
          *success = false;
        return result;
      }

      address_from          = this->sockaddr2address (addr_from);

      result.address_remote = address_from;
      result.address_local  = result._getsockname ();
      result.state          = state_e::state_opened;
      //result.log_level      = this->log_level; set via constructor
      result.parent         = this;
      result.tname          = std::string ("tcp<") + ((this->ip_type == v4) ? "ip4," : "ip6,") + "accept>";

      result.log_and_return ('-', "accept", no_error);
      accept_clients.push_back (result.sock);
      if (success)
        *success = true;
      return result;

    }

  };

} // namespace ipsockets
