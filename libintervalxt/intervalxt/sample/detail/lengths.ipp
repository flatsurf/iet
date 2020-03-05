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

#ifndef LIBINTERVALXT_SAMPLE_DETAIL_LENGTHS_IPP
#define LIBINTERVALXT_SAMPLE_DETAIL_LENGTHS_IPP

#include <cassert>
#include <numeric>
#include <vector>

#include "../../label.hpp"

#include "../arithmetic.hpp"
#include "../lengths.hpp"

namespace intervalxt::sample {

namespace {

template <typename T, std::size_t... Indices>
auto toTuple(T&& container, std::index_sequence<Indices...>) { return std::make_tuple(container.at(Indices)...); }

size_t index(Label label) {
  return std::hash<Label>()(label);
}

template <typename T>
int cmp(const T& lhs, const T& rhs) {
  if (lhs < rhs) return -1;
  if (lhs > rhs) return 1;
  return 0;
}

}  // namespace

template <typename T>
Lengths<T>::Lengths() : stack(),
                        lengths() {}

template <typename T>
Lengths<T>::Lengths(const std::vector<T>& lengths) : stack(),
                                                     lengths(lengths) {
  assert(std::all_of(lengths.begin(), lengths.end(), [](const auto& length) { return length >= 0; }) && "all Lengths must be non-negative");
}

template <typename T>
template <typename... L>
auto Lengths<T>::make(L&&... values) {
  auto lengths = Lengths<T>(std::vector{values...});
  return std::tuple_cat(
      std::make_tuple(lengths),
      toTuple(lengths.labels(), std::make_index_sequence<sizeof...(L)>()));
}

template <typename T>
std::vector<Label> Lengths<T>::labels() const {
  std::vector<Label> labels;
  for (int i = 0; i < lengths.size(); i++) labels.push_back(Label(i));
  return labels;
}

template <typename T>
Lengths<T>::operator T() const {
  return std::accumulate(begin(stack), end(stack), T(), [&](T value, Label label) { return value + at(label); });
}

template <typename T>
T Lengths<T>::get(Label label) const {
  return at(label);
}

template <typename T>
const T& Lengths<T>::at(Label label) const {
  return lengths.at(index(label));
}

template <typename T>
T& Lengths<T>::at(Label label) {
  return lengths.at(index(label));
}

template <typename T>
void Lengths<T>::push(Label label) {
  stack.push_back(label);
}

template <typename T>
void Lengths<T>::pop() {
  stack.pop_back();
}

template <typename T>
void Lengths<T>::clear() {
  stack.clear();
}

template <typename T>
int Lengths<T>::cmp(Label rhs) const {
  return ::intervalxt::sample::cmp<T>(*this, at(rhs));
}

template <typename T>
int Lengths<T>::cmp(Label lhs, Label rhs) const {
  return ::intervalxt::sample::cmp<T>(at(lhs), at(rhs));
}

template <typename T>
void Lengths<T>::subtract(Label from) {
  at(from) -= static_cast<T>(*this);
  assert(at(from) > 0 && "all lengths must be positive.");
  clear();
}

template <typename T>
Label Lengths<T>::subtractRepeated(Label from) {
  if (stack.size() == 0)
    throw std::invalid_argument("Cannot subtractRepeated() without push()");

  auto quo = Arithmetic<T>::floorDivision(at(from), static_cast<T>(*this));

  at(from) -= quo * static_cast<T>(*this);

  assert(at(from) >= 0 && "Length cannot be negative.");
  if (at(from) == 0) {
    // Undo the last subtraction
    at(from) += static_cast<T>(*this);
  }

  Label stop = *rbegin(stack);
  for (Label label : stack) {
    if (at(label) >= at(from)) {
      clear();
      return stop;
    }

    stop = label;
    at(from) -= at(label);
    assert(at(from) > 0 && "all lengths must be positive.");
  }

  throw std::logic_error("Floor Division inconsistent with cmp()/subtract()");
}

template <typename T>
std::vector<mpq_class> Lengths<T>::coefficients(Label label) const {
  return Arithmetic<T>::coefficients(at(label));
}

template <typename T>
std::string Lengths<T>::render(Label label) const {
  std::string ret;
  size_t current = index(label);
  while (current || ret.size() == 0) {
    size_t offset = current % (2u * 26u);
    if (offset < 26) {
      ret += static_cast<char>('a' + offset);
    } else {
      ret += static_cast<char>('A' + (offset - 26));
    }
    current /= (2 * 26);
  }
  return ret;
}

template <typename T>
::intervalxt::Lengths Lengths<T>::only(const std::unordered_set<Label>& labels) const {
  auto only = Lengths(lengths);
  for (const auto label : this->labels())
    if (labels.find(label) == labels.end())
      only.at(label) = T();
  return only;
}

template <typename T>
::intervalxt::Lengths Lengths<T>::forget() const {
  return Lengths(lengths);
}

template <typename T>
bool Lengths<T>::operator==(const Lengths& other) const {
  return lengths == other.lengths;
}

template <typename T>
bool Lengths<T>::similar(Label a, Label b, const ::intervalxt::Lengths& other, Label aa, Label bb) const {
  const auto& x = at(a);
  const auto otherx = other.get(aa);

  if (!x && !otherx)
    return true;

  const auto& y = at(b);
  const auto othery = other.get(bb);

  if (!y && !othery)
    return true;

  return x*othery == y*otherx;
}

}  // namespace intervalxt::sample

#endif
