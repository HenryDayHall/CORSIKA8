#pragma once

/*
  NOTE, WARNING, ATTENTION

  The .../Random.hpp implement the hooks of external modules to the C8 random
  number generator. It has to occur excatly ONCE per linked
  executable. If you include the header below multiple times and
  link this togehter, it will fail.
 */
#include <corsika/modules/sibyll/Random.hpp>
#include <corsika/modules/epos/Random.hpp>
#include <corsika/modules/urqmd/Random.hpp>
#include <corsika/modules/qgsjetII/Random.hpp>
