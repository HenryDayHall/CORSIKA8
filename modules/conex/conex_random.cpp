#include <rng_impl.hpp>

IMPLEMENT_RNG(conex)

extern "C" void rmmard_(double field[], int const* N, int*) {
  for (int i = 0; i < *N; ++i) { field[i] = draw_std_rnd(); }
}
