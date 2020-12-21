/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <boost/histogram.hpp>

namespace corsika {

  enum class SaveMode { overwrite, append };

  /**
   * This functions saves a boost::histogram into a numpy file. Only rather basic axis
   * types are supported: regular, variable, integer, category<int>. Only "ordinary" bin
   * counts (i.e. a double or int) are supported, nothing fancy like profiles.
   *
   * Note that this function makes a temporary, dense copy of the histogram, which could
   * be an issue for huge sizes (e.g. for high dimensions)
   */
  template <class Axes, class Storage>
  inline void save_hist(boost::histogram::histogram<Axes, Storage> const& h,
                        std::string const& filename, SaveMode mode = SaveMode::append);
} // namespace corsika

#include <corsika/detail/framework/utility/SaveBoostHistogram.inl>

