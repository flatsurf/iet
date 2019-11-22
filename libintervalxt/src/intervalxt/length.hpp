/**********************************************************************
 *  This file is part of intervalxt.
 *
 *        Copyright (C) 2019 Vincent Delecroix
 *        Copyright (C) 2019 Julian Rüth
 *
 *  intervalxt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  intervalxt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with intervalxt. If not, see <https://www.gnu.org/licenses/>.
 *********************************************************************/

#ifndef LIBINTERVALXT_LENGTH_HPP
#define LIBINTERVALXT_LENGTH_HPP

#include <utility>

#include <e-antic/renfxx.h>
#include <gmpxx.h>

#include <boost/operators.hpp>

#include "intervalxt/forward.hpp"

namespace intervalxt {

template <typename T>
using LengthQuotient = std::conditional_t<std::is_integral_v<T>, T, mpz_class>;

// A sample implementation of a length of a vector in ℝ² as a simple T.
template <typename T>
class Length : boost::totally_ordered<Length<T>>,
               boost::totally_ordered<Length<T>, T>,
               boost::totally_ordered<Length<T>, std::pair<T, T>>,
               boost::additive<Length<T>>,
               boost::multipliable<Length<T>, LengthQuotient<T>> {
 public:
  using Quotient = LengthQuotient<T>;
  // Ideally coefficient should only be mpz_class and the interval exchange
  // transformation keeps track of a common denominator, see https://github.com/flatsurf/intervalxt/issues/48
  using Coefficient = mpq_class;

  Length();
  Length(const T&);

  template <bool enable = !std::is_same_v<T, eantic::renf_elem_class>, std::enable_if_t<enable, int> = 0>
  Length(const std::string&);

  bool operator==(const Length&) const noexcept;
  bool operator<(const Length&) const noexcept;
  bool operator==(const T&) const noexcept;
  bool operator<(const T&) const noexcept;
  bool operator>(const T&) const noexcept;
  // Compare this length with the length of the vector (x, y)
  bool operator==(const std::pair<T, T>&) const noexcept;
  bool operator<(const std::pair<T, T>&) const noexcept;
  bool operator>(const std::pair<T, T>&) const noexcept;
  Length& operator+=(const Length&) noexcept;
  Length& operator-=(const Length&) noexcept;
  Length& operator*=(const Quotient&) noexcept;

  explicit operator bool() const noexcept;

  // Return the coefficients of this length written as a linear combination in
  // a suitable basis that makes all coefficients rational, such as a number
  // field basis, or a basis of random reals.  The coefficients are returned as
  // integers, discarding the common denominator.
  std::vector<Coefficient> coefficients() const;

  // Return the floor of the division of this length by the argument.
  Quotient operator/(const Length&);

  template <typename C>
  friend std::ostream& operator<<(std::ostream&, const Length<C>&);

 private:
  T value;

  template <typename Archive, typename S>
  friend void load(Archive&, Length<S>&);
  template <typename Archive, typename S>
  friend void save(Archive&, const Length<S>&);
};

}  // namespace intervalxt

#endif
