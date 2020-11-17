/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/IMediumPropertyModel.hpp>

namespace corsika {

  template <typename T>
  template <typename... Args>
  MediumPropertyModel<T>::MediumPropertyModel(Medium const medium, Args&&... args)
      : T(std::forward<Args>(args)...)
      , medium_(medium) {}

  template <typename T>
  Medium MediumPropertyModel<T>::getMedium(Point const&) const final override {
    return medium_;
  }

  template <typename T>
  void MediumPropertyModel<T>::setMedium(Medium const medium) {
    medium_ = medium;
  }

} // namespace corsika
