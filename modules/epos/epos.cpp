#include <iostream>

#include <epos.hpp>
#include <rng_impl.hpp>

IMPLEMENT_RNG(epos)

namespace epos {

  // this is needed as linker object, but it is not needed to do anything
  void ranfst_(int&) {}

  // this is needed as linker object, but it is not needed to do anything
  void ranfgt_(int&) {}

  // this is needed as linker object, but it is not needed to do anything
  void rmmaqd_(int[3], int&, char*, int) {}

  // this is needed as linker object, but it is not needed to do anything
  void ranfini_(double&, int&, int&) {}

  // this is needed as linker object, but it is not needed to do anything
  void ranfcv_(double&) {} // LCOV_EXCL_LINE

  void rmmard_(double rvec[], int const& lenv, int& /*iseq*/) {
    // we ignore iseq and draw all numbers from same C8 sequence
    rng_ptr(rvec, lenv);
  }

  float rangen_() {
    float f{};
    do { f = draw_std_rnd(); } while (f == 1.0f);
    return f;
  }

  double drangen_() { return draw_std_rnd(); }

  datadir::datadir(const std::string& dir) {
    if (dir.length() > 500) { // we don't test this limitation: LCOV_EXCL_START
      std::cerr << "Epos error, will cut datadir \"" << dir
                << "\" to 500 characters: " << std::endl;
    } // LCOV_EXCL_STOP
    int i = 0;
    for (i = 0; i < std::min(500, int(dir.length())); ++i) data[i] = dir[i];
    data[i + 0] = ' ';
    data[i + 1] = '\0';
    length = dir.length();
  }
} // namespace epos
