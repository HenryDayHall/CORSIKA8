/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_utl_Singleton_h_
#define _corsika_utl_Singleton_h_

//#define OFFLINE_USE_GAMMA_SINGLETON

namespace corsika::utl {
  /**
   * \class Singleton Singleton.h utl/Singleton.h
   *
   * \brief Curiously Recurring Template Pattern (CRTP) for Meyers singleton
   *
   * The singleton class is implemented as follows
   * \code
   * #include <utl/Singleton.h>
   *
   * class SomeClass : public utl::Singleton<SomeClass> {
   *   ...
   * private:
   *   // prevent creation, destruction
   *   SomeClass() { }
   *   ~SomeClass() { }
   *
   *   friend class utl::Singleton<SomeClass>;
   * };
   * \endcode
   * Singleton automatically prevents copying of the derived class.
   *
   * \author Darko Veberic
   * \date 9 Aug 2006
   * \version $Id: Singleton.h 25091 2014-01-30 09:49:57Z darko $
   * \ingroup stl
   */

  template <typename T>
  class Singleton {
  public:
    static T& GetInstance() {
      static T instance;
      return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

  protected:
    // derived class can call ctor and dtor
    Singleton() {}
    ~Singleton() {}
  };

} // namespace corsika::utl

#endif
