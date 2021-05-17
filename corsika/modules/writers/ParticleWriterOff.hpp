/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>

namespace corsika {

  class ParticleWriterOff : public BaseOutput {

  public:
    ParticleWriterOff() {}
    virtual ~ParticleWriterOff() {}

    void startOfLibrary(boost::filesystem::path const&) final override {}

    void endOfShower(unsigned int const) final override {}

    void endOfLibrary() final override {}

    // for pdg particles
    void write(Code const&, HEPEnergyType const&, LengthType const&, LengthType const&,
               double const) {}

    // for nuclei
    void write(unsigned int const, unsigned int const, HEPEnergyType const&,
               LengthType const&, LengthType const&, double const) {}

  }; // class ParticleWriterOff

} // namespace corsika
