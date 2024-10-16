#include <tarp.hpp>
#include <iostream>

#include <unistd.h>
#include <limits.h>

char hostname[HOST_NAME_MAX];

int main(int argc, char** argv) {
  gethostname(hostname, HOST_NAME_MAX);
  tarp::initialize(argc, argv);
  for(std::uint64_t i = 0; i < tarp::getNumHosts(); i++) {
    if(tarp::getHost() == i){
      std::cout << "HostId: " << i << "\n";
      std::cout << "HostName: " << hostname << std::endl;
    }
    tarp::hostBarrier();
  }
  tarp::finalize();
  return 0;
}
