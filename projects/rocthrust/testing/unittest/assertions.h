/*
 *  Copyright 2008-2013 NVIDIA Corporation
 *  Modifications Copyright© 2019-2025 Advanced Micro Devices, Inc. All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <thrust/complex.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/universal_vector.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/detail/type_traits.h>

#include <unittest/exceptions.h>
#include <unittest/util.h>

#define ASSERT_EQUAL_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)           unittest::assert_equal((X),(Y), FILE_,  LINE_)
#define ASSERT_EQUAL_QUIET_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)     unittest::assert_equal_quiet((X),(Y), FILE_, LINE_)
#define ASSERT_NOT_EQUAL_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)       unittest::assert_not_equal((X),(Y), FILE_,  LINE_)
#define ASSERT_NOT_EQUAL_QUIET_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_) unittest::assert_not_equal_quiet((X),(Y), FILE_, LINE_)
#define ASSERT_LEQUAL_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)          unittest::assert_lequal((X),(Y), FILE_,  LINE_)
#define ASSERT_GEQUAL_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)          unittest::assert_gequal((X),(Y), FILE_,  LINE_)
#define ASSERT_LESS_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)            unittest::assert_less((X),(Y), FILE_,  LINE_)
#define ASSERT_GREATER_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)         unittest::assert_greater((X),(Y), FILE_,  LINE_)
#define ASSERT_ALMOST_EQUAL_WITH_FILE_AND_LINE(X,Y,FILE_,LINE_)    unittest::assert_almost_equal((X),(Y), FILE_, LINE_)
#define ASSERT_EQUAL_RANGES_WITH_FILE_AND_LINE(X,Y,Z,FILE_,LINE_)  unittest::assert_equal((X),(Y),(Z), FILE_,  LINE_)

#define ASSERT_THROWS_WITH_FILE_AND_LINE(                                     \
  EXPR, EXCEPTION_TYPE, FILE_, LINE_                                          \
)                                                                             \
  {                                                                           \
    unittest::threw_status THRUST_PP_CAT2(__s, LINE_)                         \
      = unittest::did_not_throw;                                              \
    try { EXPR; }                                                             \
    catch (EXCEPTION_TYPE const&)                                             \
    { THRUST_PP_CAT2(__s, LINE_) = unittest::threw_right_type; }              \
    catch (...)                                                               \
    { THRUST_PP_CAT2(__s, LINE_) = unittest::threw_wrong_type; }              \
    unittest::check_assert_throws(                                            \
      THRUST_PP_CAT2(__s, LINE_), THRUST_PP_STRINGIZE(EXCEPTION_TYPE)         \
    , FILE_, LINE_                                                            \
    );                                                                        \
  }                                                                           \
  /**/

#define ASSERT_THROWS_EQUAL_WITH_FILE_AND_LINE(                               \
  EXPR, EXCEPTION_TYPE, VALUE, FILE_, LINE_                                   \
)                                                                             \
  {                                                                           \
    unittest::threw_status THRUST_PP_CAT2(__s, LINE_)                         \
      = unittest::did_not_throw;                                              \
    try { EXPR; }                                                             \
    catch (EXCEPTION_TYPE const& THRUST_PP_CAT2(__e, LINE_))                  \
    {                                                                         \
      if (VALUE == THRUST_PP_CAT2(__e, LINE_))                                \
        THRUST_PP_CAT2(__s, LINE_)                                            \
          = unittest::threw_right_type;                                       \
      else                                                                    \
        THRUST_PP_CAT2(__s, LINE_)                                            \
          = unittest::threw_right_type_but_wrong_value;                       \
    }                                                                         \
    catch (...) { THRUST_PP_CAT2(__s, LINE_) = unittest::threw_wrong_type; }  \
    unittest::check_assert_throws(                                            \
      THRUST_PP_CAT2(__s, LINE_), THRUST_PP_STRINGIZE(EXCEPTION_TYPE)         \
    , FILE_, LINE_                                                            \
    );                                                                        \
  }                                                                           \
  /**/

#define KNOWN_FAILURE_WITH_FILE_AND_LINE(FILE_, LINE_)                                  \
  { unittest::UnitTestKnownFailure f; f << "[" << FILE_ ":" << LINE_ << "]"; throw f; } \
  /**/

#define ASSERT_EQUAL(X,Y)           ASSERT_EQUAL_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_EQUAL_QUIET(X,Y)     ASSERT_EQUAL_QUIET_WITH_FILE_AND_LINE((X),(Y), __FILE__, __LINE__)
#define ASSERT_NOT_EQUAL(X,Y)       ASSERT_NOT_EQUAL_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_NOT_EQUAL_QUIET(X,Y) ASSERT_NOT_EQUAL_QUIET_WITH_FILE_AND_LINE((X),(Y), __FILE__, __LINE__)
#define ASSERT_LEQUAL(X,Y)          ASSERT_LEQUAL_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_GEQUAL(X,Y)          ASSERT_GEQUAL_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_LESS(X,Y)            ASSERT_LESS_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_GREATER(X,Y)         ASSERT_GREATER_WITH_FILE_AND_LINE((X),(Y), __FILE__,  __LINE__)
#define ASSERT_ALMOST_EQUAL(X,Y)    ASSERT_ALMOST_EQUAL_WITH_FILE_AND_LINE((X),(Y), __FILE__, __LINE__)
#define ASSERT_EQUAL_RANGES(X,Y,Z)  ASSERT_EQUAL_WITH_FILE_AND_LINE((X),(Y),(Z), __FILE__,  __LINE__)

#define ASSERT_THROWS(EXPR, EXCEPTION_TYPE)                                   \
  ASSERT_THROWS_WITH_FILE_AND_LINE(EXPR, EXCEPTION_TYPE, __FILE__, __LINE__)  \
  /**/

#define ASSERT_THROWS_EQUAL(EXPR, EXCEPTION_TYPE, VALUE)                                  \
  ASSERT_THROWS_EQUAL_WITH_FILE_AND_LINE(EXPR, EXCEPTION_TYPE, VALUE, __FILE__, __LINE__) \
  /**/

#define KNOWN_FAILURE KNOWN_FAILURE_WITH_FILE_AND_LINE(__FILE__, __LINE__)

namespace unittest
{

size_t const MAX_OUTPUT_LINES = 10;

double const DEFAULT_RELATIVE_TOL = 1e-4;
double const DEFAULT_ABSOLUTE_TOL = 1e-4;

template<typename T>
  struct value_type
{
  using type = typename THRUST_NS_QUALIFIER::detail::remove_const<
                        typename THRUST_NS_QUALIFIER::detail::remove_reference<
                        T
                        >::type
                      >::type;
};

template<typename T>
  struct value_type< THRUST_NS_QUALIFIER::device_reference<T> >
{
  using type = typename value_type<T>::type;
};

////
// check scalar values
template <typename T1, typename T2>
void assert_equal(T1 a, T2 b,
                  const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a == b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are not equal: " << a << " " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_equal(char a, char b,
                  const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a == b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are not equal: " << int(a) << " " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

// sometimes its not possible to << a type
template <typename T1, typename T2>
void assert_equal_quiet(const T1& a, const T2& b,
                        const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a == b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are not equal";
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

////
// check scalar values
template <typename T1, typename T2>
void assert_not_equal(T1 a, T2 b,
                      const std::string& filename = "unknown", int lineno = -1)
{
    if(a == b){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are equal: " << a << " " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_not_equal(char a, char b,
                      const std::string& filename = "unknown", int lineno = -1)
{
    if(a == b){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are equal: " << int(a) << " " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

// sometimes its not possible to << a type
template <typename T1, typename T2>
void assert_not_equal_quiet(const T1& a, const T2& b,
                            const std::string& filename = "unknown", int lineno = -1)
{
    if(a == b){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are equal";
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

template <typename T1, typename T2>
void assert_less(T1 a, T2 b,
                 const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a < b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << a << " is greater or equal to " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_less(char a, char b,
                 const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a < b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << int(a) << " is greater than or equal to " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

template <typename T1, typename T2>
void assert_greater(T1 a, T2 b,
                    const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a > b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << a << " is less than or equal to " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_greater(char a, char b,
                    const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a > b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << int(a) << " is less than or equal to " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

template <typename T1, typename T2>
void assert_lequal(T1 a, T2 b,
                   const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a <= b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << a << " is greater than " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_lequal(char a, char b,
                   const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a <= b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << int(a) << " is greater than " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

template <typename T1, typename T2>
void assert_gequal(T1 a, T2 b,
                   const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a >= b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << a << " is less than " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}

void assert_gequal(char a, char b,
                   const std::string& filename = "unknown", int lineno = -1)
{
    if(!(a >= b)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << int(a) << " is less than " << int(b);
        f << " [type='" << type_name<char>() << "']";
        throw f;
    }
}

// will catch everything implicitly convertable to a double
bool almost_equal(double a, double b, double a_tol, double r_tol)
{

    if (std::abs(a - b) > r_tol * (std::abs(a) + std::abs(b)) + a_tol)
        return false;
    else
        return true;
}

namespace
{ // anonymous namespace

template <typename>
struct is_complex : public THRUST_NS_QUALIFIER::false_type
{};

template <typename T>
struct is_complex<THRUST_NS_QUALIFIER::complex<T>> : public THRUST_NS_QUALIFIER::true_type
{};

template <typename T>
struct is_complex<std::complex<T>> : public THRUST_NS_QUALIFIER::true_type
{};

} // namespace

template <typename T1, typename T2>
inline
  typename THRUST_NS_QUALIFIER::detail::enable_if<is_complex<T1>::value && is_complex<T2>::value,
                                                  bool>::type
  almost_equal(const T1 &a, const T2 &b, double a_tol, double r_tol)
{
    return almost_equal(a.real(), b.real(), a_tol, r_tol) &&
           almost_equal(a.imag(), b.imag(), a_tol, r_tol);
}

template <typename T1, typename T2>
void assert_almost_equal(T1 a, T2 b,
                         const std::string& filename = "unknown", int lineno = -1,
                         double a_tol = DEFAULT_ABSOLUTE_TOL, double r_tol = DEFAULT_RELATIVE_TOL)

{
    if(!almost_equal(a, b, a_tol, r_tol)){
        unittest::UnitTestFailure f;
        f << "[" << filename << ":" << lineno << "] ";
        f << "values are not approximately equal: " << a << " " << b;
        f << " [type='" << type_name<T1>() << "']";
        throw f;
    }
}


template <typename T>
class almost_equal_to
{
    public:
        double a_tol, r_tol;
        almost_equal_to(double _a_tol = DEFAULT_ABSOLUTE_TOL, double _r_tol = DEFAULT_RELATIVE_TOL) : a_tol(_a_tol), r_tol(_r_tol) {}
        bool operator()(const T& a, const T& b) const {
            return almost_equal((double) a, (double) b, a_tol, r_tol);
        }
};


template <typename T>
class almost_equal_to<THRUST_NS_QUALIFIER::complex<T> >
{
    public:
        double a_tol, r_tol;
        almost_equal_to(double _a_tol = DEFAULT_ABSOLUTE_TOL, double _r_tol = DEFAULT_RELATIVE_TOL) : a_tol(_a_tol), r_tol(_r_tol) {}
        bool operator()(const THRUST_NS_QUALIFIER::complex<T>& a, const THRUST_NS_QUALIFIER::complex<T>& b) const {
            return almost_equal((double) a.real(), (double) b.real(), a_tol, r_tol)
                && almost_equal((double) a.imag(), (double) b.imag(), a_tol, r_tol);
        }
};

////
// check sequences

template <typename ForwardIterator1, typename ForwardIterator2, typename BinaryPredicate>
void assert_equal(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate op,
                  const std::string& filename = "unknown", int lineno = -1)
{
    using difference_type = typename THRUST_NS_QUALIFIER::iterator_difference<ForwardIterator1>::type;
    using InputType       = typename THRUST_NS_QUALIFIER::iterator_value<ForwardIterator1>::type;

    bool failure = false;

    difference_type length1 = THRUST_NS_QUALIFIER::distance(first1, last1);
    difference_type length2 = THRUST_NS_QUALIFIER::distance(first2, last2);

    difference_type min_length = THRUST_NS_QUALIFIER::min(length1, length2);

    unittest::UnitTestFailure f;
    f << "[" << filename << ":" << lineno << "] ";

    // check lengths
    if (length1 != length2)
    {
      failure = true;
      f << "Sequences have different sizes (" << length1 << " != " << length2 << ")\n";
    }

    // check values

    size_t mismatches = 0;

    for (difference_type i = 0; i < min_length; i++)
    {
      if(!op(*first1, *first2))
      {
        if (mismatches == 0)
        {
          failure = true;
          f << "Sequences are not equal [type='" << type_name<InputType>() << "']\n";
          f << "--------------------------------\n";
        }

        mismatches++;

        if(mismatches <= MAX_OUTPUT_LINES)
        {
          THRUST_IF_CONSTEXPR(sizeof(InputType) == 1)
          {
            f << "  [" << i << "] " << *first1 + InputType() << "  " << *first2 + InputType() << "\n"; // unprintable chars are a problem
          }
          else
          {
            f << "  [" << i << "] " << *first1 << "  " << *first2 << "\n";
          }
        }
      }

      first1++;
      first2++;
    }

    if (mismatches > 0)
    {
      if(mismatches > MAX_OUTPUT_LINES)
          f << "  (output limit reached)\n";
      f << "--------------------------------\n";
      f << "Sequences differ at " << mismatches << " of " << min_length << " positions" << "\n";
    }
    else if (length1 != length2)
    {
      f << "Sequences agree through " << min_length << " positions [type='" << type_name<InputType>() << "']\n";
    }

    if (failure)
      throw f;
}

template <typename ForwardIterator1, typename ForwardIterator2>
void assert_equal(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2,
                  const std::string& filename = "unknown", int lineno = -1)
{
    using InputType = typename THRUST_NS_QUALIFIER::iterator_traits<ForwardIterator1>::value_type;
    assert_equal(first1, last1, first2, last2, THRUST_NS_QUALIFIER::equal_to<InputType>(), filename, lineno);
}


template <typename ForwardIterator1, typename ForwardIterator2>
void assert_almost_equal(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    using InputType = typename THRUST_NS_QUALIFIER::iterator_traits<ForwardIterator1>::value_type;
    assert_equal(first1, last1, first2, last2, almost_equal_to<InputType>(a_tol, r_tol), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    assert_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> B_host = B;
    assert_equal(A, B_host, filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc2> A_host = A;
    assert_equal(A_host, B, filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    THRUST_NS_QUALIFIER::host_vector<T> A_host = A;
    THRUST_NS_QUALIFIER::host_vector<T> B_host = B;
    assert_equal(A_host, B_host, filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    assert_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    assert_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    assert_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> A_host = A;
    assert_equal(A_host, B, filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                  const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> B_host = B;
    assert_equal(A, B_host, filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_equal(const std::vector<T,Alloc1>& A, const std::vector<T,Alloc2>& B,
                  const std::string& filename = "unknown", int lineno = -1)
{
    assert_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    assert_almost_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> B_host = B;
    assert_almost_equal(A, B_host, filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc2> A_host = A;
    assert_almost_equal(A_host, B, filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    THRUST_NS_QUALIFIER::host_vector<T> A_host = A;
    THRUST_NS_QUALIFIER::host_vector<T> B_host = B;
    assert_almost_equal(A_host, B_host, filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    assert_almost_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::host_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    assert_almost_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::host_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    assert_almost_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::device_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::universal_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> A_host = A;
    assert_almost_equal(A_host, B, filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const THRUST_NS_QUALIFIER::universal_vector<T,Alloc1>& A,
                         const THRUST_NS_QUALIFIER::device_vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    THRUST_NS_QUALIFIER::host_vector<T,Alloc1> B_host = B;
    assert_almost_equal(A, B_host, filename, lineno, a_tol, r_tol);
}

template <typename T, typename Alloc1, typename Alloc2>
void assert_almost_equal(const std::vector<T,Alloc1>& A, const std::vector<T,Alloc2>& B,
                         const std::string& filename = "unknown", int lineno = -1,
                         const double a_tol = DEFAULT_ABSOLUTE_TOL, const double r_tol = DEFAULT_RELATIVE_TOL)
{
    assert_almost_equal(A.begin(), A.end(), B.begin(), B.end(), filename, lineno, a_tol, r_tol);
}

enum threw_status
{
  did_not_throw
, threw_wrong_type
, threw_right_type_but_wrong_value
, threw_right_type
};

void check_assert_throws(
  threw_status s
, std::string const& exception_name
, std::string const& file_name = "unknown"
, int line_number = -1
)
{
  switch (s)
  {
    case did_not_throw:
    {
      unittest::UnitTestFailure f;
      f << "[" << file_name << ":" << line_number << "] did not throw anything";
      throw f;
    }
    case threw_wrong_type:
    {
      unittest::UnitTestFailure f;
      f << "[" << file_name << ":" << line_number << "] did not throw an "
        << "object of type " << exception_name;
      throw f;
    }
    case threw_right_type_but_wrong_value:
    {
      unittest::UnitTestFailure f;
      f << "[" << file_name << ":" << line_number << "] threw an object of the "
        << "correct type (" << exception_name << ") but wrong value";
      throw f;
    }
    case threw_right_type:
      break;
    default:
    {
      unittest::UnitTestFailure f;
      f << "[" << file_name << ":" << line_number << "] encountered an "
        << "unknown error";
      throw f;
    }
  }
}

}; //end namespace unittest
