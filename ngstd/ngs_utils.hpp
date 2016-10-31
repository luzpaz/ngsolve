#ifndef FILE_NGS_UTILS
#define FILE_NGS_UTILS

/**************************************************************************/
/* File:   ngs_utils.hpp                                                  */
/* Author: Joachim Schoeberl                                              */
/* Date:   Oct. 14                                                        */
/**************************************************************************/


namespace ngstd
{

  
  template <typename T>
  INLINE T RemoveConst (const T & x)
  {
    return x;
  }
  

  //////////////////////////////////////////////////////////////////////
  // Lambda to function pointer conversion,  M. Hochsteger
  
  template <typename Function>
  struct function_traits
    : public function_traits<decltype(&Function::operator())> 
  { };
  
  template <typename ClassType, typename ReturnType, typename... Args>
  struct function_traits<ReturnType(ClassType::*)(Args...) const> 
  {
    typedef ReturnType (*pointer)(Args...);
    typedef ReturnType return_type;
  };
  
  template <typename Function>
  typename function_traits<Function>::pointer
  FunctionPointer (const Function& lambda) 
  {
    return static_cast<typename function_traits<Function>::pointer>(lambda);
  }

  template <typename Function>
  typename function_traits<Function>::return_type
  GetReturnValue (Function func) 
  {
    typename function_traits<Function>::return_type *dummy;
    return *dummy;
  }


  // ////////////////    integral constants

  template <int N>
  using IC = integral_constant<int,N>;

  /*
  template <int I1, int I2>
  constexpr INLINE IC<I1+I2> operator+ (IC<I1> i1, IC<I2> i2) { return IC<I1+I2>(); }
  */
}

 
namespace std
{
  template <int I1, int I2>
  constexpr INLINE integral_constant<int,I1+I2> 
  operator+ (integral_constant<int,I1> i1,
             integral_constant<int,I2> i2)
  {
    return integral_constant<int,I1+I2>();
  }


  // may be used as an index e.g. for FlatVector
  template <int N>
  struct is_integral<ngstd::IC<N>>
  {
    enum { value = 1 };
  };



 /*
  INLINE void MyAtomicAdd(atomic<double> & sum, double val) {
    auto current = sum.load();
    while (!sum.compare_exchange_weak(current, current + val))
      ;
  }
  */
  INLINE atomic<double> & operator+= (atomic<double> & sum, double val) {
    auto current = sum.load();
    while (!sum.compare_exchange_weak(current, current + val))
      ;
    return sum;
  }

  INLINE atomic<double> & operator-= (atomic<double> & sum, double val) {
    auto current = sum.load();
    while (!sum.compare_exchange_weak(current, current - val))
      ;
    return sum;
  }

  template<typename T>
  INLINE atomic<T> & AsAtomic (T & d)
  {
    return reinterpret_cast<atomic<T>&> (d);
  }
  
  INLINE void MyAtomicAdd (double & sum, double val)
  {
    AsAtomic(sum) += val;
    /*
    auto & asum = reinterpret_cast<atomic<double>&>(sum);
    auto current = asum.load();
    while (!asum.compare_exchange_weak(current, current + val))
      ;
    */
  }



#define aligned_alloca(size,align)  (( (size_t)alloca(size+align-1)+align-1) & -align)
#ifdef VLA
#define STACK_ARRAY(TYPE,VAR,SIZE) TYPE VAR[SIZE]
#else
#define STACK_ARRAY(TYPE,VAR,SIZE) TYPE * VAR = (TYPE*)aligned_alloca((SIZE)*sizeof(TYPE), alignof(TYPE))
#endif




  // technique from 
  // http://stackoverflow.com/questions/14939190/boost-shared-from-this-and-multiple-inheritance

  template<typename T>
  struct enable_shared_from_this_virtual;
  
  class enable_shared_from_this_virtual_base : public std::enable_shared_from_this<enable_shared_from_this_virtual_base>
  {
    typedef std::enable_shared_from_this<enable_shared_from_this_virtual_base> base_type;
    template<typename T>
    friend struct enable_shared_from_this_virtual;

    std::shared_ptr<enable_shared_from_this_virtual_base> shared_from_this()
    {
      return base_type::shared_from_this();
    }
    std::shared_ptr<enable_shared_from_this_virtual_base const> shared_from_this() const
    {
      return base_type::shared_from_this();
    }
  };
  
  template<typename T>
  struct enable_shared_from_this_virtual: virtual enable_shared_from_this_virtual_base
  {
    typedef enable_shared_from_this_virtual_base base_type;
    
  public:
    std::shared_ptr<T> shared_from_this()
    {
      std::shared_ptr<T> result(base_type::shared_from_this(), static_cast<T*>(this));
      return result;
    }
    
    std::shared_ptr<T const> shared_from_this() const
    {
      std::shared_ptr<T const> result(base_type::shared_from_this(), static_cast<T const*>(this));
      return result;
    }
  };
  
  
  
}


#endif
