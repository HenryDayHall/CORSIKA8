/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/output/Configurable.hpp>

#include <utility>

namespace corsika {

  template <typename TOutput>
  class SubWriter : public Configurable {

    TOutput& output_;

  public:
    SubWriter(TOutput& output)
        : output_(output) {}

    virtual ~SubWriter() {}

    /*
     * Write data to the underlying writer.
     */
    template <typename... TArgs>
    void write(TArgs&&... args) {
      output_.write(std::forward<TArgs>(args)...);
    }

  }; // class SubWriter

} // namespace corsika
