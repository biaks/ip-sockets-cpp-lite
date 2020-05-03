
/*
 * ip-sockets-cpp-lite — header-only C++ networking utilities
 * 
 * Copyright (c) 2020 biaks
 * Licensed under the MIT License
 */

#pragma once

#include "common/orders.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <cassert>

struct ip4_t : public std::array<uint8_t, 4> {
  // "192.168.2.1"
  // "192.0xa8.2.0x1"
  // "0xc0a80201"
  // "3232236033"
  // "127.1"
  ip4_t& from_str (const char* value, size_t length = 20, bool* success = nullptr) { // max length = "0xFF.0xFF.0xFF.0xFF" = 20

    enum { start, cont, hex, dec, error } state = start;

    uint8_t  result[3] = {};
    size_t   octet     = 0;
    uint32_t accum     = 0;

    while (length-- && *value != '\0' && state != error) {

      if (state == start) {
        if (*value < '0' || '9' < *value)
          state = error;
        else if (*value == '0' && length >= 2 && (*(value + 1) == 'x' || *(value + 1) == 'X')) {
          value  += 2;
          length -= 2;
          state   = hex;
        }
        else
          state   = dec;
      }

      if (state == dec) {
        if ('0' <= *value && *value <= '9')
          accum = accum * 10 + (*value - '0');
        else if (*value == '.')
          state = cont;
        else
          state = error;
      }
      else if (state == hex) {
        if      ('0' <= *value && *value <= '9') accum = (accum << 4) + (     *value - '0');
        else if ('a' <= *value && *value <= 'f') accum = (accum << 4) + (10 + *value - 'a');
        else if ('A' <= *value && *value <= 'F') accum = (accum << 4) + (10 + *value - 'A');
        else if (*value == '.')
          state = cont;
        else
          state = error;
      }

      if (state == cont) {
        if (accum > 0xff || octet >= 3)
          state = error;
        else {
          result[octet++] = (uint8_t)accum;
          accum           = 0;
          state           = start;
        }
      }

      value++;
    }

    if (state == error)
      *this = ip4_t {};
    else if (octet != 0) {
      if (accum > 0xff)
        *this = ip4_t {};
      else {

        // 1.2.3.4  1.2.x.3  1.x.x.2  1.x.x.x
        // 0 1 2 3  0 1 x 2  0 x x 1  0 x x x

        for (size_t i = 0; i < octet; i++)
          (*this)[i] = result[i];

        for (size_t i = octet; i < 3; i++)
          (*this)[i] = 0;

        (*this)[3] = (uint8_t)accum;
      }
    }
    else
      *((uint32_t*)this) = common::htonT (accum);

    if (success)
      *success = (state != error);

    return *this;
  }

  ip4_t& from_str (const std::string& value) {
    return from_str (value.data (), value.size ());
  }

  ip4_t () = default;

  ip4_t (std::array<uint8_t, 4>&& arr) : std::array<uint8_t, 4> (arr) {}

  // for constructor ip4_t({1,2,3,4}) we can use native constructor
  //ip4_t (std::initializer_list<uint8_t> values) {
  //  assert (values.size () == 4);
  //  std::copy (values.begin (), values.end (), this);
  //}

  template <size_t Size>
  ip4_t (char (&&value)[Size]) {
    from_str (value, Size);
  }

  ip4_t (const char* value) {
    from_str (value);
  }

  ip4_t (const char* value, size_t length) {
    from_str (value, length);
  }

  ip4_t (const std::string& value) {
    from_str (value.data (), value.size ());
  }

  ip4_t (const uint32_t& value) {
    *((uint32_t*)this) = common::htonT (value);
  }

  ip4_t (uint32_t&& value) {
    *((uint32_t*)this) = common::htonT (value);
  }

  ip4_t (uint8_t&& v1, uint8_t&& v2, uint8_t&& v3, uint8_t&& v4) {
    (*this)[0] = v1; (*this)[1] = v2; (*this)[2] = v3; (*this)[3] = v4;
  }

  ip4_t (uint8_t prefix) {
    const uint8_t masks[9]  = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
    const uint8_t byte_size = 8;
    for (size_t i = 0; i < 4; ++i) {
      if (prefix >= byte_size) {
        this->operator[] (i) = masks[byte_size];
        prefix -= byte_size;
      } else {
        this->operator[] (i) = masks[prefix];
        prefix = 0;
      }
    }
  }

  ip4_t& set_mask (uint8_t prefix) {
    ip4_t mask (prefix);
    this->operator&= (mask);
    return *this;
  }

  ip4_t operator& (const ip4_t& other) {
    return *((uint32_t*)this) & *((uint32_t*)&other);
  }

  void operator&= (const ip4_t& other) {
    *((uint32_t*)this) &= *((uint32_t*)&other);
  }

  void rotate () {
    std::swap ((*this)[0], (*this)[3]);
    std::swap ((*this)[1], (*this)[2]);
  }

  std::string to_str () const {
    return std::to_string ((*this)[0]) + '.' + std::to_string ((*this)[1]) + '.' + std::to_string ((*this)[2]) + '.' + std::to_string ((*this)[3]);
  }

  operator std::string () const {
    return to_str ();
  }

  operator uint32_t () const {
    return common::ntohT (*((uint32_t*)this));
  }
};

static inline std::ostream& operator<< (std::ostream& os, const ip4_t& ipv4) {
  os << ipv4.to_str ();
  return os;
}

template<>
struct std::hash<ip4_t> {
  inline std::size_t operator() (const ip4_t& ip) const {
    //uint32_t hash = (uint32_t)ip;
    return (uint32_t)ip;
  }
};


struct ip6_t : public std::array<uint8_t, 16> {

  // "5555:6666:7777:8888:9999:aaaa:bbbb:cccc"
  // "1:2:3:4:5:6:7:8"
  // "1::5:6:7:8"
  // "::5:6:7:8"
  // "1:2:3:4::"
  // "::"
  // "1:2:3:4:5:6:192.168.2.1"
  // "::192.168.2.1"
  // "5555:6666:7777:8888:9999:aaaa:255.255.255.255"
  ip6_t& from_str (const char* value, size_t length = 45, bool* success = nullptr) {

    enum { start, proc, error } state = start;

    uint16_t result_hex[8] = {};
    uint8_t  result_dec[3] = {};
    size_t   index_hex     = 0;
    size_t   index_dec     = 0;
    size_t   separator     = SIZE_MAX;
    uint32_t accum_dec     = 0;
    uint32_t accum_hex     = 0;
    uint8_t  sym           = 0;

    while (length-- && *value != '\0' && state != error) {

      // ::xx..  ..xx::xx..  ..xx::  ::
      if (length >= 1 && *value == ':' && *(value + 1) == ':') {
        if (separator == SIZE_MAX && index_dec == 0 && accum_hex <= 0xffff && index_hex < 7) {
          if (state == proc) {
            result_hex[index_hex++] = (uint16_t)accum_hex;
            accum_hex               = 0;
            accum_dec               = 0;
          }
          separator = index_hex;
          value  += 1;
          length -= 1;
        }
        else
          state = error;
      }
      // ..xx:xx..
      else if (*value == ':') {
        if (state == proc && index_dec == 0 && accum_hex <= 0xffff && index_hex < 7) {
          result_hex[index_hex++] = (uint16_t)accum_hex;
          accum_hex               = 0;
          accum_dec               = 0;
        }
        else
          state = error;
      }
      // ..xx:n.n.n.n  ..xx::n.n.n.n  ::n.n.n.n
      else if (*value == '.') {
        if (state == proc && accum_dec <= 0xff && index_dec <= 6 && index_dec < 3) {
          result_dec[index_dec++] = (uint8_t)accum_dec;
          accum_dec               = 0;
          accum_hex               = 0;
        }
        else
          state = error;
      }
      // xxxx
      else {
        if      ('0' <= *value && *value <= '9') sym = *value - '0';
        else if ('a' <= *value && *value <= 'f') sym = 10 + *value - 'a';
        else if ('A' <= *value && *value <= 'F') sym = 10 + *value - 'A';
        else
          state = error;

        accum_hex = (accum_hex << 4) + sym;
        accum_dec =  accum_dec * 10  + sym;
        if (state != error)
          state = proc;
      }

      value++;
    }

    if ((index_dec != 0 && index_dec != 3) || (index_dec == 0 && accum_hex > 0xffff) || (index_dec == 3 && accum_dec > 0xff))
      state = error;

    if (state == error) {
      *this = {};
      if (success)
        *success = false;
      return *this;
    }

    if (index_dec == 3) {
      result_hex[index_hex++] = (result_dec[0] << 8) | result_dec[1];
      result_hex[index_hex++] = (result_dec[2] << 8) | (uint8_t)accum_dec;
    }
    else
      result_hex[index_hex++] = (uint16_t)accum_hex;

    size_t current = 0;

    if (separator == SIZE_MAX)
      separator = 0;

    for (; current < separator; current++)
      ((uint16_t*)this)[current] = common::htonT (result_hex[current]);

    for (; current < separator + (8 - index_hex); current++)
      ((uint16_t*)this)[current] = 0x0000;

    for (; current < 8; current++)
      ((uint16_t*)this)[current] = common::htonT (result_hex[current - (8 - index_hex)]);

    if (success)
      *success = true;
    return *this;
  }

  ip6_t () = default;

  ip6_t (std::array<uint8_t, 16>&& arr) : std::array<uint8_t, 16> (arr) {}

  template <size_t Size>
  ip6_t (char (&&value)[Size]) {
    from_str (value, Size);
  }

  ip6_t (const char* value) {
    from_str (value);
  }

  ip6_t (const char* value, size_t length) {
    from_str (value, length);
  }

  ip6_t (const std::string& value) {
    from_str (value.data (), value.size ());
  }
  ip6_t (uint8_t&& v1, uint8_t&& v2,  uint8_t&& v3,  uint8_t&& v4,  uint8_t&& v5,  uint8_t&& v6,  uint8_t&& v7,  uint8_t&& v8,
         uint8_t&& v9, uint8_t&& v10, uint8_t&& v11, uint8_t&& v12, uint8_t&& v13, uint8_t&& v14, uint8_t&& v15, uint8_t&& v16) {
    ((uint8_t*)this)[0]  = common::htonT (v1);  ((uint8_t*)this)[1]  = common::htonT (v2); 
    ((uint8_t*)this)[2]  = common::htonT (v3);  ((uint8_t*)this)[3]  = common::htonT (v4);
    ((uint8_t*)this)[4]  = common::htonT (v5);  ((uint8_t*)this)[5]  = common::htonT (v6);
    ((uint8_t*)this)[6]  = common::htonT (v7);  ((uint8_t*)this)[7]  = common::htonT (v8);
    ((uint8_t*)this)[8]  = common::htonT (v9);  ((uint8_t*)this)[9]  = common::htonT (v10);
    ((uint8_t*)this)[10] = common::htonT (v11); ((uint8_t*)this)[11] = common::htonT (v12);
    ((uint8_t*)this)[12] = common::htonT (v13); ((uint8_t*)this)[13] = common::htonT (v14);
    ((uint8_t*)this)[14] = common::htonT (v15); ((uint8_t*)this)[15] = common::htonT (v16);
  }

  ip6_t (uint16_t&& v1, uint16_t&& v2, uint16_t&& v3, uint16_t&& v4, uint16_t&& v5, uint16_t&& v6, uint16_t&& v7, uint16_t&& v8) {
    ((uint16_t*)this)[0] = common::htonT (v1); ((uint16_t*)this)[1] = common::htonT (v2);
    ((uint16_t*)this)[2] = common::htonT (v3); ((uint16_t*)this)[3] = common::htonT (v4);
    ((uint16_t*)this)[4] = common::htonT (v5); ((uint16_t*)this)[5] = common::htonT (v6);
    ((uint16_t*)this)[6] = common::htonT (v7); ((uint16_t*)this)[7] = common::htonT (v8);
  }

  const ip4_t& get_ip4 () const {
    return *((ip4_t*)this + 3); // 4*3 = 12
  }

  ip4_t& get_ip4 () {
    return *((ip4_t*)this + 3); // 4*3 = 12
  }

  ip6_t (uint8_t prefix) {
    const uint8_t masks[9]  = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
    const uint8_t byte_size = 8;
    for (size_t i = 0; i < 16; ++i) {
      if (prefix >= byte_size) {
        this->operator[] (i) = masks[byte_size];
        prefix -= byte_size;
      } else {
        this->operator[] (i) = masks[prefix];
        prefix               = 0;
      }
    }
  }

  ip6_t& set_mask (uint8_t prefix) {
    ip6_t mask (prefix);
    this->operator&= (mask);
    return *this;
  }

  std::string to_str (bool reduction = true, bool embedded_ipv4 = false) const {

    std::string                result;
    const char                 symbols[] = "0123456789abcdef";
    bool                       tochka    = false;
    const uint16_t*            part      = (uint16_t*)this;
    enum { start, found, end } zero      = (reduction) ? start : end;

    result.reserve (45);

    for (size_t i = 0; i < ((embedded_ipv4) ? 6 : 8); i++, part++) {
      if (*part == 0 && zero == start) {
        zero = found;
        result.push_back (':');
        if (tochka == false) result.push_back (':');
      }
      else
        if (*part != 0 && zero == found)
          zero = end;
      if (zero != found) {
        bool    non_zero = false;
        uint8_t half_of_byte;
        half_of_byte = *((uint8_t*)part    ) >> 4; if (half_of_byte) non_zero = true; if (non_zero) result.push_back (symbols[half_of_byte]);
        half_of_byte = *((uint8_t*)part    ) & 15; if (half_of_byte) non_zero = true; if (non_zero) result.push_back (symbols[half_of_byte]);
        half_of_byte = *((uint8_t*)part + 1) >> 4; if (half_of_byte) non_zero = true; if (non_zero) result.push_back (symbols[half_of_byte]);
        half_of_byte = *((uint8_t*)part + 1) & 15; if (half_of_byte) non_zero = true; if (non_zero) result.push_back (symbols[half_of_byte]);
        tochka = true;
        if (i != 7)
          result.push_back (':');
      }
    }

    if (embedded_ipv4) {
      for (size_t i = 0; i < 4; i++) {
        uint8_t byte     = *((uint8_t*)part + i); // byte = XYZ
        uint8_t div      = byte % 100;            // div = XYZ % 100 = 0YZ
        bool    non_zero = false;
        byte /= 100; // byte = XYZ / 100 = X
        if (byte != 0) {
          result.push_back (symbols[byte]);
          non_zero = true;
        }
        byte = div / 10; // byte = 0YZ / 10 = Y
        div  = div % 10; // div = 0YZ % 10 = Z
        if (byte != 0 || non_zero == true)
          result.push_back (symbols[byte]);
        result.push_back (symbols[div]);
        if (i != 3)
          result.push_back ('.');
      }
    }
    return result;
  }

  operator std::string () const {
    return to_str ();
  }

  operator bool () const {
    const uint64_t* part1 = (uint64_t*)this;
    const uint64_t* part2 = part1 + 1;
    return (*part1 || *part2);
  }

  ip6_t operator& (const ip6_t& other_ip) {
    ip6_t result;
    *((uint64_t*)&result)     = *((uint64_t*)this)     & *((uint64_t*)&other_ip);
    *((uint64_t*)&result + 1) = *((uint64_t*)this + 1) & *((uint64_t*)&other_ip + 1);
    return result;
  }

  void operator&= (const ip6_t& other_ip) {
    *((uint64_t*)this)     &= *((uint64_t*)&other_ip);
    *((uint64_t*)this + 1) &= *((uint64_t*)&other_ip + 1);
  }

};

static inline std::ostream& operator<< (std::ostream& os, const ip6_t& ipv6) {
  os << ipv6.to_str ();
  return os;
}

template <>
struct std::hash<ip6_t> {
  inline std::size_t operator() (const ip6_t& ip) const noexcept{
    return *((uint64_t*)&ip) ^ *((uint64_t*)&ip + 1);
  }
};

enum ip_type_e { v4 = 4, v6 = 16 };

template <ip_type_e type>
struct ip_t_;

template <>
struct ip_t_<v4> {
  using type = ip4_t;
};

template <>
struct ip_t_<v6> {
  using type = ip6_t;
};

template <ip_type_e type>
using ip_t = typename ip_t_<type>::type;

// addr_t

struct addr4_t {

  ip4_t    ip   = {};
  uint16_t port = 0; // le

  // nnn.nnn.nnn.nnn:ppppp
  addr4_t& from_str (const char* value, size_t length = 21, bool* success = nullptr) {

    size_t      ip_length = 0;
    const char* ptr     = value;
    size_t      accum   = 0;

    while (*ptr && length--) {
      if (ip_length == 0) {
        if (*ptr == ':')
          ip_length = ptr - value;
        else if ((*ptr < '0' || '9' < *ptr) && (*ptr < 'a' || 'f' < *ptr) && (*ptr < 'A' || 'F' < *ptr) && *ptr != 'x' && *ptr != '.')
          break;
      }
      else {
        if ('0' <= *ptr && *ptr <= '9')
          accum = accum * 10 + (*ptr - '0');
        else
          break;
      }
      ptr++;
    }

    ip.from_str (value, ip_length, success);

    if (ip_length == 0 || accum == 0 || accum > 0xffff) {
      if (success)
        *success = false;
      *this = {};
    }
    else    
      port = (uint16_t)accum;

    return *this;

  }

  addr4_t () = default;

  addr4_t (const ip4_t& ip_, uint16_t port_) : ip (ip_), port (port_) {}

  template <size_t Size>
  addr4_t (char (&&value)[Size]) {
    from_str (value, Size);
  }

  addr4_t (const char* value) {
    from_str (value);
  }

  addr4_t (const char* value, size_t length) {
    from_str (value, length);
  }

  addr4_t (const std::string& value) {
    from_str (value.data (), value.size ());
  }

  template <size_t Size>
  addr4_t (const std::array<uint8_t, Size>& value) {
    from_str (value.data(), Size);
  }

  std::string to_str () const {
    return ip.to_str() + ':' + std::to_string (port);
  }

  operator std::string () const {
    return to_str ();
  }

  operator bool () const {
    return (ip && port != 0);
  }

  bool operator== (const addr4_t& other_addr) const {
    return this->ip == other_addr.ip && this->port == other_addr.port;
  }

};

static inline std::ostream& operator<< (std::ostream& os, const addr4_t& addr4) {
  os << addr4.to_str ();
  return os;
}

template <>
struct std::hash<addr4_t> {
  std::size_t operator() (const addr4_t& addr4) const noexcept {
    return hash<ip4_t> {}(addr4.ip) ^ hash<uint16_t> {}(addr4.port);
  }
};



struct addr6_t {

  ip6_t    ip;
  uint16_t port; // le

  // [xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:nnn.nnn.nnn.nnn]:ppppp
  addr6_t& from_str (const char* value, size_t length = 53, bool* success = nullptr) {

    enum { open, ipp, close, tochki, portt, error, end } state = open;
    const char* ip_ptr    = nullptr;
    size_t      ip_length = 0;
    const char* ptr       = value;
    size_t      accum     = 0;

    while (*ptr && length-- && state != error) {
      switch (state) {
        case open:
          if (*ptr == '[') {
            ip_ptr = ptr + 1;
            state  = ipp;
          }
          else
            state = error;
        break;
        case ipp:
          if (*ptr == ']') {
            ip_length = ptr - ip_ptr;
            if (ip_length >= 2)
              state = close;
            else
              state = error;
          }
          else if ((*ptr < '0' || '9' < *ptr) && (*ptr < 'a' || 'f' < *ptr) && (*ptr < 'A' || 'F' < *ptr) && *ptr != ':' && *ptr != '.')
            state = error;
        break;
        case close:
          if (*ptr == ':')
            state  = portt;
          else
            state = error;
        break;
        case portt:
          if ('0' <= *ptr && *ptr <= '9')
            accum = accum * 10 + (*ptr - '0');
          else if (accum != 0)
            state = end;
          else
            state = error;
        break;
        default:
        break;
      }
      ptr++;
    }

    if (state != error)
      ip.from_str (ip_ptr, ip_length, success);

    if (state == error || accum > 0xffff) {
      if (success)
        *success = false;
      *this = {};
    }
    else
      port = (uint16_t)accum;

    return *this;

  }

  addr6_t () = default;

  addr6_t (const ip6_t& ip_, uint16_t port_) : ip (ip_), port (port_) {}

  template <size_t Size>
  addr6_t (char (&&value)[Size]) {
    from_str (value, Size);
  }

  addr6_t (const char* value) {
    from_str (value);
  }

  addr6_t (const char* value, size_t length) {
    from_str (value, length);
  }

  addr6_t (const std::string& value) {
    from_str (value.data (), value.size ());
  }

  template <size_t Size>
  addr6_t (const std::array<uint8_t, Size>& value) {
    from_str (value.data (), Size);
  }

  std::string to_str (bool reduction = true, bool embedded_ipv4 = false) const {
    return std::string ("[") + ip.to_str (reduction, embedded_ipv4) + std::string ("]:") + std::to_string (port);
  }

  operator std::string () const {
    return to_str ();
  }

  operator bool () const {
    return (ip && port != 0);
  }

  bool operator== (const addr6_t& other_addr) const {
    return this->ip == other_addr.ip && this->port == other_addr.port;
  }

};

static inline std::ostream& operator<< (std::ostream& os, const addr6_t& addr6) {
  os << addr6.to_str ();
  return os;
}

template <>
struct std::hash<addr6_t> {
  inline std::size_t operator() (const addr6_t& addr6) const noexcept {
    return hash<ip6_t> {}(addr6.ip) ^ hash<uint16_t> {}(addr6.port);
  }
};

template <ip_type_e type>
struct addr_t_;

template <>
struct addr_t_<v4> {
  using type = addr4_t;
};

template <>
struct addr_t_<v6> {
  using type = addr6_t;
};

template <ip_type_e type>
using addr_t = typename addr_t_<type>::type;
