#include <cstdint>
#include <iostream>

#include <gasnet.h>

#include <tarp.hpp>

namespace tarp {

#ifndef NDEBUG_GASNET
  constexpr bool DEBUG_GASNET = true;
#else
  constexpr bool DEBUG_GASNET = false;
#endif

  struct {
    std::string_view clientName = "tarp";
    gex_Client_t client{GEX_CLIENT_INVALID};
    gex_EP_t endpoint{GEX_EP_INVALID};
    gex_TM_t team{GEX_TM_INVALID};
  } client;

  namespace internal {
    backend world {GEX_RANK_INVALID, GEX_RANK_INVALID};
  }

  void initialize(int argc, char** argv) {
    int status;

    // Initialize Gasnet
    status = gex_Client_Init(&client.client, &client.endpoint, &client.team,
        client.clientName.data(), &argc, &argv, 0);
    if(status != GASNET_OK) std::abort();

    internal::world.rank = gasnet_mynode();
    internal::world.size = gasnet_nodes();

  }

  void finalize() {
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
  }

  void hostBarrier() {
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    gasnet_barrier_wait(0, GASNET_BARRIERFLAG_ANONYMOUS);
  }


} // tarp
