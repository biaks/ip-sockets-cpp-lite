# ip-sockets-cpp-lite 🔌

**Simplicity. Lightweight. Cross-platform.**

ip-sockets-cpp-lite is a fast, header-only, dependency-free, cross-platform C++ library that makes programming UDP/TCP sockets and working with IPv4/IPv6 addresses as easy as possible.
Forget about endless `getaddrinfo` calls, confusion with `sockaddr_in`/`sockaddr_in6`, and platform-specific workarounds.

```cpp
// Send a UDP packet? One line!
udp_socket_t<v4, client> sock;
sock.open("192.168.1.100:8080");
sock.send("Hello!", 6);
```

## Why ip-sockets-cpp-lite? ✨

- **📦 Header-only** - just copy the headers into your project and start coding
- **🎯 Zero dependencies** - only the C++ standard library
- **🖥️ Cross-platform** - Linux, Windows, macOS - same code everywhere
- **🚀 Minimal learning curve** - up and running in 5 minutes
- **🪶 Lightweight** - no magic, just what you need
- **🔒 Predictable** - full control over everything
- **📋 Human-readable errors** - clear error codes instead of cryptic errno values

## Quick Start 🏁

### 1. Installation

Simply copy three files into your project:
- `ip_address.h` — IP address handling
- `udp_socket.h` — UDP sockets
- `tcp_socket.h` — TCP sockets

Then include one of what you need:
```cpp
#include "ip_address.h"  // work only with ipv4/ipv6 addresses
#include "udp_socket.h"  // work with UDP ipv4/ipv6 client/server sockets
#include "tcp_socket.h"  // work with TCP ipv4/ipv6 client/server sockets
```

### 2. Send a UDP message

```cpp
#include "udp_socket.h"
using namespace ipsockets;

int main() {

    // Create a UDP client (IPv4)
    udp_socket_t<v4, socket_type_e::client> sock;
    
    // Connect to server
    if (sock.open("192.168.1.100:8080") == no_error) {

        // Send a message
        sock.send("Hello, world!", 13);
    
        // Receive response
        char buffer[1024];
        int bytes = sock.recv(buffer, sizeof(buffer));
        if (bytes > 0)
            std::cout << "Received: " << std::string(buffer, bytes) << '\n';

    }
    
    return 0;
}
```

### 3. Simple UDP server

```cpp
#include "udp_socket.h"
using namespace ipsockets;

int main() {

    // Create a UDP server on port 8080
    udp_socket_t<v4, socket_type_e::server> server;
    
    if (server.open("0.0.0.0:8080") != no_error) {

        std::cout << "Server listening on port 8080\n";
    
        char buffer[1024];
        addr4_t client;
    
        while (true) {
            // Wait for client message
            int bytes = server.recvfrom(buffer, sizeof(buffer), client);
            if (bytes > 0) {
                std::cout << "Received from " << client << ": " 
                          << std::string(buffer, bytes) << '\n';
            
                // Reply to the same client
                server.sendto("OK", 2, client);
            }
        }
    }
}
```

### 4. TCP echo server in 20 lines

```cpp
#include "tcp_socket.h"
using namespace ipsockets;

int main() {
    // Create a TCP server
    tcp_socket_t<v4, socket_type_e::server> server;
    
    if (server.open("0.0.0.0:8080") != no_error) {

        std::cout << "TCP server listening on port 8080\n";
    
        while (true) {
            addr4_t client_addr;
        
            // Accept incoming connection
            auto client = server.accept(client_addr);
            if (client.state != state_e::state_opened)
                continue;
        
            std::cout << "Client connected: " << client_addr << '\n';
        
            // Communicate with client
            char buffer[1024];
            int bytes = client.recv(buffer, sizeof(buffer));
            if (bytes > 0)
                client.send(buffer, bytes);  // echo back
        
            client.close();  // close connection
        }
    }
}
```

## Why is this convenient? 🤔

### Instead of this (raw sockets):
```cpp
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr);
// ... and 20 more lines to send a single packet
```

### Write this:
```cpp
sock.open("192.168.1.100:8080");  // done!
sock.send("Hello", 5);
```

## Features 🔧

### IP Address Handling (`ip_address.h`)
- Unified IPv4 and IPv6 support
- String to binary and back conversion
- Network prefixes and masks
- IPv4-over-IPv6 mapping
- Address + port as a single unit

### UDP Sockets (`udp_socket.h`)
- Client and server modes
- Configurable timeouts
- Polling-based asynchronous I/O
- Clear states and error codes

### TCP Sockets (`tcp_socket.h`)
- Blocking I/O with timeouts
- Convenient accept with client socket creation
- Automatic connection management
- Consistent API with UDP

## Requirements 📋

- C++11 or newer
- C++ standard library
- Nothing else!

## Project Integration 🛠️

### Option 1 — Just copy the files
```bash
cp ip_address.h udp_socket.h tcp_socket.h /path/to/your/project/
```

### Option 2 — CMake
```cmake
add_subdirectory(ip-sockets-cpp-lite)
target_link_libraries(your_app ip-sockets-cpp-lite)
```

## Examples 📚

Check out the `examples/` directory for complete working examples:
- `ip_address.cpp` - all IP address manipulation features
- `udp_socket.cpp` - UDP client-server interaction
- `tcp_socket.cpp` - TCP client-server interaction
- `resolve_host.cpp` - resolving host to ipv4/ipv6 address example

## License 📄

MIT License - use it anywhere, even in commercial projects.

---

**ip-sockets-cpp-lite** - sockets that don't scare you. Try it, and you'll be amazed at how little code you need for network programming in C++! 🚀
