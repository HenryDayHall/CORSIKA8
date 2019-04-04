/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_NameModel_h
#define _include_NameModel_h

#include <string>
#include <utility>

namespace corsika::environment {

  template <typename T>
  struct NameModel : public T {
    virtual std::string const& GetName() const = 0;
    virtual ~NameModel() = default;
  };

} // namespace corsika::environment

#endif
