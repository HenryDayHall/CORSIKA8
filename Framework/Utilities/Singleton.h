
/**
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

#ifndef OFFLINE_USE_GAMMA_SINGLETON

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
    static T& GetInstance()
#ifdef __MAKECINT__
        ;
#else
    {
      static T instance;
      return instance;
    }
#endif

  protected:
    // derived class can call ctor and dtor
    Singleton() {}
    ~Singleton() {}

  private:
    // no one should do copies
    Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);
  };

#else

  /// classical Gamma singleton
  template <typename T>
  class Singleton {
  public:
    static T& GetInstance() {
      if (!fgInstance) fgInstance = new T;
      return *fgInstance;
    }

  protected:
    // derived class can call ctor and dtor
    Singleton() {}
    ~Singleton() {}

  private:
    // no one should do copies
    Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);

    static T* fgInstance = 0;
  };

#endif

  /**
   * \class LeakingSingleton Singleton.h utl/Singleton.h
   *
   * \brief CRTP for leaking singleton
   *
   * This type of creation (Gamma singleton) leaks the object at
   * the end of the run, i.e. class T destructor does not get called
   * in at_exit().
   *
   * This singleton can be implemented as follows
   * \code
   * #include <utl/Singleton.h>
   *
   * class SomeClass : public utl::LeakingSingleton<SomeClass> {
   *   ...
   * private:
   *   // prevent creation, destruction
   *   SomeClass() { }
   *   ~SomeClass() { }
   *
   *   friend class utl::LeakingSingleton<SomeClass>;
   * };
   * \endcode
   * LeakingSingleton automatically prevents copying of the derived
   * class.
   *
   * \author Darko Veberic
   * \date 9 Aug 2006
   * \version $Id: Singleton.h 25091 2014-01-30 09:49:57Z darko $
   * \ingroup stl
   */

  template <class T>
  class LeakingSingleton {
  public:
    static T& GetInstance() {
      static T* const instance = new T;
      return *instance;
    }

  protected:
    // derived class can call ctor and dtor
    LeakingSingleton() {}
    ~LeakingSingleton() {}

  private:
    // no one should do copies
    LeakingSingleton(const LeakingSingleton&);
    LeakingSingleton& operator=(const LeakingSingleton&);
  };

} // namespace corsika::utl

#endif
