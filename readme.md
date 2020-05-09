
# ip-sockets-cpp-lite

A lightweight, header-only C++ networking utility library providing unified IP address handling.
The library is designed to be simple, dependency-free, and easy to integrate into existing C++ projects.

## Key Features

- Header-only library
- Written in pure C++

## Components

The library consists of three main headers:

### `ip.h`
Unified IPv4 and IPv6 address and prefix handling:

- IPv4 and IPv6 address types
- Text ↔ binary conversion
- Native storage using `std::array<>`
- IPv4-over-IPv6 mapping
- Network prefix handling and masking
- Binary network prefix format
- Address + port types (`addr4_t`, `addr6_t`)
- Rich constructors and implicit/explicit conversions
- Bitwise operations and comparisons

### `udp_socket.h`
Cross-platform UDP socket abstraction:

- Unified client and server modes
- IPv4 and IPv6 support
- Synchronous I/O with configurable timeouts
- Polling-based asynchronous mode
- Explicit socket state and error handling
- Cross-platform API (POSIX / Windows)

License

MIT (or your preferred license)