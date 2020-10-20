/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/stack/NuclearStackExtension.hpp>
#include <corsika/stack/SuperStupidStack.hpp>

using namespace corsika;
using namespace corsika::nuclear_extension;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

// this is an auxiliary help typedef, which I don't know how to put
// into NuclearStackExtension.h where it belongs...
template <typename StackIter>
using ExtendedParticleInterfaceType =
    corsika::nuclear_extension::NuclearParticleInterface<
        corsika::super_stupid::SuperStupidStack::template PIType, StackIter>;

using ExtStack = NuclearStackExtension<corsika::super_stupid::SuperStupidStack,
                                       ExtendedParticleInterfaceType>;

#include <iostream>
using namespace std;

TEST_CASE("NuclearStackExtension", "[stack]") {

  CoordinateSystem& dummyCS =
      RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

  SECTION("write non nucleus") {
    NuclearStackExtension<corsika::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;
    s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                             Point, units::si::TimeType>{
        Code::Electron, 1.5_GeV, corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("write nucleus") {
    NuclearStackExtension<corsika::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;
    s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                             Point, units::si::TimeType, unsigned short, unsigned short>{
        Code::Nucleus, 1.5_GeV, corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 10});
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("write invalid nucleus") {
    ExtStack s;
    REQUIRE_THROWS(s.AddParticle(
        std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector, Point,
                   units::si::TimeType, unsigned short, unsigned short>{
            Code::Nucleus, 1.5_GeV,
            corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 0, 0}));
  }

  SECTION("read non nucleus") {
    ExtStack s;
    s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                             Point, units::si::TimeType>{
        Code::Electron, 1.5_GeV, corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    const auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == Code::Electron);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    REQUIRE(pout.GetTime() == 100_s);
  }

  SECTION("read nucleus") {
    ExtStack s;
    s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                             Point, units::si::TimeType, unsigned short, unsigned short>{
        Code::Nucleus, 1.5_GeV, corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9});
    const auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == Code::Nucleus);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    REQUIRE(pout.GetTime() == 100_s);
    REQUIRE(pout.GetNuclearA() == 10);
    REQUIRE(pout.GetNuclearZ() == 9);
  }

  SECTION("read invalid nucleus") {
    ExtStack s;
    s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                             Point, units::si::TimeType>{
        Code::Electron, 1.5_GeV, corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    const auto pout = s.GetNextParticle();
    CHECK_THROWS(pout.GetNuclearA());
    CHECK_THROWS(pout.GetNuclearZ());
  }

  SECTION("stack fill and cleanup") {

    ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(
            std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector, Point,
                       units::si::TimeType, unsigned short, unsigned short>{
                Code::Nucleus, 1.5_GeV,
                corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2});
      } else {
        s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                                 Point, units::si::TimeType>{
            Code::Electron, 1.5_GeV,
            corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      }
    }

    CHECK(s.getEntries() == 99);
    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();
    CHECK(s.getEntries() == 0);
  }

  SECTION("stack operations") {

    ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    // i=9, 19, 29, etc. are nuclei
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(
            std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector, Point,
                       units::si::TimeType, unsigned short, unsigned short>{
                Code::Nucleus, i * 15_GeV,
                corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2});
      } else {
        s.AddParticle(std::tuple<Code, units::si::HEPEnergyType, corsika::MomentumVector,
                                 Point, units::si::TimeType>{
            Code::Electron, i * 1.5_GeV,
            corsika::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      }
    }

    // copy
    {
      s.Copy(s.begin() + 9, s.begin() + 10); // nuclei to non-nuclei
      const auto& p9 = s.cbegin() + 9;
      const auto& p10 = s.cbegin() + 10;

      REQUIRE(p9.GetPID() == Code::Nucleus);
      REQUIRE(p9.GetEnergy() == 9 * 15_GeV);
      REQUIRE(p9.GetTime() == 100_s);
      REQUIRE(p9.GetNuclearA() == 9);
      REQUIRE(p9.GetNuclearZ() == 9 / 2);

      REQUIRE(p10.GetPID() == Code::Nucleus);
      REQUIRE(p10.GetEnergy() == 9 * 15_GeV);
      REQUIRE(p10.GetTime() == 100_s);
      REQUIRE(p10.GetNuclearA() == 9);
      REQUIRE(p10.GetNuclearZ() == 9 / 2);
    }

    // copy
    {
      s.Copy(s.begin() + 93, s.begin() + 9); // non-nuclei to nuclei
      const auto& p93 = s.cbegin() + 93;
      const auto& p9 = s.cbegin() + 9;

      REQUIRE(p9.GetPID() == Code::Electron);
      REQUIRE(p9.GetEnergy() == 93 * 1.5_GeV);
      REQUIRE(p9.GetTime() == 100_s);

      REQUIRE(p93.GetPID() == Code::Electron);
      REQUIRE(p93.GetEnergy() == 93 * 1.5_GeV);
      REQUIRE(p93.GetTime() == 100_s);
    }

    // swap
    {
      s.Swap(s.begin() + 11, s.begin() + 10);
      const auto& p11 = s.cbegin() + 11; // now: nucleus
      const auto& p10 = s.cbegin() + 10; // now: electron

      REQUIRE(p11.GetPID() == Code::Nucleus);
      REQUIRE(p11.GetEnergy() == 9 * 15_GeV);
      REQUIRE(p11.GetTime() == 100_s);
      REQUIRE(p11.GetNuclearA() == 9);
      REQUIRE(p11.GetNuclearZ() == 9 / 2);

      REQUIRE(p10.GetPID() == Code::Electron);
      REQUIRE(p10.GetEnergy() == 11 * 1.5_GeV);
      REQUIRE(p10.GetTime() == 100_s);
    }

    // swap two nuclei
    {
      s.Swap(s.begin() + 29, s.begin() + 59);
      const auto& p29 = s.cbegin() + 29;
      const auto& p59 = s.cbegin() + 59;

      REQUIRE(p29.GetPID() == Code::Nucleus);
      REQUIRE(p29.GetEnergy() == 59 * 15_GeV);
      REQUIRE(p29.GetTime() == 100_s);
      REQUIRE(p29.GetNuclearA() == 59);
      REQUIRE(p29.GetNuclearZ() == 59 / 2);

      REQUIRE(p59.GetPID() == Code::Nucleus);
      REQUIRE(p59.GetEnergy() == 29 * 15_GeV);
      REQUIRE(p59.GetTime() == 100_s);
      REQUIRE(p59.GetNuclearA() == 29);
      REQUIRE(p59.GetNuclearZ() == 29 / 2);
    }

    for (int i = 0; i < 99; ++i) s.last().Delete();
    CHECK(s.getEntries() == 0);
  }

  SECTION("not allowed") {
    NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;

    // not valid:
    CHECK_THROWS(s.AddParticle(std::make_tuple(
        particles::Code::Oxygen, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 16, 8)));

    // valid
    auto particle = s.AddParticle(
        std::make_tuple(particles::Code::Nucleus, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));

    // not valid
    CHECK_THROWS(particle.AddSecondary(std::make_tuple(
        particles::Code::Oxygen, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 16, 8)));

    // add a another nucleus, so there are two now
    s.AddParticle(
        std::make_tuple(particles::Code::Nucleus, 1.5_GeV,
                        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
                        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));

    // not valid, since end() is not a valid entry
    CHECK_THROWS(s.Swap(s.begin(), s.end()));
  }
}
