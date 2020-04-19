#include <corsika/utl/CorsikaData.h>

#include <cstdlib>
#include <stdexcept>
#include <string>

std::filesystem::path corsika::utl::CorsikaData(std::filesystem::path const& key) {
  if (auto const* p = std::getenv("CORSIKA_DATA"); p != nullptr) {
    auto const path = std::filesystem::path(p) / key;
    return path;
  } else {
    throw std::runtime_error("CORSIKA_DATA not set");
  }
}
