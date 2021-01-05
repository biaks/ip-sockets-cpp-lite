
# ip-sockets-cpp-lite

A lightweight, header-only C++ networking utility library providing unified IP address handling and cross-platform UDP/TCP socket abstractions.

The library is designed to be simple, dependency-free, and easy to integrate into existing C++ projects.

## Key Features

- Header-only library
- Written in pure C++
- Cross-platform (Linux, Windows, macOS)
- No third-party dependencies
- Minimalistic and low-level design
- Very easy to integrate and use
- Optional CMake integration
- Can be used by simply including headers
- Unified IPv4 / IPv6 handling
- Cross-platform UDP and TCP socket wrappers
- Suitable for embedded, tools, services, and low-level networking code

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

### `tcp_socket.h`
TCP socket abstraction built on top of `udp_socket.h`:

- Unified TCP client and server
- Blocking I/O with timeouts
- Accept loop helpers
- Explicit connection state handling
- Same API style as UDP for consistency

## Requirements

- C++11 or newer
- Standard C++ library
- No external dependencies

## Integration

### Option 1 — Header-only (simplest)

Just copy the headers into your project and include one of them:

```cpp
#include "ip.h"          // if you need only ip logic
#include "udp_socket.h"  // if you need udp socket
#include "tcp_socket.h"  // if you need tcp socket
```

No build system changes required.

### Option 2 — CMake (optional)

You can also add the library as a subdirectory and link it via CMake.

## Quick Examples

### IPv4 / IPv6 address handling

```cpp
#include "ip.h"

int main() {
    ip4_t ip4 = "192.168.1.10";
    ip6_t ip6 = "2001:db8::1";

    std::cout << ip4 << std::endl;
    std::cout << ip6 << std::endl;

    // Convert IPv4 to IPv6 (IPv4-mapped IPv6)
    ip6_t ip6_from_v4 = ip4;

    // Apply network mask
    ip4_t network = ip4 & ip4_t{255, 255, 255, 0};

    // Address + port
    addr4_t addr4 = "192.168.1.10:8080";
    addr6_t addr6 = "[::1]:8080";

    std::cout << addr4 << std::endl;
    std::cout << addr6 << std::endl;
}
```

### Simple UDP server

```cpp
#include "udp_socket.h"

using server_t = udp_socket_t<v4, socket_type_e::server>;

int main() {
    server_t server(server_t::log_e::info);

    if (server.open("0.0.0.0:2000") != server_t::no_error) {
        return 1;
    }

    char buffer[1024];
    addr4_t client;

    while (true) {
        int res = server.recvfrom(buffer, sizeof(buffer), client);
        if (res > 0) {
            server.sendto("ok", 2, client);
        }
    }
}
```

### Simple TCP client

```cpp
#include "tcp_socket.h"

using client_t = tcp_socket_t<v4, socket_type_e::client>;

int main() {
    client_t client(client_t::log_e::info);

    if (client.open("127.0.0.1:2000") != client_t::no_error) {
        return 1;
    }

    client.send("hello", 5);

    char buffer[1024];
    client.recv(buffer, sizeof(buffer));
}
```

## Design Goals

- Minimal abstraction overhead
- Predictable performance
- Explicit error handling
- Unified API for IPv4 and IPv6
- No hidden background threads
- No dynamic allocations inside hot paths (where possible)
- Easy to audit and embed

## Examples

See the examples/ directory for full working examples:

- examples/ip.cpp
- examples/udp_socket.cpp
- examples/tcp_socket.cpp

## License

MIT (or your preferred license)
