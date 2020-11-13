/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

/**
  \author Hans Dembinski
  \author Lukas Nellen
  \author Darko Veberic
  \date 27 Jan 2014

  \version $Id: Bit.h 25126 2014-02-03 22:13:10Z darko $
*/

#include <exception>


namespace corsika {

  template <typename TType>
  class BitField {
  public:
    BitField(TType& target);

    class Bit {
    public:
      Bit(TType& target, TType mask);

      operator bool() const ;

      bool operator~() const ;

      Bit& operator=(const bool value);

      Bit& flip();

    private:
      TType& target_;
      TType mask_;
    };

    Bit operator[](unsigned int position);

    Bit at(unsigned int position);

    template <typename TMask>
    BitField& mask(const TMask mask, const bool value);

    template <typename TMask>
    TType get(const TMask mask);

  private:
    TType& target_;
  };

  // helper
  template <typename TType>
  inline corsika::BitField<TType> asBitField(TType& target) {
    return corsika::BitField<TType>(target);
  }

} // namespace corsika

#include <corsika/detail/framework/utility/BitField.inl>
