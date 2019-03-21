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
      
    template <typename... Args>
    NameModel(std::string const& name, Args&&... args) : T(std::forward<Args>(args)...), fName(name) {}

    std::string const& GetName() const {
        return fName;
    }
    
    private:
      std::string fName;
  };

} // namespace corsika::environment

#endif
