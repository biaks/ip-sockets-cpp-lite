
#pragma once

/* Get from: https://stackoverflow.com/questions/809902/64-bit-ntohl-in-c */

#include <type_traits>

#ifdef _MSC_VER
  /* detect Little-Endian or Big-Endian */
  # define __BYTE_ORDER       (*(const int *)"\x01\x02\x03\x04")
  # define __LITTLE_ENDIAN    0x04030201
  # define __BIG_ENDIAN       0x01020304
  # define __PDP_ENDIAN       0x02010403
#else
  # include <endian.h>   // __BYTE_ORDER __LITTLE_ENDIAN
#endif /* _MSC_VER */


#ifdef _MSC_VER

  #include <stdlib.h>
  #define bswap_16(x) _byteswap_ushort(x)
  #define bswap_32(x) _byteswap_ulong(x)
  #define bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

  // Mac OS X / Darwin features
  #include <libkern/OSByteOrder.h>
  #define bswap_16(x) OSSwapInt16(x)
  #define bswap_32(x) OSSwapInt32(x)
  #define bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

  #include <sys/byteorder.h>
  #define bswap_16(x) BSWAP_16(x)
  #define bswap_32(x) BSWAP_32(x)
  #define bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

  #include <sys/endian.h>
  #define bswap_16(x) bswap16(x)
  #define bswap_32(x) bswap32(x)
  #define bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

  #include <sys/types.h>
  #define bswap_16(x) swap16(x)
  #define bswap_32(x) swap32(x)
  #define bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

  #include <sys/types.h>
  #include <machine/bswap.h>
  #if defined(__BSWAP_RENAME) && !defined(__bswap_32)
    #define bswap_16(x) bswap16(x)
    #define bswap_32(x) bswap32(x)
    #define bswap_64(x) bswap64(x)
  #endif

  #else

  #include <byteswap.h>

#endif

namespace common {

  // from BIG ENDIAN to LITTLE ENDIAN
  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 1, T>
  ntohT (const T& value) {
    return value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 2, T>
  ntohT (const T& value) {
    return (__BYTE_ORDER == __LITTLE_ENDIAN) ? bswap_16(value) : value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 4, T>
  ntohT (const T& value) {
    return (__BYTE_ORDER == __LITTLE_ENDIAN) ? bswap_32(value) : value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 8, T>
  ntohT (const T& value) {
    return (__BYTE_ORDER == __LITTLE_ENDIAN) ? bswap_64(value) : value;
  }

  // from LITTLE ENDIAN to BIG ENDIAN
  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 1, T>
  htonT (const T& value) {
    return value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 2, T>
  htonT (const T& value) {
    return (__BYTE_ORDER != __BIG_ENDIAN) ? bswap_16(value) : value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 4, T>
  htonT (const T& value) {
    return (__BYTE_ORDER != __BIG_ENDIAN) ? bswap_32(value) : value;
  }

  template <typename T>
  inline typename std::enable_if_t<sizeof(T) == 8, T>
  htonT (const T& value) {
    return (__BYTE_ORDER != __BIG_ENDIAN) ? bswap_64(value) : value;
  }

} /* common:: */

