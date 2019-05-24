/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_utl_Bit_h_
#define _corsika_utl_Bit_h_

/**
  \author Hans Dembinski
  \author Lukas Nellen
  \author Darko Veberic
  \date 27 Jan 2014

  \version $Id: Bit.h 25126 2014-02-03 22:13:10Z darko $
*/

#include <exception>

// #include <utl/AugerException.h>

namespace corsika::utl {

  namespace Bit {

    template <typename T>
    class Array {
    public:
      Array(T& target)
          : fTarget(target) {}

      class Bit {
      public:
        Bit(T& target, T mask)
            : fTarget(target)
            , fMask(mask) {}

        operator bool() const { return fTarget & fMask; }

        bool operator~() const { return !bool(*this); }

        Bit& operator=(const bool value) {
          if (value)
            fTarget |= fMask;
          else
            fTarget &= ~fMask;
          return *this;
        }

        Bit& Flip() { return *this = ~(*this); }

      private:
        T& fTarget;
        T fMask;
      };

      Bit operator[](unsigned int position) { return Bit(fTarget, T(1) << position); }

      Bit At(unsigned int position) {
        if (position >= 8 * sizeof(T))
          // throw std::exceptionOutOfBoundException("Running out of bits.");
          throw std::exception("Running out of bits.");
        return (*this)[position];
      }

      template <typename M>
      Array& Mask(const M mask, const bool value) {
        Bit(fTarget, mask) = value;
        return *this;
      }

      template <typename M>
      T Get(const M mask) {
        return fTarget & T(mask);
      }

    private:
      T& fTarget;
    };

  } // namespace Bit

  // helper
  template <typename T>
  inline Bit::Array<T> AsBitArray(T& target) {
    return Bit::Array<T>(target);
  }

} // namespace corsika::utl

#endif
