/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/geometry/RootCoordinateSystem.hpp>
#include <corsika/stack/NuclearStackExtension.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

using namespace corsika;

#include <catch2/catch.hpp>

#include <iostream>
using namespace std;

TEST_CASE("NuclearStackExtension", "[stack]") {

  CoordinateSystemPtr const& dummyCS = get_root_CoordinateSystem();

  SECTION("write non nucleus") {
    nuclear_stack::NuclearStackExtension<simple_stack::SuperStupidStack,
                                         nuclear_stack::ExtendedParticleInterfaceType>
        s;
    s.addParticle(std::make_tuple(
        Code::Electron, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    CHECK(s.getEntries() == 1);
  }

  SECTION("write nucleus") {
    nuclear_stack::NuclearStackExtension<simple_stack::SuperStupidStack,
                                         nuclear_stack::ExtendedParticleInterfaceType>
        s;
    s.addParticle(std::make_tuple(
        Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 10));
    CHECK(s.getEntries() == 1);
  }

  SECTION("write invalid nucleus") {
    nuclear_stack::ParticleDataStack s;
    CHECK_THROWS(s.addParticle(std::make_tuple(
        Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 0, 0)));
  }

  SECTION("read non nucleus") {
    nuclear_stack::ParticleDataStack s;
    s.addParticle(std::make_tuple(
        Code::Electron, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    const auto pout = s.getNextParticle();
    CHECK(pout.getPID() == Code::Electron);
    CHECK(pout.getEnergy() == 1.5_GeV);
    CHECK(pout.getTime() == 100_s);
  }

  SECTION("read nucleus") {
    nuclear_stack::ParticleDataStack s;
    s.addParticle(std::make_tuple(
        Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));
    const auto pout = s.getNextParticle();
    CHECK(pout.getPID() == Code::Nucleus);
    CHECK(pout.getEnergy() == 1.5_GeV);
    CHECK(pout.getTime() == 100_s);
    CHECK(pout.getNuclearA() == 10);
    CHECK(pout.getNuclearZ() == 9);
  }

  SECTION("read invalid nucleus") {
    nuclear_stack::ParticleDataStack s;
    s.addParticle(std::make_tuple(
        Code::Electron, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
    const auto pout = s.getNextParticle();
    CHECK_THROWS(pout.getNuclearA());
    CHECK_THROWS(pout.getNuclearZ());
  }

  SECTION("stack fill and cleanup") {

    nuclear_stack::ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.addParticle(std::make_tuple(
            Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2));
      } else {
        s.addParticle(std::make_tuple(
            Code::Electron, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
      }
    }

    CHECK(s.getEntries() == 99);
    for (int i = 0; i < 99; ++i) s.getNextParticle().erase();
    CHECK(s.getEntries() == 0);
  }

  SECTION("stack operations") {

    nuclear_stack::ParticleDataStack s;
    // add 99 particles, each 10th particle is a nucleus with A=i and Z=A/2!
    // i=9, 19, 29, etc. are nuclei
    for (int i = 0; i < 99; ++i) {
      if ((i + 1) % 10 == 0) {
        s.addParticle(std::make_tuple(
            Code::Nucleus, i * 15_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, i, i / 2));
      } else {
        s.addParticle(std::make_tuple(
            Code::Electron, i * 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
            Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s));
      }
    }

    // copy
    {
      s.copy(s.begin() + 9, s.begin() + 10); // nuclei to non-nuclei
      const auto& p9 = s.cbegin() + 9;
      const auto& p10 = s.cbegin() + 10;

      CHECK(p9.getPID() == Code::Nucleus);
      CHECK(p9.getEnergy() == 9 * 15_GeV);
      CHECK(p9.getTime() == 100_s);
      CHECK(p9.getNuclearA() == 9);
      CHECK(p9.getNuclearZ() == 9 / 2);

      CHECK(p10.getPID() == Code::Nucleus);
      CHECK(p10.getEnergy() == 9 * 15_GeV);
      CHECK(p10.getTime() == 100_s);
      CHECK(p10.getNuclearA() == 9);
      CHECK(p10.getNuclearZ() == 9 / 2);
    }

    // copy
    {
      s.copy(s.begin() + 93, s.begin() + 9); // non-nuclei to nuclei
      const auto& p93 = s.cbegin() + 93;
      const auto& p9 = s.cbegin() + 9;

      CHECK(p9.getPID() == Code::Electron);
      CHECK(p9.getEnergy() == 93 * 1.5_GeV);
      CHECK(p9.getTime() == 100_s);

      CHECK(p93.getPID() == Code::Electron);
      CHECK(p93.getEnergy() == 93 * 1.5_GeV);
      CHECK(p93.getTime() == 100_s);
    }

    // copy
    {
      s.copy(s.begin() + 89, s.begin() + 79); // nuclei to nuclei
      const auto& p89 = s.cbegin() + 89;
      const auto& p79 = s.cbegin() + 79;

      CHECK(p89.getPID() == Code::Nucleus);
      CHECK(p89.getEnergy() == 89 * 15_GeV);
      CHECK(p89.getTime() == 100_s);

      CHECK(p79.getPID() == Code::Nucleus);
      CHECK(p79.getEnergy() == 89 * 15_GeV);
      CHECK(p79.getTime() == 100_s);
    }

    // swap
    {
      s.swap(s.begin() + 11, s.begin() + 10);
      const auto& p11 = s.cbegin() + 11; // now: nucleus
      const auto& p10 = s.cbegin() + 10; // now: electron

      CHECK(p11.getPID() == Code::Nucleus);
      CHECK(p11.getEnergy() == 9 * 15_GeV);
      CHECK(p11.getTime() == 100_s);
      CHECK(p11.getNuclearA() == 9);
      CHECK(p11.getNuclearZ() == 9 / 2);

      CHECK(p10.getPID() == Code::Electron);
      CHECK(p10.getEnergy() == 11 * 1.5_GeV);
      CHECK(p10.getTime() == 100_s);
    }

    // swap two nuclei
    {
      s.swap(s.begin() + 29, s.begin() + 59);
      const auto& p29 = s.cbegin() + 29;
      const auto& p59 = s.cbegin() + 59;

      CHECK(p29.getPID() == Code::Nucleus);
      CHECK(p29.getEnergy() == 59 * 15_GeV);
      CHECK(p29.getTime() == 100_s);
      CHECK(p29.getNuclearA() == 59);
      CHECK(p29.getNuclearZ() == 59 / 2);

      CHECK(p59.getPID() == Code::Nucleus);
      CHECK(p59.getEnergy() == 29 * 15_GeV);
      CHECK(p59.getTime() == 100_s);
      CHECK(p59.getNuclearA() == 29);
      CHECK(p59.getNuclearZ() == 29 / 2);
    }

    for (int i = 0; i < 99; ++i) s.last().erase();
    CHECK(s.getEntries() == 0);
  }

  SECTION("not allowed") {
    nuclear_stack::NuclearStackExtension<simple_stack::SuperStupidStack,
                                         nuclear_stack::ExtendedParticleInterfaceType>
        s;

    // not valid:
    CHECK_THROWS(s.addParticle(std::make_tuple(
        Code::Oxygen, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 16, 8)));

    // valid
    auto particle = s.addParticle(std::make_tuple(
        Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));

    // not valid
    CHECK_THROWS(particle.addSecondary(std::make_tuple(
        Code::Oxygen, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 16, 8)));

    // add a another nucleus, so there are two now
    s.addParticle(std::make_tuple(
        Code::Nucleus, 1.5_GeV, MomentumVector(dummyCS, {1_GeV, 1_GeV, 1_GeV}),
        Point(dummyCS, {1 * meter, 1 * meter, 1 * meter}), 100_s, 10, 9));

    // not valid, since end() is not a valid entry
    CHECK_THROWS(s.swap(s.begin(), s.end()));
  }
}
