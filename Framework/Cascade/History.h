#include <memory>
#include <utility>
#include <variant>

using ParticleCode = std::variant<corsika::particles::ParticleCode, std::pair<int, int>>;

class SecondaryParticle {
  corsika::units::si::HEPEnergyType const energy_;
  corsika::geometry::Vector<corsika::units::si::hepmomentum_d> const momentum_;
  ParticleCode const pid_;

  // what else...?
  // - polarization

public:
  SecondaryParticle(energy, momentum, pid)
      : energy_{energy}
      , momentum_{momentum}
      , pid_{pid} {}
};

class Event; // forward declaration
using EventPtr = std::shared_ptr<Event>;

struct ProjectileParticle {
  std::pair<EventPtr, size_t> origin;
  corsika::geometry::Point position;
  coriska::units::si::TimeType time;
  corsika::geometry::Vector<corsika::units::si::hepmomentum_d> momentum;
  corsika::units::si::HEPEnergyType energy;

  // weight
};

struct Event {
  corsika::particles::ParticleCode targetCode; // cannot be const...
  ProjectileParticle projectile;

  std::vector<SecondaryParticle> secondaries;
};

template <typename TStackView>
class EventBuilder {
  TStackView& stackView_;
  EventPtr event_;

public:
  EventBuilder(TStackView& view)
      : stackView_{stackView}
      , event_{std::make_shared<Event>()} {}

  template <typename... Args>
  void AddSecondary(Args&&... args) {
    auto const s = stackView.AddSecondary(std::forward<Args>(args), event);
    event->secondaries.emplace_back(s.GetEnergy(), s.GetMomentum(), s.GetParticleID());
  }

  void SetProjectile() {}
  void SetTarget(corsika::particles::ParticleCode targetCode) {
    event_->targetCode = targetCode;
  }
};
