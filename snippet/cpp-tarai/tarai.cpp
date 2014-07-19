// g++ -std=c++11 -O3 tarai.cpp && ./a.out
#include <chrono>
#include <iostream>
#include <cstdint>

// http://en.wikipedia.org/wiki/Tak_(function)
int tarai(int x, int y, int z) {
  return (y >= x) ? y : tarai(tarai(x-1,y,z), tarai(y-1,z,x), tarai(z-1,x,y));
}

int main() {
  // http://stackoverflow.com/a/15755865/2132223
  typedef std::chrono::high_resolution_clock Clock;
  typedef std::chrono::duration<double> ClockSec;
  const auto t0 = Clock::now();

  std::cout << "tarai=" << tarai(14, 10, 0) << std::endl;

  const auto t1 = Clock::now();
  const auto dt = static_cast<ClockSec>(t1 - t0);
  std::cout << dt.count() << "s" << std::endl;
}
