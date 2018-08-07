#ifndef _include_ProcessSequence_h_
#define _include_ProcessSequence_h_

#include <iostream>
#include <typeinfo>
using namespace std;

namespace processes {

  template <typename derived>
  struct Base 
  {
    const derived& GetRef() const
    {
      return static_cast<const derived&>(*this);
    }
  };



  template <typename T1, typename T2>
  class ProcessSequence : public Base <ProcessSequence<T1,T2> >
  {
  public:
    const T1& A;
    const T2& B;
    
    ProcessSequence(const T1& in_A, const T2& in_B)
      : A(in_A)
      , B(in_B)
    { }
    
    template<typename D>
      inline void Call(D& d) const { A.Call(d); B.Call(d); }
    
  };
  

  
  template <typename T1, typename T2>
  inline
  const ProcessSequence<T1,T2>
  operator+ (const Base<T1>& A, const Base<T2>& B)
  {
    return ProcessSequence<T1,T2>( A.GetRef(), B.GetRef() );
  }
  
  
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

} // end namespace
  
#endif

