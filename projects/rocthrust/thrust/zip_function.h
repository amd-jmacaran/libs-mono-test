
/*! \file thrust/zip_function.h
 *  \brief Adaptor type that turns an N-ary function object into one that takes
 *         a tuple of size N so it can easily be used with algorithms taking zip
 *         iterators
 */

#pragma once

#include <thrust/detail/config.h>
#include <thrust/detail/modern_gcc_required.h>
#if !defined(THRUST_LEGACY_GCC)

#include <thrust/tuple.h>
#include <thrust/type_traits/integer_sequence.h>
#include <thrust/detail/type_deduction.h>

THRUST_NAMESPACE_BEGIN

/*! \addtogroup function_objects Function Objects
 *  \{
 */

/*! \addtogroup function_object_adaptors Function Object Adaptors
 *  \ingroup function_objects
 *  \{
 */

namespace detail {
namespace zip_detail {

// Add workaround for decltype(auto) on C++11-only compilers:
#if THRUST_CPP_DIALECT >= 2014

THRUST_EXEC_CHECK_DISABLE
template <typename Function, typename Tuple, std::size_t... Is>
THRUST_HOST_DEVICE
decltype(auto) apply_impl(Function&& func, Tuple&& args, index_sequence<Is...>)
{
  return func(thrust::get<Is>(THRUST_FWD(args))...);
}

template <typename Function, typename Tuple>
THRUST_HOST_DEVICE
decltype(auto) apply(Function&& func, Tuple&& args)
{
  constexpr auto tuple_size = thrust::tuple_size<typename std::decay<Tuple>::type>::value;
  return apply_impl(THRUST_FWD(func), THRUST_FWD(args), make_index_sequence<tuple_size>{});
}

#else // THRUST_CPP_DIALECT

THRUST_EXEC_CHECK_DISABLE
template <typename Function, typename Tuple, std::size_t... Is>
THRUST_HOST_DEVICE
auto apply_impl(Function&& func, Tuple&& args, index_sequence<Is...>)
THRUST_DECLTYPE_RETURNS(func(thrust::get<Is>(THRUST_FWD(args))...))

template <typename Function, typename Tuple>
THRUST_HOST_DEVICE
auto apply(Function&& func, Tuple&& args)
THRUST_DECLTYPE_RETURNS(
    apply_impl(
      THRUST_FWD(func),
      THRUST_FWD(args),
      make_index_sequence<
        thrust::tuple_size<typename std::decay<Tuple>::type>::value>{})
)

#endif // THRUST_CPP_DIALECT

} // namespace zip_detail
} // namespace detail

/*! \p zip_function is a function object that allows the easy use of N-ary
 *  function objects with \p zip_iterators without redefining them to take a
 *  \p tuple instead of N arguments.
 *
 *  This means that if a functor that takes 2 arguments which could be used with
 *  the \p transform function and \p device_iterators can be extended to take 3
 *  arguments and \p zip_iterators without rewriting the functor in terms of
 *  \p tuple.
 *
 *  The \p make_zip_function convenience function is provided to avoid having
 *  to explicitely define the type of the functor when creating a \p zip_function,
 *  whic is especially helpful when using lambdas as the functor.
 *
 *  \code
 *  #include <thrust/iterator/zip_iterator.h>
 *  #include <thrust/device_vector.h>
 *  #include <thrust/transform.h>
 *  #include <thrust/zip_function.h>
 *
 *  struct SumTuple {
 *    float operator()(auto tup) const {
 *      return thrust::get<0>(tup) + thrust::get<1>(tup) + thrust::get<2>(tup);
 *    }
 *  };
 *  struct SumArgs {
 *    float operator()(float a, float b, float c) const {
 *      return a + b + c;
 *    }
 *  };
 *
 *  int main() {
 *    thrust::device_vector<float> A{0.f, 1.f, 2.f};
 *    thrust::device_vector<float> B{1.f, 2.f, 3.f};
 *    thrust::device_vector<float> C{2.f, 3.f, 4.f};
 *    thrust::device_vector<float> D(3);
 *
 *    auto begin = thrust::make_zip_iterator(thrust::make_tuple(A.begin(), B.begin(), C.begin()));
 *    auto end = thrust::make_zip_iterator(thrust::make_tuple(A.end(), B.end(), C.end()));
 *
 *    // The following four invocations of transform are equivalent:
 *    // Transform with 3-tuple
 *    thrust::transform(begin, end, D.begin(), SumTuple{});
 *
 *    // Transform with 3 parameters
 *    thrust::zip_function<SumArgs> adapted{};
 *    thrust::transform(begin, end, D.begin(), adapted);
 *
 *    // Transform with 3 parameters with convenience function
 *    thrust::transform(begin, end, D.begin(), thrust::make_zip_function(SumArgs{}));
 *
 *    // Transform with 3 parameters with convenience function and lambda
 *    thrust::transform(begin, end, D.begin(), thrust::make_zip_function([] (float a, float b, float c) {
 *                                                                         return a + b + c;
 *                                                                       }));
 *    return 0;
 *  }
 *  \endcode
 *
 *  \see make_zip_function
 *  \see zip_iterator
 */
template <typename Function>
class zip_function
{
  public:
    //! Default constructs the contained function object.
    zip_function() = default;

    /*! Constructs a \p zip_function with the provided function object \p func. */
    THRUST_HOST_DEVICE zip_function(Function func)
        : func(std::move(func))
    {}

    /*! Applies the N-ary function object to elements of the tuple \p args. */
// Add workaround for decltype(auto) on C++11-only compilers:
#if THRUST_CPP_DIALECT >= 2014

    template <typename Tuple>
    THRUST_HOST_DEVICE
    decltype(auto) operator()(Tuple&& args) const
    {
        return detail::zip_detail::apply(func, THRUST_FWD(args));
    }

#else // THRUST_CPP_DIALECT

    // Can't just use THRUST_DECLTYPE_RETURNS here since we need to use
    // std::declval for the signature components:
    template <typename Tuple>
    THRUST_HOST_DEVICE
    auto operator()(Tuple&& args) const
    noexcept(noexcept(detail::zip_detail::apply(std::declval<Function>(), THRUST_FWD(args))))
    -> decltype(detail::zip_detail::apply(std::declval<Function>(), THRUST_FWD(args)))
    {
        return detail::zip_detail::apply(func, THRUST_FWD(args));
    }

#endif // THRUST_CPP_DIALECT

  //! Returns a reference to the underlying function.
  THRUST_HOST_DEVICE Function& underlying_function() const
  {
    return func;
  }

  private:
    mutable Function func;
};

/*! \p make_zip_function creates a \p zip_function from a function object.
 *
 *  \param fun The N-ary function object.
 *  \return A \p zip_function that takes a N-tuple.
 *
 *  \see zip_function
 */
template <typename Function>
THRUST_HOST_DEVICE
zip_function<typename std::decay<Function>::type>
make_zip_function(Function&& fun)
{
    using func_t = typename std::decay<Function>::type;
    return zip_function<func_t>(THRUST_FWD(fun));
}

/*! \} // end function_object_adaptors
 */

/*! \} // end function_objects
 */

THRUST_NAMESPACE_END

#endif
