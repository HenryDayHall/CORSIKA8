/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/BaseOutput.hpp>

namespace corsika {

  /**
   * Generic class to switch off any output.
   *
   * The 'write' method is catch-all and does nothing.
   */

  class WriterOff : public BaseOutput {

  public:
    WriterOff() {}
    virtual ~WriterOff() {}

    void startOfLibrary(boost::filesystem::path const&) final override {}

    void endOfShower(unsigned int const) final override {}

    void endOfLibrary() final override {}

    template <typename... TArgs>
    void write(TArgs&&...) {}

    virtual YAML::Node getConfig() const override { return YAML::Node(); }

  }; // class WriterOff

  class WriterOff2 : public BaseOutput {

  public:
    WriterOff2() {}
    virtual ~WriterOff2() {}

    void startOfLibrary(boost::filesystem::path const&) final override {}

    void endOfShower(unsigned int const) final override {}

    void endOfLibrary() final override {}

    template <typename... TArgs>
    void write(TArgs&&...) {}

    virtual YAML::Node getConfig() const override { return YAML::Node(); }

  }; // class WriterOff

} // namespace corsika
