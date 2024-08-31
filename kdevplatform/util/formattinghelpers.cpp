/*
    SPDX-FileCopyrightText: 2011 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "formattinghelpers.h"

#include "debug.h"

#include <QChar>
#include <QString>
#include <QStringView>
#include <QVarLengthArray>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <utility>

namespace {

bool asciiStringContains(const char* str, QChar c)
{
    constexpr ushort maxAscii{127};
    return c.unicode() <= maxAscii && std::strchr(str, c.unicode());
}

bool isFuzzy(QChar c)
{
    return asciiStringContains("{}()\"/\\*", c);
}

bool isFuzzyBracket(QChar c)
{
    return asciiStringContains("{}()", c);
}

class DoubleQuoteValidator
{
public:
    /**
     * Disable matching and clearing two consecutive inserted or removed double quotes.
     *
     * A double quote is closed by the next double quote by default.
     * Once this function is called, such a sequence is considered invalid and reported as a match failure.
     */
    void disableMatchingDoubleQuotes()
    {
        m_matchDoubleQuotes = false;
    }

    /**
     * Insert or remove a double quote.
     * @return @c false if double quote matching fails.
     */
    bool addDoubleQuote(bool isInserted);

    bool hasUnmatchedDoubleQuote() const
    {
        return m_unmatchedDoubleQuote;
    }

    /**
     * Finish validation.
     * @return @c false if double quote matching fails.
     */
    bool validate()
    {
        if (m_unmatchedDoubleQuote) {
            printFailureWarning();
            return false;
        }
        return true;
    }

    static void printFailureWarning()
    {
        qCWarning(UTIL) << "giving up formatting because the formatter inserted or removed "
                           "a pair of double quotes across context-text boundaries";
    }

private:
    bool m_matchDoubleQuotes = true;
    /// Whether a double quote has been inserted or removed and not yet matched.
    bool m_unmatchedDoubleQuote = false;
    /// Whether the unmatched double quote was inserted or removed.
    /// This data member has a meaning only if @a m_unmatchedDoubleQuote equals @c true.
    bool m_unmatchedDoubleQuoteWasInserted = false;
};

bool DoubleQuoteValidator::addDoubleQuote(bool isInserted)
{
    if (!m_unmatchedDoubleQuote) {
        m_unmatchedDoubleQuote = true;
        m_unmatchedDoubleQuoteWasInserted = isInserted;
        return true;
    }

    if (m_matchDoubleQuotes || m_unmatchedDoubleQuoteWasInserted != isInserted) {
        // Either the two double quotes are matched and cleared, or one was inserted and another removed,
        // which unconditionally cancels them out (could be some code reordering by the formatter).
        m_unmatchedDoubleQuote = false;
        return true;
    }

    // Fail because of two consecutive inserted or removed double quotes, matching which has been disabled.
    printFailureWarning();
    return false;
}

/**
 * Tracks and validates inserted and removed, opening and closing brackets of a single type, e.g. '{' and '}'.
 */
class BracketStack
{
public:
    /**
     * Insert or remove a bracket.
     */
    void addBracket(bool isOpening, bool isInserted)
    {
        const auto countIncrement = isInserted ? 1 : -1;
        if (!m_data.empty()) {
            auto& last = m_data.back();
            if (last.isOpening == isOpening) {
                last.count += countIncrement;
                if (last.count == 0) {
                    m_data.pop_back(); // remove the now empty sequence
                }
                return;
            }
        }

        m_data.push_back({isOpening, countIncrement});
    }

    /**
     * Validate brackets once parsing ends.
     * @return @c false if matching brackets fails.
     * @note Validation is destructive, that is, it transitions this object into an undefined state.
     */
    bool validate()
    {
        // Inserted opening brackets normally match inserted closing brackets. Same for removed brackets.
        // In addition, inserted opening brackets match removed opening brackets. Same for closing brackets.
        // An inserted-removed match can mean that an unformatted-formatted text matcher misinterpreted the formatter's
        // intention (other fuzzy characters could actually be inserted or removed near the untouched bracket).
        // Such a misinterpretation is not a problem at all thanks to the inserted-removed match.

        // We move backwards, so closing brackets must be encountered before the matching open brackets.
        int closingBracketCount = 0;
        for (int i = m_data.size() - 1; i >= 0; --i) {
            auto p = m_data.at(i);
            Q_ASSERT(p.count != 0);

            if (p.isOpening) {
                if ((closingBracketCount > 0) != (p.count > 0)) {
                    // Insertion/removal mismatch of opening and closing brackets. When a closing bracket is
                    // inserted and an opening bracket is removed (or vice versa), they clearly cannot match.
                    return false;
                }
                if (std::abs(closingBracketCount) < std::abs(p.count)) {
                    return false; // not enough closing brackets to match all opening brackets
                }
                closingBracketCount -= p.count;
            } else {
                closingBracketCount += p.count;
            }
        }
        return closingBracketCount == 0; // are all closing brackets matched by opening brackets?
    }

private:
    struct BracketSequence
    {
        bool isOpening : 1; ///< whether the brackets in this sequence are of the opening type
        int count : 31; ///< abs(count) is the number of consecutive inserted (count > 0) or removed (count < 0) brackets
    };

    QVarLengthArray<BracketSequence, 64> m_data;
};

class BracketValidator
{
public:
    /**
     * Insert or remove a fuzzy bracket.
     * @pre isFuzzyBracket(@p bracket)
     */
    void addBracket(QChar bracket, bool isInserted);

    /// Insert or remove an opening "/*" or closing "*/" comment sequence.
    void addCommentSequence(bool isOpening, bool isInserted)
    {
        m_stacks[commentsIndex].addBracket(isOpening, isInserted);
    }

    /**
     * Validate brackets and comments once parsing ends.
     * @return @c false if matching brackets or comments fails.
     * @note Validation is destructive, that is, it transitions this object into an undefined state.
     */
    bool validate()
    {
        for (std::size_t i = 0; i < m_stacks.size(); ++i) {
            if (!m_stacks[i].validate()) {
                qCWarning(UTIL) << "giving up formatting because the formatter inserted or removed the pair"
                                << m_stackStrings[i] << "across context-text boundaries";
                return false;
            }
        }
        return true;
    }

private:
    // the following 3 constants are indices into m_stacks and m_stackStrings
    static constexpr std::size_t bracesIndex = 0; ///< { }
    static constexpr std::size_t parenthesesIndex = 1; ///< ( )
    static constexpr std::size_t commentsIndex = 2; ///< /* */
    /// human-readable identifications of fuzzy bracket types
    static constexpr std::array<const char*, 3> m_stackStrings = {"{ }", "( )", "/* */"};

    std::array<BracketStack, 3> m_stacks; ///< stacks of fuzzy brackets of each type
};

void BracketValidator::addBracket(QChar bracket, bool isInserted)
{
    Q_ASSERT(isFuzzyBracket(bracket));

    std::size_t index;
    bool isOpening;
    switch (bracket.unicode()) {
    case '{':
        index = bracesIndex;
        isOpening = true;
        break;
    case '}':
        index = bracesIndex;
        isOpening = false;
        break;
    case '(':
        index = parenthesesIndex;
        isOpening = true;
        break;
    case ')':
        index = parenthesesIndex;
        isOpening = false;
        break;
    default:
        Q_UNREACHABLE();
    }

    m_stacks[index].addBracket(isOpening, isInserted);
}

enum class FuzzyMatchResult { Fuzzy, NotFuzzy, MatchingFailed };

class FuzzyMatcher
{
    Q_DISABLE_COPY_MOVE(FuzzyMatcher)
public:
    virtual ~FuzzyMatcher() = default;
    /**
     * Inserts (if @p isInserted == @c true) or removes (otherwise) a single character at
     * the specified address @p characterLocation.
     *
     * Prints an explaining warning when @c FuzzyMatchResult::MatchingFailed is returned.
     *
     * @param characterLocation a valid character address into a complete string or view.
     *        The address is used to group consecutive characters '/' and '*' into a C-style comment.
     * @return the added character's classification (fuzzy or not) or @c MatchingFailed if a match failure occurs.
     */
    virtual FuzzyMatchResult add(const QChar* characterLocation, bool isInserted) = 0;

    /**
     * Informs this matcher that the first exact match of a character in unformatted and formatted text just occurred.
     *
     * May be called twice. For example, the first call right after construction indicates that the ranges
     * do not begin at a context-text boundary, and thus the matcher should not fail if a character, which is
     * forbidden at a boundary, is removed or inserted before the second (regular) call when the exact match occurs.
     */
    virtual void firstNonWhitespaceCharacterMatch() = 0;
    /**
     * Specifies the locations in the two matched ranges of the last exact match of a character.
     *
     * Call this function if the ranges end at a context-text boundary.
     * The function must be called after all fuzzy characters, which precede
     * the last-match addresses in the iteration order, have been added.
     * Prints an explaining warning when @c false is returned.
     *
     * @param removalRangeMatchAddress the address of the last matching character (or the very first
     *                                 address if there were no matches) in the complete string or view,
     *                                 from which fuzzy characters can be removed.
     * @param insertionRangeMatchAddress the address of the last matching character (or the very first
     *                                   address if there were no matches) in the complete string or view,
     *                                   into which fuzzy characters can be inserted.
     * @return @c false if matching fails immediately.
     */
    virtual bool lastNonWhitespaceCharacterMatch(const QChar* removalRangeMatchAddress,
                                                 const QChar* insertionRangeMatchAddress) = 0;

protected:
    FuzzyMatcher() = default;
};

/**
 * This class reports a match failure if a double quote is removed or inserted at the boundary between context and text.
 * That is, after a nonempty left context and before the first exact non-whitespace character match between
 * prefix and text, or after the last exact non-whitespace character match and before text or nonempty right context.
 *
 * Such a removal or insertion should cancel formatting, because whitespace within string literals is significant,
 * but the implementation of formatted text extraction is not sophisticated enough to distinguish it from regular
 * whitespace that is subject to formatting manipulations. So the combination of a double quote insertion/removal
 * by a formatter and an unlucky selection of a text fragment for formatting can cause whitespace changes within
 * a string literal. The purpose of this class is to reduce the probability of such a quiet breaking code change.
 */
class BoundaryFuzzyMatcher : public FuzzyMatcher
{
public:
    /**
     * Indicates that higher addresses are encountered before lower addresses (reverse iteration).
     */
    void setReverseAddressDirection()
    {
        m_reverseAddressDirection = true;
    }

    void firstNonWhitespaceCharacterMatch() override
    {
        m_allowDoubleQuoteChanges = true;
    }

    bool lastNonWhitespaceCharacterMatch(const QChar* removalRangeMatchAddress,
                                         const QChar* insertionRangeMatchAddress) override
    {
        if (!validate(m_lastRemovedDoubleQuoteAddress, removalRangeMatchAddress)) {
            printFailureWarning(false);
            return false;
        }
        if (!validate(m_lastInsertedDoubleQuoteAddress, insertionRangeMatchAddress)) {
            printFailureWarning(true);
            return false;
        }
        m_allowDoubleQuoteChanges = false;
        return true;
    }

protected:
    /**
     * Derived classes must call this function each time a double quote is passed to add().
     * @return @c false if matching fails immediately.
     */
    bool addDoubleQuote(const QChar* location, bool isInserted)
    {
        Q_ASSERT(location);
        Q_ASSERT(*location == QLatin1Char{'"'});
        if (!m_allowDoubleQuoteChanges) {
            printFailureWarning(isInserted);
            return false;
        }
        (isInserted ? m_lastInsertedDoubleQuoteAddress : m_lastRemovedDoubleQuoteAddress) = location;
        return true;
    }

private:
    bool validate(const QChar* lastDoubleQuoteAddress, const QChar* lastMatchAddress) const
    {
        if (!lastDoubleQuoteAddress) {
            return true;
        }
        // lastDoubleQuoteAddress can equal lastMatchAddress in case there were no matches (lastMatchAddress
        // is the very first address, i.e. the beginning of the range, then), and the formatter happened to
        // remove or insert a double quote at this same address. In this case we correctly return false.
        return m_reverseAddressDirection ? lastDoubleQuoteAddress > lastMatchAddress
                                         : lastDoubleQuoteAddress < lastMatchAddress;
    }

    static void printFailureWarning(bool isInserted)
    {
        qCWarning(UTIL) << "giving up formatting because the formatter" << (isInserted ? "inserted" : "removed")
                        << "a double quote at a context-text boundary";
    }

    bool m_reverseAddressDirection = false;
    bool m_allowDoubleQuoteChanges = false;
    const QChar* m_lastRemovedDoubleQuoteAddress = nullptr;
    const QChar* m_lastInsertedDoubleQuoteAddress = nullptr;
};

class DoubleQuoteFuzzyMatcher : public BoundaryFuzzyMatcher
{
public:
    FuzzyMatchResult add(const QChar* characterLocation, bool isInserted) override
    {
        Q_ASSERT(characterLocation);
        const auto c = *characterLocation;
        if (c == QLatin1Char{'"'}) {
            const bool valid = addDoubleQuote(characterLocation, isInserted) && m_validator.addDoubleQuote(isInserted);
            return valid ? FuzzyMatchResult::Fuzzy : FuzzyMatchResult::MatchingFailed;
        }
        return isFuzzy(c) ? FuzzyMatchResult::Fuzzy : FuzzyMatchResult::NotFuzzy;
    }

    bool hasUnmatchedDoubleQuote() const
    {
        return m_validator.hasUnmatchedDoubleQuote();
    }

private:
    DoubleQuoteValidator m_validator;
};

/**
 * A fuzzy matcher that tracks and validates double quotes, brackets and comments.
 *
 * Supports only direct iteration over a string [view], that is, each added insertion
 * address must be greater than all previous added insertion addresses. Same for removal addresses.
 */
class CompleteFuzzyMatcher : public BoundaryFuzzyMatcher
{
public:
    struct Range
    {
        const QChar* first;
        const QChar* last;
    };

    /**
     * @param removalRange the address range of the complete string or view,
     *                     from which fuzzy characters can be removed.
     * @param insertionRange the address range of the complete string or view,
     *                       into which fuzzy characters can be inserted.
     */
    explicit CompleteFuzzyMatcher(Range removalRange, Range insertionRange)
        : m_validRanges{removalRange, insertionRange}
    {
    }

    void disableMatchingDoubleQuotes()
    {
        m_doubleQuoteValidator.disableMatchingDoubleQuotes();
    }

    FuzzyMatchResult add(const QChar* characterLocation, bool isInserted) override
    {
        Q_ASSERT(characterLocation);
        const auto& validRange = m_validRanges[isInserted];
        Q_ASSERT(characterLocation >= validRange.first);
        Q_ASSERT(characterLocation < validRange.last);

        const auto c = *characterLocation;
        switch (c.unicode()) {
        case '"': {
            const bool valid =
                addDoubleQuote(characterLocation, isInserted) && m_doubleQuoteValidator.addDoubleQuote(isInserted);
            return valid ? FuzzyMatchResult::Fuzzy : FuzzyMatchResult::MatchingFailed;
        }
        case '/':
        case '*': {
            const auto*& prev = m_lastEndOfCommentAddresses[isInserted];
            Q_ASSERT(!prev || prev <= characterLocation);
            if (prev == characterLocation) {
                // a comment sequence ending at characterLocation has been added already, do nothing else
            } else if (characterLocation > validRange.first && prev != characterLocation - 1
                       && addIfCommentSequence(*(characterLocation - 1), c, isInserted)) {
                // NOTE: the (prev != characterLocation - 1) check above prevents
                // finding two comment sequences in the string "/*/".
                prev = characterLocation;
            } else if (characterLocation + 1 < validRange.last
                       && addIfCommentSequence(c, *(characterLocation + 1), isInserted)) {
                prev = characterLocation + 1;
            }
            return FuzzyMatchResult::Fuzzy;
        }
        default:
            if (!isFuzzy(c)) {
                return FuzzyMatchResult::NotFuzzy;
            }
            if (isFuzzyBracket(c)) {
                m_bracketValidator.addBracket(c, isInserted);
            }
            return FuzzyMatchResult::Fuzzy;
        }
    }

    bool validate()
    {
        return m_doubleQuoteValidator.validate() && m_bracketValidator.validate();
    }

private:
    bool addIfCommentSequence(QChar a, QChar b, bool isInserted)
    {
        bool isOpening;
        if (a == QLatin1Char{'/'} && b == QLatin1Char{'*'}) {
            isOpening = true;
        } else if (a == QLatin1Char{'*'} && b == QLatin1Char{'/'}) {
            isOpening = false;
        } else {
            return false;
        }
        m_bracketValidator.addCommentSequence(isOpening, isInserted);
        return true;
    }

    // NOTE: both array data members are indexed by isInserted.

    const std::array<Range, 2> m_validRanges;
    /// Each element is the address of the second part of a C-style comment sequence, which was
    /// last added as a removed/inserted comment. So each element equals @c nullptr or points to '/' or '*'.
    std::array<const QChar*, 2> m_lastEndOfCommentAddresses{nullptr, nullptr};

    DoubleQuoteValidator m_doubleQuoteValidator;
    BracketValidator m_bracketValidator;
};

template<typename ForwardIt>
void skipWhitespace(ForwardIt& first, ForwardIt last)
{
    first = std::find_if_not(first, last, [](QChar c) {
        return c.isSpace();
    });
}

template<typename ForwardIt>
struct FindResult
{
    bool found = false; ///< whether the needle has been found
    int fuzzyCount = 0; ///< the number of fuzzy characters skipped before @a location
    ForwardIt location; ///< the location of the needle or a non-whitespace non-fuzzy character, or the search range end
};

/**
 * Finds @p needle in [@p first, @p last).
 *
 * If a non-whitespace non-fuzzy character is encountered before @p needle,
 * sets the returned result's @a found to @c false and returns the said character's location.
 */
template<typename ForwardIt>
FindResult<ForwardIt> findUntilNeitherFuzzyNorWhitespace(ForwardIt first, ForwardIt last, QChar needle)
{
    FindResult<ForwardIt> result;
    for (; first != last; ++first) {
        if (*first == needle) {
            result.found = true;
            break;
        }
        if (first->isSpace()) {
        } else if (isFuzzy(*first)) {
            ++result.fuzzyCount;
        } else {
            break;
        }
    }
    result.location = first;
    return result;
}

/**
 * Advances @p first until it points to a neither fuzzy nor whitespace character or until
 * @p fuzzyMatcher fails or until @p first becomes equal to @p last, whichever happens first.
 *
 * @pre @p first != @p last && @p !first->isSpace()
 * @post @p first == @p last if and only if only whitespace and fuzzy characters
 *       have been encountered and @p fuzzyMatcher hasn't failed.
 * @return the last return value of FuzzyMatcher::add()
 */
template<typename ForwardIt>
FuzzyMatchResult skipFuzzyAndWhitespace(ForwardIt& first, ForwardIt last, FuzzyMatcher& fuzzyMatcher, bool isInserted)
{
    Q_ASSERT(first != last);
    Q_ASSERT(!first->isSpace());
    do {
        const auto result = fuzzyMatcher.add(&*first, isInserted);
        if (result != FuzzyMatchResult::Fuzzy) {
            return result;
        }
        ++first;
        skipWhitespace(first, last);
    } while (first != last);
    return FuzzyMatchResult::Fuzzy;
}

/**
 * Matches the given unformatted prefix against the given formatted text.
 * Skips whitespace. Skips fuzzy (defined by isFuzzy()) characters using the given @c FuzzyMatcher.
 */
template<typename ForwardIt>
class PrefixMatcher
{
public:
    explicit PrefixMatcher(ForwardIt prefixFirst, ForwardIt prefixLast, ForwardIt textFirst, ForwardIt textLast,
                           FuzzyMatcher& fuzzyMatcher)
        : m_prefixFirst{prefixFirst}
        , m_prefixLast{prefixLast}
        , m_textFirst{textFirst}
        , m_textLast{textLast}
        , m_fuzzyMatcher{fuzzyMatcher}
    {
    }

    struct Result
    {
        /// @c true if the prefix has been matched successfully
        bool hasMatched;
        /// a text iterator that points to where the prefix match ends, or where a mismatch or match failure occurs
        ForwardIt matchEnd;
    };

    /// Call this destructive function once, because it transitions this object into an undefined state.
    Result match(bool isEndAContextTextBoundary = true)
    {
        bool characterMatchOccurred = false;
        auto lastPrefixCharacterMatchIt = m_prefixFirst;
        auto lastTextCharacterMatchIt = m_textFirst;
        for (;; ++m_textFirst, ++m_prefixFirst) {
            skipWhitespace(m_prefixFirst, m_prefixLast);
            if (m_prefixFirst == m_prefixLast) {
                setResult(HasMatched::Yes);
                break;
            }

            skipWhitespace(m_textFirst, m_textLast);
            if (m_textFirst == m_textLast) {
                skipFuzzyAndWhitespace(m_prefixFirst, m_prefixLast, m_fuzzyMatcher, /*isInserted=*/false);
                if (m_prefixFirst == m_prefixLast) {
                    setResult(HasMatched::Yes);
                } else {
                    setUnmatchedPrefixCharacterResult();
                }
                break;
            }

            if (*m_prefixFirst != *m_textFirst && !skipToMatchingPositions()) {
                break;
            }

            if (*m_prefixFirst == *m_textFirst) {
                if (!characterMatchOccurred) {
                    characterMatchOccurred = true;
                    m_fuzzyMatcher.firstNonWhitespaceCharacterMatch();
                }
                lastPrefixCharacterMatchIt = m_prefixFirst;
                lastTextCharacterMatchIt = m_textFirst;
            }
        }

        if (isEndAContextTextBoundary && m_result.hasMatched) {
            m_result.hasMatched = m_fuzzyMatcher.lastNonWhitespaceCharacterMatch(&*lastPrefixCharacterMatchIt,
                                                                                 &*lastTextCharacterMatchIt);
        }

        return m_result;
    }

private:
    /**
     * Skips fuzzy and whitespace characters in prefix and text until @a *m_prefixFirst == @a *m_textFirst
     * or until a fuzzy @a *m_textFirst replaces a fuzzy @a *m_prefixFirst using @a m_fuzzyMatcher.
     *
     * @return @c true in case of success;
     *         @c false in case of no matching characters left (successful match),
     *                  unrecoverable mismatch or match failure.
     * @post @a m_result is set and ready to be returned if this function returns @c false.
     */
    bool skipToMatchingPositions()
    {
        Q_ASSERT(m_prefixFirst != m_prefixLast);
        Q_ASSERT(!m_prefixFirst->isSpace());

        Q_ASSERT(m_textFirst != m_textLast);
        Q_ASSERT(!m_textFirst->isSpace());

        Q_ASSERT(*m_prefixFirst != *m_textFirst);

        const bool prefixIsFuzzy = isFuzzy(*m_prefixFirst);
        const bool textIsFuzzy = isFuzzy(*m_textFirst);
        if (!prefixIsFuzzy && !textIsFuzzy) {
            setUnrecoverableMismatchResult();
            return false;
        }
        if (prefixIsFuzzy != textIsFuzzy) {
            auto& first = prefixIsFuzzy ? m_prefixFirst : m_textFirst;
            const auto last = prefixIsFuzzy ? m_prefixLast : m_textLast;
            const auto result = skipFuzzyAndWhitespace(first, last, m_fuzzyMatcher, /*isInserted=*/textIsFuzzy);
            switch (result) {
            case FuzzyMatchResult::Fuzzy:
                Q_ASSERT(first == last);
                if (prefixIsFuzzy) {
                    setResult(HasMatched::Yes);
                } else {
                    setUnmatchedPrefixCharacterResult();
                }
                return false;
            case FuzzyMatchResult::NotFuzzy:
                if (*m_prefixFirst == *m_textFirst) {
                    return true;
                }
                setUnrecoverableMismatchResult();
                return false;
            case FuzzyMatchResult::MatchingFailed:
                setResult(HasMatched::No);
                return false;
            }
            Q_UNREACHABLE();
        }
        Q_ASSERT(prefixIsFuzzy && textIsFuzzy);
        return selectFuzzyInsertionRemovalOrReplacement();
    }

    /// Chooses between valid insertion and valid removal.
    bool shouldRemove(const FindResult<ForwardIt>& insertionResult, const FindResult<ForwardIt>& removalResult) const
    {
        Q_ASSERT(insertionResult.found);
        Q_ASSERT(removalResult.found);

        // prefer inserting/removing fewer fuzzy characters
        if (removalResult.fuzzyCount != insertionResult.fuzzyCount) {
            return removalResult.fuzzyCount < insertionResult.fuzzyCount;
        }

        // break the tie: prefer inserting/removing fewer whitespace characters
        const auto removalDistance = std::distance(m_prefixFirst, removalResult.location);
        const auto insertionDistance = std::distance(m_textFirst, insertionResult.location);
        if (removalDistance != insertionDistance) {
            return removalDistance < insertionDistance;
        }

        // break the tie: consider an insertion (of e.g. a brace) more likely
        return false;
    }

    /**
     * Implements the fuzzy prefix and fuzzy text characters case of skipToMatchingPositions()
     * by selecting the likeliest of the 4 possibilities:
     * 1) the formatter kept *m_prefixFirst but inserted fuzzy characters before it in text;
     * 2) the formatter kept *m_textFirst but removed fuzzy characters before it in prefix;
     * 3) the formatter replaced *m_prefixFirst with *m_textFirst;
     * 4) the formatter removed all (fuzzy and whitespace) characters in the range [m_prefixFirst, m_prefixLast).
     */
    bool selectFuzzyInsertionRemovalOrReplacement()
    {
        Q_ASSERT(m_prefixFirst != m_prefixLast);
        Q_ASSERT(isFuzzy(*m_prefixFirst));

        Q_ASSERT(m_textFirst != m_textLast);
        Q_ASSERT(isFuzzy(*m_textFirst));

        Q_ASSERT(*m_prefixFirst != *m_textFirst);

        auto insertionResult = findUntilNeitherFuzzyNorWhitespace(m_textFirst, m_textLast, *m_prefixFirst);
        auto removalResult = findUntilNeitherFuzzyNorWhitespace(m_prefixFirst, m_prefixLast, *m_textFirst);

        if (removalResult.location == m_prefixLast) {
            Q_ASSERT(!removalResult.found);
            // The formatter could have removed all remaining characters in prefix (the 4th possibility). When
            // removalResult.found is true, this possibility is strictly worse than the one stored in removalResult
            // (the 2nd possibility). In this scope removalResult.found is false, therefore, the 2nd possibility is
            // unavailable and the 4th possibility is essentially stored in removalResult instead.

            if (!insertionResult.found) {
                // The formatter must have removed *m_prefixFirst, because insertionResult.found is false, and so the
                // 1st possibility is unavailable. Choose among the (roughly) 3rd and the 4th possibilities like this:
                // 1. Skip fuzzy characters in prefix, starting from *m_prefixFirst.
                // 2. When only one fuzzy character absent from the range
                //    [m_textFirst, insertionResult.location) remains in prefix (the loop condition),
                //    leave insertionResult.found == false and thus select the 4th possibility.
                // 3. When the findUntilNeitherFuzzyNorWhitespace() call below finds a prefix's fuzzy
                //    character in text, insertionResult.found is set to true and the code below chooses
                //    between the 3rd and the 4th possibility (with an increased value of m_prefixFirst).

                // skippedFuzzySet is a set of skipped fuzzy characters in prefix that are absent from
                // the range [m_textFirst, insertionResult.location). This set is used only for optimization:
                // to avoid searching for the same fuzzy character in text more than once.
                QVarLengthArray<QChar, 4> skippedFuzzySet{*m_prefixFirst};
                while (--removalResult.fuzzyCount > 0) {
                    const auto result = m_fuzzyMatcher.add(&*m_prefixFirst, /*isInserted=*/false);
                    if (result == FuzzyMatchResult::MatchingFailed) {
                        setResult(HasMatched::No);
                        return false;
                    }
                    Q_ASSERT(result == FuzzyMatchResult::Fuzzy);
                    ++m_prefixFirst;

                    skipWhitespace(m_prefixFirst, m_prefixLast);
                    Q_ASSERT_X(m_prefixFirst != m_prefixLast, Q_FUNC_INFO, "Wrong value of removalResult.fuzzyCount?");
                    Q_ASSERT(isFuzzy(*m_prefixFirst));

                    if (skippedFuzzySet.indexOf(*m_prefixFirst) != -1) {
                        continue; // do not fruitlessly search for the same fuzzy character in text again
                    }

                    insertionResult = findUntilNeitherFuzzyNorWhitespace(m_textFirst, m_textLast, *m_prefixFirst);
                    if (insertionResult.found) {
                        break; // let the code below choose between valid insertion and valid removal
                    }
                    skippedFuzzySet.push_back(*m_prefixFirst);
                }
            }

            // Mark removalResult as valid in order to let the code below consider the 4th possibility stored in it.
            removalResult.found = true;
        } else if (!insertionResult.found && !removalResult.found) {
            // *m_textFirst replaces *m_prefixFirst
            bool isInserted = false;
            for (auto it : {m_prefixFirst, m_textFirst}) {
                const auto result = m_fuzzyMatcher.add(&*it, isInserted);
                if (result == FuzzyMatchResult::MatchingFailed) {
                    setResult(HasMatched::No);
                    return false;
                }
                Q_ASSERT(result == FuzzyMatchResult::Fuzzy);

                isInserted = true;
            }
            return true;
        }

        if (insertionResult.found && removalResult.found) {
            const bool remove = shouldRemove(insertionResult, removalResult);
            // select insertion or removal by disabling the losing choice
            (remove ? insertionResult : removalResult).found = false;
        }

        bool isInserted;
        ForwardIt* first; // this is a pointer in order to automatically modify the referenced data member
        ForwardIt last;
        if (insertionResult.found) {
            Q_ASSERT(!removalResult.found);
            isInserted = true;
            first = &m_textFirst;
            last = insertionResult.location;
        } else {
            Q_ASSERT(removalResult.found);
            isInserted = false;
            first = &m_prefixFirst;
            last = removalResult.location;
        }

        const auto result = skipFuzzyAndWhitespace(*first, last, m_fuzzyMatcher, isInserted);
        if (result == FuzzyMatchResult::MatchingFailed) {
            setResult(HasMatched::No);
            return false;
        }
        Q_ASSERT(result == FuzzyMatchResult::Fuzzy);
        Q_ASSERT(*first == last);

        Q_ASSERT(m_textFirst != m_textLast);
        if (m_prefixFirst == m_prefixLast) {
            setResult(HasMatched::Yes);
            return false;
        }
        Q_ASSERT(*m_prefixFirst == *m_textFirst);
        return true;
    }

    enum class HasMatched { No, Yes };

    void setResult(HasMatched hasMatched)
    {
        Q_ASSERT_X(hasMatched == HasMatched::No || m_prefixFirst == m_prefixLast, Q_FUNC_INFO,
                   "The entire prefix must be examined before reporting a successful match.");

        m_result.hasMatched = hasMatched == HasMatched::Yes;
        m_result.matchEnd = m_textFirst;
    }

    void setUnmatchedPrefixCharacterResult()
    {
        Q_ASSERT(m_prefixFirst != m_prefixLast);
        Q_ASSERT(!m_prefixFirst->isSpace());
        Q_ASSERT(!isFuzzy(*m_prefixFirst));

        Q_ASSERT(m_textFirst == m_textLast);

        qCWarning(UTIL) << "giving up formatting because of a character in prefix not matched in text:"
                        << *m_prefixFirst;
        setResult(HasMatched::No);
    }

    void setUnrecoverableMismatchResult()
    {
        Q_ASSERT(m_prefixFirst != m_prefixLast);
        Q_ASSERT(!m_prefixFirst->isSpace());
        Q_ASSERT(!isFuzzy(*m_prefixFirst));

        Q_ASSERT(m_textFirst != m_textLast);
        Q_ASSERT(!m_textFirst->isSpace());
        Q_ASSERT(!isFuzzy(*m_textFirst));

        Q_ASSERT(*m_prefixFirst != *m_textFirst);

        qCWarning(UTIL) << "giving up formatting because of unrecoverable character mismatch:" << *m_prefixFirst
                        << "!=" << *m_textFirst;
        setResult(HasMatched::No);
    }

    ForwardIt m_prefixFirst;
    const ForwardIt m_prefixLast;
    ForwardIt m_textFirst;
    const ForwardIt m_textLast;

    FuzzyMatcher& m_fuzzyMatcher;

    Result m_result{false, m_textFirst};
};

/**
 * Matches the given unformatted text against the given formatted text exactly and validates the match.
 * Skips whitespace. Skips fuzzy (defined by isFuzzy()) characters using the given @p fuzzyMatcher.
 * @return whether the match was successful.
 */
bool matchFormattedText(const QString& text, QStringView formattedText, CompleteFuzzyMatcher&& fuzzyMatcher,
                        bool isEndAContextTextBoundary)
{
    // This intermediary view guarantees that iterators passed to PrefixMatcher() are of the same type.
    const QStringView textView = text;
    PrefixMatcher prefixMatcher(textView.cbegin(), textView.cend(), formattedText.cbegin(), formattedText.cend(),
                                fuzzyMatcher);
    auto result = prefixMatcher.match(isEndAContextTextBoundary);
    if (!result.hasMatched) {
        return false;
    }

    // Skip to verify that only whitespace and fuzzy characters remain in the formatted text after result.matchEnd.
    // Satisfy the preconditions of skipFuzzyAndWhitespace() by skipping whitespace
    // and checking whether the range is empty before calling it.
    skipWhitespace(result.matchEnd, formattedText.cend());
    if (result.matchEnd != formattedText.cend()) {
        const auto fuzzyResult =
            skipFuzzyAndWhitespace(result.matchEnd, formattedText.cend(), fuzzyMatcher, /*isInserted=*/true);
        if (fuzzyResult == FuzzyMatchResult::MatchingFailed) {
            return false;
        }
        if (result.matchEnd != formattedText.cend()) {
            Q_ASSERT(fuzzyResult == FuzzyMatchResult::NotFuzzy);
            qCWarning(UTIL) << "giving up formatting because of a character in formatted text not matched in text:"
                            << *result.matchEnd;
            return false;
        }
        Q_ASSERT(fuzzyResult == FuzzyMatchResult::Fuzzy);
    }

    return fuzzyMatcher.validate();
}

QString reverse(QStringView str)
{
    QString ret;
    ret.reserve(str.length());
    for (int a = str.length() - 1; a >= 0; --a)
        ret.append(str[a]);

    return ret;
}

// Returns the text start position with all whitespace that is redundant in the given context skipped
int skipRedundantWhiteSpace(QStringView context, QStringView text, int tabWidth)
{
    if (context.isEmpty() || !context[context.size() - 1].isSpace() || text.isEmpty() || !text[0].isSpace())
        return 0;

    int textPosition = 0;

    // Extract trailing whitespace in the context
    int contextPosition = context.size() - 1;
    while (contextPosition > 0 && context[contextPosition - 1].isSpace())
        --contextPosition;

    int textWhitespaceEnd = 0;
    while (textWhitespaceEnd < text.size() && text[textWhitespaceEnd].isSpace())
        ++textWhitespaceEnd;

    auto contextWhiteSpace = context.sliced(contextPosition);
    auto textWhiteSpace = text.first(textWhitespaceEnd);

    // Step 1: Remove redundant newlines
    while (contextWhiteSpace.contains(QLatin1Char('\n')) && textWhiteSpace.contains(QLatin1Char('\n'))) {
        int contextOffset = contextWhiteSpace.indexOf(QLatin1Char('\n')) + 1;
        int textOffset = textWhiteSpace.indexOf(QLatin1Char('\n')) + 1;

        contextWhiteSpace = contextWhiteSpace.sliced(contextOffset);

        textPosition += textOffset;
        textWhiteSpace = textWhiteSpace.sliced(textOffset);
    }

    int contextOffset = 0;
    int textOffset = 0;
    // Skip redundant ordinary whitespace
    while (contextOffset < contextWhiteSpace.size() && textOffset < textWhiteSpace.size() &&
           contextWhiteSpace[contextOffset].isSpace() && contextWhiteSpace[contextOffset] != QLatin1Char('\n') &&
           textWhiteSpace[textOffset].isSpace() && textWhiteSpace[textOffset] != QLatin1Char('\n')) {
        bool contextWasTab = contextWhiteSpace[contextOffset] == QLatin1Char('\t');
        bool textWasTab = textWhiteSpace[contextOffset] == QLatin1Char('\t');
        ++contextOffset;
        ++textOffset;
        if (contextWasTab != textWasTab) {
            // Problem: We have a mismatch of tabs and/or ordinary whitespaces
            if (contextWasTab) {
                for (int s = 1; s < tabWidth; ++s)
                    if (textOffset < textWhiteSpace.size() && textWhiteSpace[textOffset] == QLatin1Char(' '))
                        ++textOffset;
            } else if (textWasTab) {
                for (int s = 1; s < tabWidth; ++s)
                    if (contextOffset < contextWhiteSpace.size() &&
                        contextWhiteSpace[contextOffset] == QLatin1Char(' '))
                        ++contextOffset;
            }
        }
    }
    textPosition += textOffset;

    Q_ASSERT(textPosition >= 0);
    Q_ASSERT(textPosition <= text.size());
    return textPosition;
}

} // unnamed namespace

QString KDevelop::extractFormattedTextFromContext(const QString& _formattedMergedText, const QString& text,
                                                  QStringView leftContext, QStringView rightContext, int tabWidth)
{
    if (leftContext.isEmpty() && rightContext.isEmpty()) {
        return _formattedMergedText; // fast path, nothing to do
    }

    QStringView formattedMergedText = _formattedMergedText;
    //Now remove "leftContext" and "rightContext" from the sides

    // For double quotes, C-style comments and brackets of each type, ensure that inserted or removed opening characters
    // (or sequences) are matched by inserted or removed closing characters (or sequences) in the formatted version of
    // text. The matching closing entities should be encountered after the opening ones.
    // Tracking/matching only inserted and removed (ignoring untouched) fuzzy characters is important to prevent
    // issues with commented out and (conditionally) disabled by preprocessor code. The assumption is that formatters
    // do not break code and do not insert or remove unmatched quotes, comments or brackets into disabled code.

    // For simplicity, only track double quotes (and not comments or brackets) while prefix-matching the two
    // contexts. Brackets opened in the left context can be closed in the right context. Such closing could be
    // validated, but is not, in order to avoid further complicating the implementation. Validating quotes,
    // comments and all bracket types while matching text against its formatted counterpart should be sufficient.

    // Fuzzy characters inserted at context-text boundaries are always included into the formatted text fragment.
    // If this greedy inclusion produces a quote, comment or bracket mismatch, unformatted text is returned.
    // We could try to achieve a match by giving some of such inserted fuzzy characters to contexts. But that would
    // substantially complicate the implementation and likely worsen existing formatting. For example, a brace inserted
    // at a context-text boundary affects the surrounding whitespace. If we give the brace to the adjacent context,
    // it won't be part of the final version of formatted text, and therefore our attempts to compute whitespace to be
    // included into formatted text at the affected boundary would probably produce an unsatisfactory result.

    const auto handleDoubleQuoteMatchFailure = [] {
        // The formatter inserted or removed a double quote in exactly one of two contexts, which means that it
        // inserted/removed the matching double quote into the formatted text. Therefore, reformatting only
        // the text fragment would produce an unpaired quote and break code. Give up formatting to prevent that.
        DoubleQuoteValidator::printFailureWarning();
    };

    bool hasUnmatchedDoubleQuote = false;

    if (!leftContext.isEmpty()) {
        DoubleQuoteFuzzyMatcher fuzzyMatcher;
        // Inform the matcher that the beginning of the left context is not a context-text boundary.
        fuzzyMatcher.firstNonWhitespaceCharacterMatch();
        PrefixMatcher prefixMatcher(leftContext.cbegin(), leftContext.cend(), formattedMergedText.cbegin(),
                                    formattedMergedText.cend(), fuzzyMatcher);
        const auto result = prefixMatcher.match();
        if (!result.hasMatched) {
            return text;
        }

        hasUnmatchedDoubleQuote = fuzzyMatcher.hasUnmatchedDoubleQuote();
        if (hasUnmatchedDoubleQuote && rightContext.isEmpty()) {
            handleDoubleQuoteMatchFailure();
            return text;
        }

        // include all possible whitespace at the context-text boundary
        auto rMatchEnd = std::make_reverse_iterator(result.matchEnd);
        skipWhitespace(rMatchEnd, formattedMergedText.crend());
        // remove the left context from formattedMergedText
        formattedMergedText = formattedMergedText.last(rMatchEnd - formattedMergedText.crbegin());

        int skip = skipRedundantWhiteSpace(leftContext, formattedMergedText, tabWidth);
        formattedMergedText = formattedMergedText.sliced(skip);
    }

    if (!rightContext.isEmpty()) {
        DoubleQuoteFuzzyMatcher fuzzyMatcher;
        fuzzyMatcher.setReverseAddressDirection();
        // Inform the matcher that the end of the right context is not a context-text boundary.
        fuzzyMatcher.firstNonWhitespaceCharacterMatch();
        PrefixMatcher prefixMatcher(rightContext.crbegin(), rightContext.crend(), formattedMergedText.crbegin(),
                                    formattedMergedText.crend(), fuzzyMatcher);
        const auto result = prefixMatcher.match();
        if (!result.hasMatched) {
            return text;
        }

        if (fuzzyMatcher.hasUnmatchedDoubleQuote() != hasUnmatchedDoubleQuote) {
            handleDoubleQuoteMatchFailure();
            return text;
        }

        // include all possible whitespace at the context-text boundary
        auto matchEnd = result.matchEnd.base();
        skipWhitespace(matchEnd, formattedMergedText.cend());
        // remove the right context from formattedMergedText
        formattedMergedText.chop(formattedMergedText.cend() - matchEnd);

        int skip = skipRedundantWhiteSpace(reverse(rightContext), reverse(formattedMergedText), tabWidth);
        formattedMergedText.chop(skip);
    }

    CompleteFuzzyMatcher fuzzyMatcher({&*text.cbegin(), &*text.cend()},
                                      {&*_formattedMergedText.cbegin(), &*_formattedMergedText.cend()});
    if (leftContext.isEmpty()) {
        // Inform the matcher that the beginning of the text is not a context-text boundary.
        fuzzyMatcher.firstNonWhitespaceCharacterMatch();
    }
    if (hasUnmatchedDoubleQuote) {
        // The formatter inserted or removed a double quote into each context. A double quote inserted into
        // or removed from text would be a matching closing or moved across context-text boundaries double quote.
        // Disable matching double quotes to treat two consecutive inserted or removed double quotes as
        // a match failure rather than the usual opening and closing double quotes, which they are not.
        fuzzyMatcher.disableMatchingDoubleQuotes();
    }
    if (!matchFormattedText(text, formattedMergedText, std::move(fuzzyMatcher),
                            /*isEndAContextTextBoundary =*/!rightContext.isEmpty())) {
        return text;
    }

    return formattedMergedText.toString();
}
