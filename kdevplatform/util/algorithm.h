/*
    SPDX-FileCopyrightText: 2020 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ALGORITHM_H
#define KDEVPLATFORM_ALGORITHM_H

#include <QSet>

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

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
template <typename T>
QSet<T> unite(std::vector<QSet<T>>&& sets)
{
    return unite(std::make_move_iterator(sets.begin()),
                 std::make_move_iterator(sets.end()));
}

template<typename Iterator>
struct InsertionResult
{
    Iterator iterator; ///< an iterator pointing at the inserted item or at the existing element equal to the value
    bool inserted; ///< whether the value was inserted (in other words, whether it was absent from the set)
};

// NOTE: two separate type names are used to make @p value a forwarding reference.
/**
 * Same as @p set.insert(@p value) but returns more information
 */
template<typename ElementType, typename ValueType>
InsertionResult<typename QSet<ElementType>::iterator> insert(QSet<ElementType>& set, ValueType&& value)
{
    const auto oldSize = set.size();
    auto it = set.insert(std::forward<ValueType>(value));
    Q_ASSERT(set.size() >= oldSize);
    Q_ASSERT(set.size() <= oldSize + 1);
    return {std::move(it), set.size() != oldSize};
}
}

#endif // KDEVPLATFORM_ALGORITHM_H
