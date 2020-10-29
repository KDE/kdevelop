/* Algorithm

    Copyright 2020 Igor Kushnir <igorkuo@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_ALGORITHM_H
#define KDEVPLATFORM_ALGORITHM_H

#include <QSet>

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace Algorithm {
/**
 * Computes the union of the QSet range [first, last) efficiently.
 *
 * @note Pass move iterators to this function if possible. Otherwise the largest set will be
 * detached and copied if there is more than one non-empty set in the range [first, last).
 */
template <typename ForwardIt>
auto unite(ForwardIt first, ForwardIt last)
{
    if (first == last) {
        return std::remove_cv_t<std::remove_reference_t<decltype(*first)>>{};
    }
    const auto maxElement = std::max_element(first, last, [](const auto& a, const auto& b) {
        return a.size() < b.size();
    });
    Q_ASSERT(maxElement != last);

    // Start with the largest-size set. None of this set's elements is inserted
    // into another set, so picking the largest one achieves optimum performance.
    auto result = *maxElement;
    for ( ; first != maxElement; ++first) {
        result.unite(*first);
    }
    // skip the already included *maxElement
    for (++first; first != last; ++first) {
        result.unite(*first);
    }
    return result;
}

/**
 * This is an overloaded convenience function.
 */
template <typename T, template<typename> class Container>
QSet<T> unite(Container<QSet<T>>&& sets)
{
    return unite(std::make_move_iterator(sets.begin()),
                 std::make_move_iterator(sets.end()));
}
}

#endif // KDEVPLATFORM_ALGORITHM_H
