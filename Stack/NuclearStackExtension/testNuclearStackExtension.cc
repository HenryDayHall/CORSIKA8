/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>
#include <corsika/units/PhysicalUnits.h>

using namespace corsika;
using namespace corsika::stack::nuclear_extension;
using namespace corsika::geometry;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

#include <iostream>
using namespace std;

TEST_CASE("NuclearStackExtension", "[stack]") {

  geometry::CoordinateSystem& dummyCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  SECTION("write non nucleus") {
    NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;
    s.AddParticle(
        std::make_tuple(particles::Code::Electron, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    CHECK(s.getEntries() == 1);
  }

  SECTION("write nucleus") {
    NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;
    s.AddParticle(std::make_tuple(
        particles::Code::Nucleus, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 10));
    CHECK(s.getEntries() == 1);
  }

  SECTION("write invalid nucleus") {
    ParticleDataStack s;
    CHECK_THROWS(s.AddParticle(
        std::make_tuple(particles::Code::Nucleus, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 0, 0)));
  }

  SECTION("read non nucleus") {
    ParticleDataStack s;
    s.AddParticle(
        std::make_tuple(particles::Code::Electron, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    const auto pout = s.GetNextParticle();
    CHECK(pout.GetPID() == particles::Code::Electron);
    CHECK(pout.GetEnergy() == 1.5_GeV);
    CHECK(pout.GetTime() == 100_s);
  }

  SECTION("read nucleus") {
    ParticleDataStack s;
    s.AddParticle(
        std::make_tuple(particles::Code::Nucleus, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));
    const auto pout = s.GetNextParticle();
    CHECK(pout.GetPID() == particles::Code::Nucleus);
    CHECK(pout.GetEnergy() == 1.5_GeV);
    CHECK(pout.GetTime() == 100_s);
    CHECK(pout.GetNuclearA() == 10);
    CHECK(pout.GetNuclearZ() == 9);
  }

  SECTION("read invalid nucleus") {
    ParticleDataStack s;
    s.AddParticle(
        std::make_tuple(particles::Code::Electron, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    const auto pout = s.GetNextParticle();
    CHECK_THROWS(pout.GetNuclearA());
    CHECK_THROWS(pout.GetNuclearZ());
  }

  SECTION("stack fill and cleanup") {

    ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(std::make_tuple(
            particles::Code::Nucleus, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2));
      } else {
        s.AddParticle(std::make_tuple(
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
      }
    }

    CHECK(s.getEntries() == 99);
    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();
    CHECK(s.getEntries() == 0);
  }

  SECTION("stack operations") {

    ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(std::make_tuple(
            particles::Code::Nucleus, i * 15_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2));
      } else {
        s.AddParticle(std::make_tuple(
            particles::Code::Electron, i * 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
      }
    }

    // copy
    {
      s.Copy(s.begin() + 9, s.begin() + 10); // nuclei to non-nuclei
      const auto& p9 = s.cbegin() + 9;
      const auto& p10 = s.cbegin() + 10;

      CHECK(p9.GetPID() == particles::Code::Nucleus);
      CHECK(p9.GetEnergy() == 9 * 15_GeV);
      CHECK(p9.GetTime() == 100_s);
      CHECK(p9.GetNuclearA() == 9);
      CHECK(p9.GetNuclearZ() == 9 / 2);

      CHECK(p10.GetPID() == particles::Code::Nucleus);
      CHECK(p10.GetEnergy() == 9 * 15_GeV);
      CHECK(p10.GetTime() == 100_s);
      CHECK(p10.GetNuclearA() == 9);
      CHECK(p10.GetNuclearZ() == 9 / 2);
    }

    // copy
    {
      s.Copy(s.begin() + 93, s.begin() + 9); // non-nuclei to nuclei
      const auto& p93 = s.cbegin() + 93;
      const auto& p9 = s.cbegin() + 9;

      CHECK(p9.GetPID() == particles::Code::Electron);
      CHECK(p9.GetEnergy() == 93 * 1.5_GeV);
      CHECK(p9.GetTime() == 100_s);

      CHECK(p93.GetPID() == particles::Code::Electron);
      CHECK(p93.GetEnergy() == 93 * 1.5_GeV);
      CHECK(p93.GetTime() == 100_s);
    }

    // swap
    {
      s.Swap(s.begin() + 11, s.begin() + 10);
      const auto& p11 = s.cbegin() + 11; // now: nucleus
      const auto& p10 = s.cbegin() + 10; // now: electron

      CHECK(p11.GetPID() == particles::Code::Nucleus);
      CHECK(p11.GetEnergy() == 9 * 15_GeV);
      CHECK(p11.GetTime() == 100_s);
      CHECK(p11.GetNuclearA() == 9);
      CHECK(p11.GetNuclearZ() == 9 / 2);

      CHECK(p10.GetPID() == particles::Code::Electron);
      CHECK(p10.GetEnergy() == 11 * 1.5_GeV);
      CHECK(p10.GetTime() == 100_s);
    }

    // swap two nuclei
    {
      s.Swap(s.begin() + 29, s.begin() + 59);
      const auto& p29 = s.cbegin() + 29;
      const auto& p59 = s.cbegin() + 59;

      CHECK(p29.GetPID() == particles::Code::Nucleus);
      CHECK(p29.GetEnergy() == 59 * 15_GeV);
      CHECK(p29.GetTime() == 100_s);
      CHECK(p29.GetNuclearA() == 59);
      CHECK(p29.GetNuclearZ() == 59 / 2);

      CHECK(p59.GetPID() == particles::Code::Nucleus);
      CHECK(p59.GetEnergy() == 29 * 15_GeV);
      CHECK(p59.GetTime() == 100_s);
      CHECK(p59.GetNuclearA() == 29);
      CHECK(p59.GetNuclearZ() == 29 / 2);
    }

    for (int i = 0; i < 99; ++i) s.last().Delete();
    CHECK(s.getEntries() == 0);
  }
}
