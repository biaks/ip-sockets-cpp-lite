
/*
 * Copyright (c) 2020 biaks ianiskr@gmail.com
 * Licensed under the MIT License
 */

#pragma once

#include "orders.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <cassert>

// base classes for working with ipv4 and ipv6 addresses classes are based on standard std::array<uint, type>
// and can be "overlayed" onto corresponding memory areas, for example:
// uint8_t fragment[10] = {0xff,  0xc0, 0xa8, 0x02, 0x01,  0xff,0xff,0xff,0xff,0xff};
// ip4_t& ipv4 = *((ip4_t*)&fragment[1]);
// after that, all tools for working with this ip address become available, without copying the ip address itself
//
// also, classes can be declared in two different ways:
// use ip4_t and ip6_t classes directly where it is known in advance which ip address type will be used
// or use the generic form ip_t<type> where working with an abstract ip address is intended, for example:
// template <ip_type_e type>
// struct ip_address_holder_t {
//   ip_t<type> ip_address;
// };

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
      *((uint32_t*)this) = htonT (accum);

    if (success)
      *success = (state != error);

    return *this;
  }

  ip4_t& from_str (const std::string& value) {
    return from_str (value.data (), value.size ());
  }

  ip4_t () = default;

  ip4_t (std::array<uint8_t, 4>&& arr) : std::array<uint8_t, 4> (arr) {}

  ip4_t (std::initializer_list<uint8_t> values) {
    assert (values.size () == 4);
    std::copy (values.begin (), values.end (), this);
  }

  template <size_t Size>
  ip4_t (char (&&value)[Size]) {
    from_str (value, Size);
  }

  ip4_t (const char* value) {
    from_str (value);
  }

  ip4_t (const std::string& value) {
    from_str (value.data (), value.size ());
  }

  ip4_t (const uint32_t& value) {
    *((uint32_t*)this) = htonT (value);
  }

  ip4_t (uint32_t&& value) {
    *((uint32_t*)this) = htonT (value);
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
    return ntohT (*((uint32_t*)this));
  }
};

static inline std::ostream& operator<< (std::ostream& os, const ip4_t& ipv4) {
  os << ipv4.to_str ();
  return os;
}


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
      ((uint16_t*)this)[current] = htonT (result_hex[current]);

    for (; current < separator + (8 - index_hex); current++)
      ((uint16_t*)this)[current] = 0x0000;

    for (; current < 8; current++)
      ((uint16_t*)this)[current] = htonT (result_hex[current - (8 - index_hex)]);

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

// here is a lot of theory about IPv6 addresses
// and different ways of packing ipv4 addresses into ipv6 addresses

// about ipv6 addresses: https://en.wikipedia.org/wiki/IPv6_address
// https://www.ccexpert.us/routing-switching-2/ipv6-address-types.html

// UNICAST - represents the address of a specific interface (receiver). the packet is delivered to the specified interface
// ANYCAST - represents a group of interfaces (receivers). the packet is delivered to any one interface from the specified group
// MULTICAST - represents a group of interfaces (receivers). the packet is delivered to all interfaces from the specified group

// UNICAST
// ANYCAST
// network       subnet         interface
// 48 or more    16 or less     64
// XXXX:XXXX:XXXX: XXXX :XXXX:XXXX:XXXX:XXXX
// XXXX:XXXX:XXXX:XXXX  :XXXX:XXXX:XXXX:XXXX

// LOCAL
// prefix                        interface
// 10 = 0b1111111010            64
// fe80:0000:0000:0000  :XXXX:XXXX:XXXX:XXXX

// ::1/128 — The loopback address is a unicast localhost address
//           (corresponding to 127.0.0.1/8 in IPv4).
// fe80::/10 — Addresses in the link-local prefix are only valid and unique on a single link
//            (comparable to the auto-configuration addresses 169.254.0.0/16 of IPv4).
// fc00::/7 — Unique local addresses (ULAs) are intended for local communication[34]
//            (comparable to IPv4 private addresses 10.0.0.0/8, 172.16.0.0/12 and 192.168.0.0/16).

// TEREDO - is a tunneling mechanism for ipv6 internet access for ipv4 clients through ipv4 network https://www.rfc-editor.org/rfc/rfc4380

// Teredo IPv6 Address https://blog.ip2location.com/knowledge-base/what-is-teredo-tunneling/
// teredo prefix   teredo server       ~NAT port  ~client IP
// 2001:0000      :x.x.x.x       :8000 :port      :x.x.x.x

// Teredo ip6to4 https://blog.ip2location.com/knowledge-base/what-is-6to4-tunneling/
// teredo prefix ipv4 address  subnet
// 2001          :x.x.x.x     :XXXX   :0000:0000 :x.x.x.x

// Dedicated IPv6 addresses for internet. "2000::/16" up to 2001. "2001::/16" after 2001. "2002::/16" for routing from IPv6 to IPv4 on the internet.

// MULTICAST (old) RFC 2373
// prefix fig sc group
// 8       4   4  112
// ff      x   x  :XXXX:XXXX:XXXX:XXXX:XXXX:XXXX:XXXX

// MULTICAST (new) RFC 3306, updated by RFC 7371
// prefix ff1 sc ff2 rsrvd plen prefix              group
// 8       4   4  4   4     8    64                   32
// ff      x   x :x   x     xx  :XXXX:XXXX:XXXX:XXXX :XXXX:XXXX

// transform ipv4 to ipv6:
// http://www.gestioip.net/cgi-bin/subnet_calculator.cgi
// https://www.iana.org/assignments/iana-ipv6-special-registry/iana-ipv6-special-registry.xml
// ::ffff:x.x.x.x/96                   - IPv4-mapped IPv6 address                            ::ffff:0:0/96   rfc4291 https://www.iana.org/go/rfc4291
// ::ffff:0:x.x.x.x/96                 - IPv4-translated IPv6 address
// 2002:x.x.x.x::                      - Additional 6to4 information                         2002::/16       rfc3056 https://www.iana.org/go/rfc3056
// 64:ff9b::x.x.x.x/96                 - IPv4-IPv6 Translatable Address - Global internet    64:ff9b::/96    rfc6052 https://www.iana.org/go/rfc6052
// 64:ff9b:1:ffff:ffff:ffff:x.x.x.x/48 - IPv4-IPv6 Translatable Address - Private internets  64:ff9b:1::/48  rfc8215 https://www.iana.org/go/rfc8215

// The prefix that we recommend has the particularity of being "checksum neutral".
// The sum of the hexadecimal numbers "0064" and "ff9b" is "ffff",
// i.e.,
// a value equal to zero in one's complement arithmetic. An IPv4-embedded IPv6 address constructed
// with this prefix will have the same one's complement checksum as the embedded IPv4 address.

// 127.0.0.1/8 -> ::1/128
// 169.254.0.0/16 -> fe80::/10
// 192.168.0.0/16 -> fc00::/7
// 10.0.0.0/8 -> fc00::/7

// rfc4291 https://www.rfc-editor.org/rfc/rfc4291
// 2.5.5.1.  IPv4-Compatible IPv6 Address
//   ::x.x.x.x - ipv4 address must be unique, this type is now deprecated
// 2.5.5.2.  IPv4-Mapped IPv6 Address (more details rfc4038 https://www.rfc-editor.org/rfc/rfc4038)
//   ::ffff:x.x.x.x -

// more about multicast addresses: https://en.wikipedia.org/wiki/Multicast_address#IPv6

// "IP" or "[IP]"
// "xxxx" can be shortened to a single value "x"
// ::
// ::xxxx
// xxxx::
// ::xxxx:xxxx:xxxx
// xxxx:xxxx:xxxx::
// xxxx::xxxx
// max length xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:nnn.nnn.nnn.nnn
// ipv4 a.b.c.d -> ipv6 ::ffff:a.b.c.d
