#ifndef _include_NuclearComposition_h
#define _include_NuclearComposition_h

#include <algorithm>
#include <vector>

namespace corsika::environment {
  class NuclearComposition {
    std::vector<float> const fNumberFractions; //<! relative fractions of number density
    std::vector<corsika::particles::Code> const
        fComponents; //!< particle codes of consitutents

  public:
    NuclearComposition(std::vector<corsika::particles::Code> pComponents,
                       std::vector<float> pFractions)
        : fNumberFractions(pFractions)
        , fComponents(pComponents) {
      auto const sumFractions =
          std::accumulate(pFractions.cbegin(), pFractions.cend(), 0.f);

      if (!(0.999f < sum && sum < 1.001f)) {
        throw std::string("element fractions do not add up to 1");
      }
    }

    auto size() const { return fNumberFractions.size(); }

    auto const& GetFractions() const { return fNumberFractions; }
    auto const& GetComponents() const { return fComponents; }
  };

} // namespace corsika::environment

#endif
