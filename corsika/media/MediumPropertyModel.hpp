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

  /**
   * A model for the energy loss property of a medium.
   *
   */
  template <typename T>
  class MediumPropertyModel : public T {

    Medium medium_; ///< The medium that is used for this medium.

  public:
    /**
     * Construct a MediumPropertyModel.
     *
     * @param field    The refractive index to return everywhere.
     */
    template <typename... Args>
    MediumPropertyModel(Medium const medium, Args&&... args);

    /**
     * Evaluate the medium type at a given location.
     *
     * @param  point    The location to evaluate at.
     * @returns    The medium type as enum environment::Medium
     */
    Medium getMedium(Point const&) const override;

    /**
     * Set the medium type.
     *
     * @param  medium    The medium to store.
     * @returns    The medium type as enum environment::Medium
     */
    void setMedium(Medium const medium) override;

  }; // END: class MediumPropertyModel

} // namespace corsika

#include <corsika/detail/media/MediumPropertyModel.inl>
