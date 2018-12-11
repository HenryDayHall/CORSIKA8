
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_ProcessSequence_h_
#define _include_ProcessSequence_h_

#include <corsika/process/BaseProcess.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/DiscreteProcess.h>
#include <corsika/process/ProcessReturn.h>

#include <limits>
#include <cmath>

//#include <corsika/setup/SetupTrajectory.h>
// using corsika::setup::Trajectory;
//#include <variant>

//#include <type_traits> // still needed ?

namespace corsika::process {

  // namespace detail {

  /*   /\* template<typename TT1, typename TT2, typename Type = void> *\/ */
  /*   /\*   struct CallHello { *\/ */
  /*   /\* 	static void Call(const TT1&, const TT2&) { *\/ */
  /*   /\* 	  std::cout << "normal" << std::endl; *\/ */
  /*   /\* 	} *\/ */
  /*   /\*   }; *\/ */

  /*   /\* template<typename TT1, typename TT2> *\/ */
  /*   /\*   struct CallHello<TT1, TT2, typename
   * std::enable_if<std::is_base_of<ContinuousProcess<TT2>, TT2>::value>::type> *\/ */
  /*   /\*   { *\/ */
  /*   /\* 	static void Call(const TT1&, const TT2&) { *\/ */
  /*   /\* 	  std::cout << "special" << std::endl; *\/ */
  /*   /\* 	}	 *\/ */
  /*          }; */

  /*   template<typename T1, typename T2, typename Particle, typename Trajectory, typename
   * Stack> //, typename Type = void> */
  /*     struct DoContinuous { */
  /* 	static EProcessReturn Call(const T1& A, const T2& B, Particle& p, Trajectory& t,
   * Stack& s) { */
  /* 	  EProcessReturn ret = EProcessReturn::eOk; */
  /* 	  if constexpr (!std::is_base_of<DiscreteProcess<T1>, T1>::value)  { */
  /* 	      A.DoContinuous(p, t, s); */
  /* 	    } */
  /* 	  if constexpr (!std::is_base_of<DiscreteProcess<T2>, T2>::value)  { */
  /* 	      B.DoContinuous(p, t, s); */
  /* 	    } */
  /* 	  return ret; */
  /* 	} */
  /*     }; */

  /*   /\* */
  /*   template<typename T1, typename T2, typename Particle, typename Trajectory, typename
   * Stack> */
  /*     struct DoContinuous<T1,T2,Particle,Trajectory,Stack, typename
   * std::enable_if<std::is_base_of<DiscreteProcess<T1>, T1>::value>::type> { */
  /* 	static EProcessReturn Call(const T1& A, const T2& B, Particle& p, Trajectory& t,
   * Stack& s) { */
  /* 	  EProcessReturn ret = EProcessReturn::eOk; */
  /* 	  A.DoContinuous(p, t, s); */
  /* 	  B.DoContinuous(p, t, s); */
  /* 	  return ret; */
  /* 	} */
  /*     }; */

  /*       template<typename T1, typename T2, typename Particle, typename Trajectory,
   * typename Stack> */
  /*     struct DoContinuous<T1,T2,Particle,Trajectory,Stack, typename
   * std::enable_if<std::is_base_of<DiscreteProcess<T2>, T2>::value>::type> { */
  /* 	static EProcessReturn Call(const T1& A, const T2&, Particle& p, Trajectory& t,
   * Stack& s) { */
  /* 	  EProcessReturn ret = EProcessReturn::eOk; */
  /* 	  A.DoContinuous(p, t, s); */
  /* 	  B.DoContinuous(p, t, s); */
  /* 	  return ret; */
  /* 	} */
  /*     }; */
  /*   *\/ */

  /*   template<typename T1, typename T2, typename Particle, typename Stack>//, typename
   * Type = void> */
  /*     struct DoDiscrete { */
  /* 	static EProcessReturn Call(const T1& A, const T2& B, Particle& p, Stack& s)  { */
  /* 	  if constexpr (!std::is_base_of<ContinuousProcess<T1>, T1>::value) { */
  /* 	      A.DoDiscrete(p, s); */
  /* 	    } */
  /* 	  if constexpr (!std::is_base_of<ContinuousProcess<T2>, T2>::value) { */
  /* 	      B.DoDiscrete(p, s); */
  /* 	    } */
  /* 	  return EProcessReturn::eOk; */
  /* 	} */
  /*     }; */
  /*   /\* */
  /*   template<typename T1, typename T2, typename Particle, typename Stack> */
  /*     struct DoDiscrete<T1,T2,Particle,Stack, typename
   * std::enable_if<std::is_base_of<ContinuousProcess<T1>, T1>::value>::type> { */
  /*     static EProcessReturn Call(const T1&, const T2& B, Particle& p, Stack& s) { */
  /* 	// A.DoDiscrete(p, s); */
  /*       B.DoDiscrete(p, s); */
  /*       return EProcessReturn::eOk; */
  /*     } */
  /*   }; */

  /*   template<typename T1, typename T2, typename Particle, typename Stack> */
  /*     struct DoDiscrete<T1,T2,Particle,Stack, typename
   * std::enable_if<std::is_base_of<ContinuousProcess<T2>, T2>::value>::type> { */
  /*     static EProcessReturn Call(const T1& A, const T2&, Particle& p, Stack& s) { */
  /* 	A.DoDiscrete(p, s); */
  /*       //B.DoDiscrete(p, s); */
  /*       return EProcessReturn::eOk; */
  /*     } */
  /*   }; */
  /*   *\/ */
  //} // end namespace detail

  /**
     \class ProcessSequence

     A compile time static list of processes. The compiler will
     generate a new type based on template logic containing all the
     elements.

     \comment Using CRTP pattern,
     https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
   */

  //template <typename T1, typename T2>
  //class ProcessSequence : public BaseProcess<ProcessSequence<T1, T2> >;

  // this is a marker to track which BaseProcess is also a ProcessSequence
    template <typename T>
    struct is_process_sequence {
      static const bool value = false;
    };
    

  template <typename T1, typename T2>
  class ProcessSequence : public BaseProcess<ProcessSequence<T1, T2> > {
    
    /*    template <>
      struct BaseProcess<ProcessSequence<T1, T2> >::is_process_sequence<true> {
      static const bool value = true;
      };*/
    
  public:
    const T1& A;
    const T2& B;

    ProcessSequence(const T1& in_A, const T2& in_B)
        : A(in_A)
        , B(in_B) {}

    // example for a trait-based call:
    // void Hello() const  { detail::CallHello<T1,T2>::Call(A, B); }

    template <typename Particle, typename Track, typename Stack>
    inline EProcessReturn DoContinuous(Particle& p, Track& t, Stack& s) const {
      EProcessReturn ret = EProcessReturn::eOk;
      if constexpr (!std::is_base_of<DiscreteProcess<T1>, T1>::value) {
        // A.DoContinuous(std::forward<Particle>(p), t, std::forward<Stack>(s));
        A.DoContinuous(p, t, s);
      }
      if constexpr (!std::is_base_of<DiscreteProcess<T2>, T2>::value) {
        // B.DoContinuous(std::forward<Particle>(p), t, std::forward<Stack>(s));
        B.DoContinuous(p, t, s);
      }
      return ret;
    }

    template <typename Particle, typename Track>
    inline double MinStepLength(Particle& p, Track& track) const {
      double min_length = std::numeric_limits<double>::infinity();
      if constexpr (!std::is_base_of<DiscreteProcess<T1>, T1>::value) {
	  min_length = std::min(min_length, A.MinStepLength(p,track));
	}
      if constexpr (!std::is_base_of<DiscreteProcess<T2>, T2>::value) {
	  min_length = std::min(min_length, B.MinStepLength(p,track));
	}
      return min_length;
    }

    template <typename Particle, typename Track>
    inline double GetTotalInteractionLength(Particle& p, Track& t) const {
      return 1. / GetInverseInteractionLength(p, t);
    }

    template <typename Particle, typename Track>
    inline double GetTotalInverseInteractionLength(Particle& p, Track& t) const {
      return GetInverseInteractionLength(p, t);
    }

    template <typename Particle, typename Track>
    inline double GetInverseInteractionLength(Particle& p, Track& t) const {
      double tot = 0;
      if constexpr (!std::is_base_of<ContinuousProcess<T1>, T1>::value) {
        tot += A.GetInverseInteractionLength(p, t);
      }
      if constexpr (!std::is_base_of<ContinuousProcess<T2>, T2>::value) {
        tot += B.GetInverseInteractionLength(p, t);
      }
      return tot;
    }

    template <typename Particle, typename Stack>
    inline EProcessReturn DoDiscrete(Particle& p, Stack& s) const {
      if constexpr (!std::is_base_of<ContinuousProcess<T1>, T1>::value) {
        A.DoDiscrete(p, s);
      }
      if constexpr (!std::is_base_of<ContinuousProcess<T2>, T2>::value) {
        B.DoDiscrete(p, s);
      }
      return EProcessReturn::eOk;
    }

    template <typename Particle, typename Stack>
      inline EProcessReturn SelectDiscrete(Particle& p, Stack& s,
					   const double lambda_inv_tot,
					   const double rndm_select,
					   double& lambda_inv_count) const {
      if constexpr (is_process_sequence<T1>::value) {
	  // if A is a process sequence --> check inside
	  const EProcessReturn ret = A.SelectDiscrete(p, s, lambda_inv_count, rndm_select, lambda_inv_count);
	  // if A did suceed, stop routine
	  if (ret != EProcessReturn::eOk) {
	    return ret;
	  }
	} else if constexpr (!std::is_base_of<ContinuousProcess<T1>, T1>::value) {
	  // if this is not a ContinuousProcess --> evaluate probability
	  lambda_inv_count += A.GetInverseInteractionLength(p, s);
	  // check if we should execute THIS process and then EXIT
	  if (rndm_select<lambda_inv_count/lambda_inv_tot) {
	    A.DoDiscrete(p, s);
	    return EProcessReturn::eInteracted;
	  }
	} // end branch A
      
      if constexpr (is_process_sequence<T2>::value) {
	  // if A is a process sequence --> check inside
	  const EProcessReturn ret = B.SelectDiscrete(p, s, lambda_inv_count, rndm_select, lambda_inv_count);
	  // if A did suceed, stop routine
	  if (ret != EProcessReturn::eOk) {
	    return ret;
	  }
	} else if constexpr (!std::is_base_of<ContinuousProcess<T2>, T2>::value) {
	  // if this is not a ContinuousProcess --> evaluate probability
	  lambda_inv_count += B.GetInverseInteractionLength(p, s);
	  // check if we should execute THIS process and then EXIT
	  if (rndm_select<lambda_inv_count/lambda_inv_tot) {
	    B.DoDiscrete(p, s);
	    return EProcessReturn::eInteracted;
	  }
	} // end branch A
      return EProcessReturn::eOk;
    }

    /// TODO the const_cast is not nice, think about the constness here
    inline void Init() const {
      const_cast<T1*>(&A)->Init();
      const_cast<T2*>(&B)->Init();
    }
  };

  /// the +operator assembles many BaseProcess, ContinuousProcess, and
  /// DiscreteProcess objects into a ProcessSequence, all combinatorics
  /// must be allowed, this is why we define a macro to define all
  /// combinations here:

#define OPSEQ(C1, C2)                                                                \
  template <typename T1, typename T2>                                                \
  inline const ProcessSequence<T1, T2> operator+(const C1<T1>& A, const C2<T2>& B) { \
    return ProcessSequence<T1, T2>(A.GetRef(), B.GetRef());                          \
  }

  OPSEQ(BaseProcess, BaseProcess)
  OPSEQ(BaseProcess, DiscreteProcess)
  OPSEQ(BaseProcess, ContinuousProcess)
  OPSEQ(ContinuousProcess, BaseProcess)
  OPSEQ(ContinuousProcess, DiscreteProcess)
  OPSEQ(ContinuousProcess, ContinuousProcess)
  OPSEQ(DiscreteProcess, BaseProcess)
  OPSEQ(DiscreteProcess, DiscreteProcess)
  OPSEQ(DiscreteProcess, ContinuousProcess)

  /*
    template <typename T1>
    struct depth_lhs
    {
    static const int num = 0;
    };



    // terminating condition
    template <typename T1, typename T2>
    struct depth_lhs< Sequence<T1,T2> >
    {
    // try to expand the left node (T1) which might be a Sequence type
    static const int num = 1 + depth_lhs<T1>::num;
    };
  */

  /*
    template <typename T1>
    struct mat_ptrs
    {
    static const int num = 0;

    inline static void
    get_ptrs(const Process** ptrs, const T1& X)
    {
    ptrs[0] = reinterpret_cast<const Process*>(&X);
    }
    };


    template <typename T1, typename T2>
    struct mat_ptrs< Sequence<T1,T2> >
    {
    static const int num = 1 + mat_ptrs<T1>::num;

    inline static void
    get_ptrs(const Process** in_ptrs, const Sequence<T1,T2>& X)
    {
    // traverse the left node
    mat_ptrs<T1>::get_ptrs(in_ptrs, X.A);
    // get address of the matrix on the right node
    in_ptrs[num] = reinterpret_cast<const Process*>(&X.B);
    }
    };
  */

  /*
    template<typename T1, typename T2>
    const Process&
    Process::operator=(const Sequence<T1,T2>& X)
    {
    int N = 1 + depth_lhs< Sequence<T1,T2> >::num;
    const Process* ptrs[N];
    mat_ptrs< Sequence<T1,T2> >::get_ptrs(ptrs, X);
    int r = ptrs[0]->rows;
    int c = ptrs[0]->cols;
    // ... check that all matrices have the same size ...
    set_size(r, c);
    for(int j=0; j<r*c; ++j)
    {
    double sum = ptrs[0]->data[j];
    for(int i=1; i<N; ++i)
    {
    sum += ptrs[i]->data[j];
    }
    data[j] = sum;
    }
    return *this;
    }
  */

        template<template<typename, typename> class T, typename A, typename B>
      struct is_process_sequence< T<A,B> > 
    {
      static const bool value = true;
    };

} // namespace corsika::process

#endif
