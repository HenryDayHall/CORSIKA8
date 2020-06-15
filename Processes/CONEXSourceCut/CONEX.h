#pragma once
#include <array>

extern "C" {
int InitialParticle_(int const&);

struct cxoptl_ {
  std::array<double, 16> dptl;
};
}
