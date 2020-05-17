
/*
 * ip-sockets-cpp-lite — header-only C++ networking utilities
 * 
 * Copyright (c) 2020 biaks ianiskr@gmail.com
 * Licensed under the MIT License
 */

#pragma once

#include "udp_socket.h"

template <ip_type_e Ip_type, socket_type_e Socket_type>
struct tcp_socket_t : udp_socket_t<Ip_type, Socket_type> {

  using base_socket_t = udp_socket_t<Ip_type, Socket_type>;
  using typename base_socket_t::address_t;
  using typename base_socket_t::log_e;
  using typename base_socket_t::state_e;
  using typename base_socket_t::error_e;

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

  int open (const address_t& address, uint32_t timeout_ms = 1000) {

    if ( parent != nullptr ) return this->log_and_return ('<', "open", base_socket_t::error_not_allowed);

    int result = base_socket_t::open (address, timeout_ms);

    if (this->socket_type == socket_type_e::server) {
      int res = listen (this->sock, 1000);

      if (res == SOCKET_ERROR) {
        this->log_and_return ('-', "open", this->_get_err (), "listen");
        closesocket (this->sock);
        return base_socket_t::error_open_failed;
      }
    }

    return result;

  }

  int recv (char* buf, int buf_len) {

    // udp socket can receive a zero-length datagram,
    // tcp socket CANNOT receive data of 0 bytes, the value 0 indicates that
    // the connection was intentionally terminated by the other side
    int result = base_socket_t::recv (buf, buf_len);
    return (result == 0)
      ? this->log_and_return ('<', "recv", base_socket_t::error_tcp_closed)
      : result;

  }

  int close () {
    if (this->state == base_socket_t::state_opened && parent)
      for (size_t i = 0; i < parent->accept_clients.size (); i++)
        if (parent->accept_clients[i] == this->sock)
          parent->accept_clients.erase (parent->accept_clients.begin () + i);
    return base_socket_t::close ();
  }

  tcp_socket_t<Ip_type, socket_type_e::client> accept (address_t& address_from) {

    using sockaddr_in_t = typename base_socket_t::sockaddr_in_t;

    tcp_socket_t<Ip_type, socket_type_e::client> result;

    int cerr = base_socket_t::no_error;

    if (this->state       != base_socket_t::state_opened) cerr = base_socket_t::error_not_open;
    if (this->socket_type != socket_type_e::server)       cerr = base_socket_t::error_not_allowed;

    if (cerr != base_socket_t::no_error) {
      this->log_and_return ('-', "accept", cerr);
      return result;
    }

    socklen_t     addr_len  = sizeof (sockaddr_in_t);
    sockaddr_in_t addr_from = {};
    result.sock             = ::accept (this->sock, (sockaddr*)&addr_from, &addr_len);

    if (result.sock == INVALID_SOCKET) {
      this->log_and_return ('-', "accept", this->_get_err ());
      address_from = {};
      return result;
    }

    address_from          = this->sockaddr2address (addr_from);

    result.address_remote = address_from;
    result.address_local  = result._getsockname ();
    result.state          = (typename tcp_socket_t<Ip_type, socket_type_e::client>::state_e) base_socket_t::state_opened;
    result.log_level      = (typename tcp_socket_t<Ip_type, socket_type_e::client>::log_e)   this->log_level;
    result.parent         = this;
    result.tname          = std::string ("tcp<") + ((this->ip_type == v4) ? "ip4," : "ip6,") + "accept>";

    result.log_and_return ('-', "accept", base_socket_t::no_error);
    accept_clients.push_back (result.sock);
    return result;

  }




};
