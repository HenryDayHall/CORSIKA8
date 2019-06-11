/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/stack/nuclear_extension/NuclearStackExtension.h>
#include <corsika/stack/super_stupid/SuperStupidStack.h>
#include <corsika/units/PhysicalUnits.h>

#include <boost/type_index.hpp>
using boost::typeindex::type_id_with_cvr;

using namespace corsika;
using namespace corsika::stack::nuclear_extension;
using namespace corsika::geometry;
using namespace corsika::units::si;

#include <catch2/catch.hpp>

// this is an auxiliary help typedef, which I don't know how to put
// into NuclearStackExtension.h where it belongs...
template <typename StackIter>
using ExtendedParticleInterfaceType =
    corsika::stack::nuclear_extension::NuclearParticleInterface<
        corsika::stack::super_stupid::SuperStupidStack::template PIType, StackIter>;

using ExtStack = NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack,
                                       ExtendedParticleInterfaceType>;

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
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("write nucleus") {
    NuclearStackExtension<corsika::stack::super_stupid::SuperStupidStack,
                          ExtendedParticleInterfaceType>
        s;
    s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                             corsika::stack::MomentumVector, geometry::Point,
                             units::si::TimeType, unsigned short, unsigned short>{
        particles::Code::Nucleus, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 10});
    REQUIRE(s.GetSize() == 1);
  }

  SECTION("write invalid nucleus") {
    ExtStack s;
    REQUIRE_THROWS(
        s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
            particles::Code::Nucleus, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 0, 0}));
  }

  SECTION("read non nucleus") {
    ExtStack s;
    s.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    const auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == particles::Code::Electron);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    REQUIRE(pout.GetTime() == 100_s);
  }

  SECTION("read nucleus") {
    ExtStack s;
    s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                             corsika::stack::MomentumVector, geometry::Point,
                             units::si::TimeType, unsigned short, unsigned short>{
        particles::Code::Nucleus, 1.5_GeV,
        corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9});
    const auto pout = s.GetNextParticle();
    REQUIRE(pout.GetPID() == particles::Code::Nucleus);
    REQUIRE(pout.GetEnergy() == 1.5_GeV);
    REQUIRE(pout.GetTime() == 100_s);
    REQUIRE(pout.GetNuclearA() == 10);
    REQUIRE(pout.GetNuclearZ() == 9);
  }

  SECTION("read invalid nucleus") {
    ExtStack s;
    s.AddParticle(
        std::tuple<particles::Code, units::si::HEPEnergyType,
                   corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
    const auto pout = s.GetNextParticle();
    REQUIRE_THROWS(pout.GetNuclearA());
    REQUIRE_THROWS(pout.GetNuclearZ());
  }

  SECTION("stack fill and cleanup") {

    ExtStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
            particles::Code::Nucleus, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2});
      } else {
        s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType>{
            particles::Code::Electron, 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      }
    }

    REQUIRE(s.GetSize() == 99);
    for (int i = 0; i < 99; ++i) s.GetNextParticle().Delete();
    REQUIRE(s.GetSize() == 0);
  }

  SECTION("stack operations") {

    ExtStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType, unsigned short, unsigned short>{
            particles::Code::Nucleus, i * 15_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2});
      } else {
        s.AddParticle(std::tuple<particles::Code, units::si::HEPEnergyType,
                                 corsika::stack::MomentumVector, geometry::Point,
                                 units::si::TimeType>{
            particles::Code::Electron, i * 1.5_GeV,
            corsika::stack::MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s});
      }
    }

    // copy
    {
      s.Copy(s.begin() + 9, s.begin() + 10); // nuclei to non-nuclei
      const auto& p9 = s.cbegin() + 9;
      const auto& p10 = s.cbegin() + 10;

      REQUIRE(p9.GetPID() == particles::Code::Nucleus);
      REQUIRE(p9.GetEnergy() == 9 * 15_GeV);
      REQUIRE(p9.GetTime() == 100_s);
      REQUIRE(p9.GetNuclearA() == 9);
      REQUIRE(p9.GetNuclearZ() == 9 / 2);

      REQUIRE(p10.GetPID() == particles::Code::Nucleus);
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

      REQUIRE(p9.GetPID() == particles::Code::Electron);
      REQUIRE(p9.GetEnergy() == 93 * 1.5_GeV);
      REQUIRE(p9.GetTime() == 100_s);

      REQUIRE(p93.GetPID() == particles::Code::Electron);
      REQUIRE(p93.GetEnergy() == 93 * 1.5_GeV);
      REQUIRE(p93.GetTime() == 100_s);
    }

    // swap
    {
      s.Swap(s.begin() + 11, s.begin() + 10);
      const auto& p11 = s.cbegin() + 11; // now: nucleus
      const auto& p10 = s.cbegin() + 10; // now: electron

      REQUIRE(p11.GetPID() == particles::Code::Nucleus);
      REQUIRE(p11.GetEnergy() == 9 * 15_GeV);
      REQUIRE(p11.GetTime() == 100_s);
      REQUIRE(p11.GetNuclearA() == 9);
      REQUIRE(p11.GetNuclearZ() == 9 / 2);

      REQUIRE(p10.GetPID() == particles::Code::Electron);
      REQUIRE(p10.GetEnergy() == 11 * 1.5_GeV);
      REQUIRE(p10.GetTime() == 100_s);
    }

    // swap two nuclei
    {
      s.Swap(s.begin() + 29, s.begin() + 59);
      const auto& p29 = s.cbegin() + 29;
      const auto& p59 = s.cbegin() + 59;

      REQUIRE(p29.GetPID() == particles::Code::Nucleus);
      REQUIRE(p29.GetEnergy() == 59 * 15_GeV);
      REQUIRE(p29.GetTime() == 100_s);
      REQUIRE(p29.GetNuclearA() == 59);
      REQUIRE(p29.GetNuclearZ() == 59 / 2);

      REQUIRE(p59.GetPID() == particles::Code::Nucleus);
      REQUIRE(p59.GetEnergy() == 29 * 15_GeV);
      REQUIRE(p59.GetTime() == 100_s);
      REQUIRE(p59.GetNuclearA() == 29);
      REQUIRE(p59.GetNuclearZ() == 29 / 2);
    }

    for (int i = 0; i < 99; ++i) s.DeleteLast();
    REQUIRE(s.GetSize() == 0);
  }
}
