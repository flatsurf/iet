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

#include <cassert>
#include <variant>

#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include "intervalxt/length.hpp"

namespace intervalxt {
template <typename T>
Length<T>::Length() : Length(T(0)) {}

template <typename T>
Length<T>::Length(const T& value) : value(value) {
  assert(value >= 0 && "a length cannot be negative");
}

template <typename T>
template <bool enable, std::enable_if_t<enable, int>>
Length<T>::Length(const std::string& s) {
  if constexpr (std::is_integral_v<T>) {
    value = std::stoi(s);
  } else if constexpr (std::is_same_v<T, mpz_class> || std::is_same_v<T, mpq_class>) {
    value = T(s);
  }
  assert(value >= 0 && "a length cannot be negative");
}

template <typename T>
Length<T>& Length<T>::operator+=(const Length<T>& rhs) noexcept {
  value += rhs.value;
  assert(value >= 0 && "a length cannot be negative");
  return *this;
}

template <typename T>
Length<T>& Length<T>::operator-=(const Length<T>& rhs) noexcept {
  value -= rhs.value;
  assert(value >= 0 && "a length cannot be negative");
  return *this;
}

template <typename T>
Length<T>& Length<T>::operator*=(const mpz_class& rhs) noexcept {
  if constexpr (std::is_integral_v<T>) {
    assert(rhs.fits_sint_p());
    value *= boost::numeric_cast<T>(rhs.get_si());
  } else {
    value *= rhs;
  }
  return *this;
}

template <typename T>
mpz_class Length<T>::operator/(const Length<T>& rhs) {
  assert(static_cast<bool>(rhs) && "cannot divide by zero vector");
  if constexpr (std::is_integral_v<T>) {
    auto ret = value / rhs.value;
    if constexpr (std::is_same_v<T, long long>) {
      return mpz_class(boost::lexical_cast<std::string>(ret));
    } else {
      return ret;
    }
  } else if constexpr (std::is_same_v<T, mpz_class>) {
    mpz_class ret = value / rhs.value;
    return ret;
  } else if constexpr (std::is_same_v<T, mpq_class>) {
    // NOTE: floor division could be smarter than that!
    mpz_class ret = (value.get_num() * rhs.value.get_den()) / (value.get_den() * rhs.value.get_num());
    return ret;
  } else if constexpr (std::is_same_v<T, eantic::renf_elem_class>) {
    // NOTE: floor division could be smarter than that!
    return (value / rhs.value).floor();
  } else {
    throw std::logic_error("not implemented: floor division for this type");
  }
}

template <typename T>
const T& Length<T>::length() const {
  return value;
}

template <typename T>
T Length<T>::squared() const {
  auto ret = value * value;
  assert(ret >= value);
  return ret;
}

template <typename T>
Length<T>::operator bool() const noexcept {
  return static_cast<bool>(value);
}

template <typename T>
bool Length<T>::operator<(const Length<T>& rhs) const noexcept {
  return value < rhs.value;
}

template <typename T>
bool Length<T>::operator==(const Length<T>& rhs) const noexcept {
  return value == rhs.value;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Length<T>& self) {
  return os << self.value;
}
}  // namespace intervalxt

// Create instantiations of the template for the linker.

namespace intervalxt {
template class Length<int>;
template Length<int>::Length(const std::string&);
template std::ostream& operator<<(std::ostream&, const Length<int>&);
template class Length<long long>;
template Length<long long>::Length(const std::string&);
template std::ostream& operator<<(std::ostream&, const Length<long long>&);
template class Length<mpz_class>;
template Length<mpz_class>::Length(const std::string&);
template std::ostream& operator<<(std::ostream&, const Length<mpz_class>&);
template class Length<mpq_class>;
template Length<mpq_class>::Length(const std::string&);
template std::ostream& operator<<(std::ostream&, const Length<mpq_class>&);
template class Length<eantic::renf_elem_class>;
template std::ostream& operator<<(std::ostream&, const Length<eantic::renf_elem_class>&);
}  // namespace intervalxt
