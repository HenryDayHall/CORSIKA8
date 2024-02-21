/*
 * (c) Copyright 2024 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

/*
  these tests were removed since hadronic interactions with pythia 8.310 currently do not
  work. This issue is addressed in MR 549:
  https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/merge_requests/549
  when this is merged the interaction tests can be activated again by including this file
  in testPytha8.cpp eg #include "tests/modules/testPythia8Interaction.inl"
*/
SECTION("pythia interaction") {

  // this will be a p-p collision at sqrts=3.5TeV -> no problem for pythia
  auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
      Code::Proton, 7_TeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
  auto& view = *secViewPtr;

  corsika::pythia8::Interaction collision;

  REQUIRE(collision.canInteract(Code::Proton));
  REQUIRE(collision.canInteract(Code::AntiProton));
  REQUIRE(collision.canInteract(Code::Neutron));
  REQUIRE(collision.canInteract(Code::AntiNeutron));
  REQUIRE(collision.canInteract(Code::PiMinus));
  REQUIRE(collision.canInteract(Code::PiPlus));
  REQUIRE_FALSE(collision.canInteract(Code::Electron));

  // pi+p
  REQUIRE(collision.getCrossSection(
              Code::PiPlus, Code::Proton,
              {sqrt(static_pow<2>(PiPlus::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Proton::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

  // pi+H
  REQUIRE(collision.getCrossSection(
              Code::PiPlus, Code::Hydrogen,
              {sqrt(static_pow<2>(PiPlus::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Hydrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

  // K+{p,n,N,O,Ar}
  Code const target =
      GENERATE(Code::Proton, Code::Neutron, Code::Nitrogen, Code::Oxygen, Code::Argon);
  REQUIRE(collision.getCrossSection(
              Code::KPlus, target,
              {sqrt(static_pow<2>(KPlus::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {get_mass(target), {rootCS, {0_eV, 0_eV, 0_eV}}}) > 0_mb);

  collision.doInteraction(view, Code::Proton, target,
                          {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
                           {rootCS, {0_eV, 0_eV, 100_GeV}}},
                          {get_mass(target), {rootCS, {0_eV, 0_eV, 0_eV}}});
  REQUIRE(view.getSize() >= 2);
}

SECTION("pythia too low energy") {

  // this is a projectile neutron with very little energy
  auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
      Code::Neutron, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
  auto& view = *secViewPtr;

  corsika::pythia8::Interaction collision;

  // 5 MeV lab is too low, 0 mb expected
  REQUIRE(collision.getCrossSectionInelEla(
              Code::Proton, Code::Proton,
              {calculate_total_energy(Proton::mass, 5_MeV), {rootCS, 0_eV, 0_eV, 5_MeV}},
              {Proton::mass, {rootCS, 0_eV, 0_eV, 0_eV}}) == std::tuple{0_mb, 0_mb});

  REQUIRE_THROWS(collision.doInteraction(
      view, Code::Neutron, Code::Proton,
      {calculate_total_energy(Neutron::mass, 5_MeV), {rootCS, 0_eV, 0_eV, 5_MeV}},
      {Proton::mass, {rootCS, 0_eV, 0_eV, 0_eV}}));
}

SECTION("pythia wrong target") {

  // incompatible target
  auto [env_Fe, csPtr_Fe, nodePtr_Fe] = setup::testing::setup_environment(Code::Iron);
  {
    [[maybe_unused]] auto const& cs_Fe = *csPtr_Fe;
    [[maybe_unused]] auto const& env_dummy_Fe = env_Fe;
    [[maybe_unused]] auto const& node_dummy_Fe = nodePtr_Fe;
  }

  // resonable projectile, but too low energy
  auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
      Code::Proton, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr_Fe, *csPtr_Fe);
  auto& view = *secViewPtr;
  { [[maybe_unused]] auto const& dummy_StackPtr = stackPtr; }

  corsika::pythia8::Interaction collision;

  REQUIRE(collision.getCrossSectionInelEla(Code::Proton, Code::Iron,
                                           {calculate_total_energy(Proton::mass, 100_GeV),
                                            {rootCS, {0_eV, 0_eV, 100_GeV}}},
                                           {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) ==
          std::tuple{0_mb, 0_mb});

  REQUIRE(collision.getCrossSection(
              Code::Proton, Code::Iron,
              {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == 0_mb);

  REQUIRE_THROWS(
      collision.doInteraction(view, Code::Proton, Code::Iron,
                              {sqrt(static_pow<2>(Proton::mass) + static_pow<2>(100_GeV)),
                               {rootCS, {0_eV, 0_eV, 100_GeV}}},
                              {Iron::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}));
}

SECTION("pythia wrong projectile") {
  // resonable projectile, but tool low energy
  auto [stackPtr, secViewPtr] = setup::testing::setup_stack(
      Code::Iron, 1_GeV, (DummyEnvironment::BaseNodeType* const)nodePtr, *csPtr);
  { [[maybe_unused]] auto const& dummy_StackPtr = stackPtr; }

  corsika::pythia8::Interaction collision;
  REQUIRE(collision.getCrossSectionInelEla(
              Code::Electron, Code::Electron,
              {sqrt(static_pow<2>(Electron::mass) + static_pow<2>(100_GeV)),
               {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Proton::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == std::tuple{0_mb, 0_mb});

  REQUIRE_THROWS(
      collision.doInteraction(*secViewPtr, Code::Helium, Code::Nitrogen,
                              {sqrt(static_pow<2>(Helium::mass) + static_pow<2>(100_GeV)),
                               {rootCS, {0_eV, 0_eV, 100_GeV}}},
                              {Nitrogen::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}));

  // gamma+p not possible
  REQUIRE(collision.getCrossSection(
              Code::Photon, Code::Proton, {100_GeV, {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Proton::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) == CrossSectionType::zero());

  REQUIRE(collision.getCrossSectionInelEla(
              Code::Photon, Code::Proton, {100_GeV, {rootCS, {0_eV, 0_eV, 100_GeV}}},
              {Proton::mass, {rootCS, {0_eV, 0_eV, 0_eV}}}) ==
          std::make_tuple(CrossSectionType::zero(), CrossSectionType::zero()));
}