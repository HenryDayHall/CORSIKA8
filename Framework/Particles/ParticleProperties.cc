#include <corsika/particles/ParticleProperties.h>

namespace corsika::particles::io {

  std::ostream& operator<<(std::ostream& stream, Code const p) {
    return stream << GetName(p);
  }

} // namespace corsika::particles::io
