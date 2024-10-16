#ifndef _TARP_HPP_
#define _TARP_HPP_

#include <cstdint>
#include <gasnet.h>

namespace tarp {
  struct backend {
    std::uint64_t rank;
    std::uint64_t size;
  };

  struct globalPointer {
#pragma pack(push, 1)
    std::uint16_t header  : 16;
    std::uint64_t pointer : 48;
#pragma pack(pop)

    bool isLocal() const {
      return header == 0x0;
    }

    void* getLocal() const {
      return reinterpret_cast<void*>(pointer);
    }
  };

  union pointer {
    void* pointer;
    globalPointer global;
  };

  namespace internal {
    extern backend world;
  }
  void initialize(int argc, char** argv);
  void finalize();

  inline uint64_t getHost() {
    return internal::world.rank;
  }

  inline uint64_t getNumHosts() {
    return internal::world.size;
  }

  void hostBarrier();

  constexpr uint16_t createHostMask(uint16_t hostID){
    return 0xFFFF - hostID;
  }

  constexpr uint16_t getHostFromMask(uint16_t maskID) {
    return 0xFFFF - maskID;
  }


  uint64_t getHostFromMaskedPointer(void* ptr){
    pointer p;
    p.pointer = ptr;
    return getHostFromMask(p.global.header);
  }

  void* getLocalPointer(const globalPointer& global){
    return global.getLocal();
  }

  void* globalify(void* ptr) {
    pointer p;
    p.pointer = ptr;
    p.global.header = createHostMask(uint16_t(getHost()));
    return p.pointer;
  }

  void* deglobalify(void* ptr) {
    pointer p;
    p.pointer = ptr;
    return p.global.getLocal();
  }

  /*
   * Local accesses have the semantics of normal load stores
   */
  template<typename T>
  T load(T* ptr) {
    constexpr std::size_t SIZE = sizeof(T);
    pointer p;
    p.pointer = (void*) ptr;
    if(p.global.isLocal() || p.global.header == getHost()) {
      return *reinterpret_cast<T*>(p.pointer);
    } else if constexpr (SIZE <= SIZEOF_GASNET_REGISTER_VALUE_T) {
      gasnet_register_value_t val = gasnet_get_val(getHostFromMask(p.header), ptr, SIZE);
      return *reinterpret_cast<T*>(&val)
    } else {
      T ret;
      gasnet_get(&ret, getHostFromMask(p.header), p.getLocal(), SIZE);
      return ret;
    }
  }

  template<typename T>
  void store(T val, T* ptr) {
    constexpr std::size_t SIZE = sizeof(T);
    pointer p;
    p.pointer = (void*) ptr;
    if(p.global.isLocal() || p.global.header == getHost()) {
      *reinterpret_cast<T*>(p.global.getLocal()) =  val;
    } else if constexpr (SIZE <= SIZEOF_GASNET_REGISTER_VALUE_T) {
      gasnet_register_value_t reg_val = *reinterpret_cast<gasnet_register_value_t*>(&val);
      gasnet_put_val(getHostFromMask(p.header), p.getLocal(), reg_val, SIZE);
    } else {
      gasnet_put(getHostFromMask(p.header), p.getLocal(), &val, SIZE);
    }
  }


} // tarp

#define TARP_GENERATE_LOAD_STORE_GLOBAL_DEGLOBAL(x) \
  x* _tarp_globalify_##x(x* ptr) { \
    return (x*) tarp::globalify((void*) ptr); \
  } \
  x* _tarp_deglobalify_##x(x* ptr) { \
    return (x*) tarp::deglobalify((void*) ptr); \
  } \
  void _tarp_store_##x(x val, x* ptr) { \
    tarp::store<x>(val, ptr); \
  } \
  x _tarp_load_##x(x* ptr) { \
    return tarp::load<x>(ptr); \
  }

extern "C" {
  TARP_GENERATE_LOAD_STORE_GLOBAL_DEGLOBAL(uint64_t)
  TARP_GENERATE_LOAD_STORE_GLOBAL_DEGLOBAL(int64_t)
  TARP_GENERATE_LOAD_STORE_GLOBAL_DEGLOBAL(int64_t)

}

#endif // _TARP_HPP_
