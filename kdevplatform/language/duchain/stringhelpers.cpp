/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "stringhelpers.h"
#include <debug.h>

#include <QString>

#include <algorithm>
#include <utility>

namespace {
bool endsWithWordBoundary(QStringView str)
{
    if (str.isEmpty()) {
        return true;
    }
    const auto boundary = str.last();
    return !boundary.isLetterOrNumber() && boundary != QLatin1Char('_');
}

/// libclang surrounds binary operators but not angle brackets with spaces.
bool isOperatorSurroundedWithSpaces(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());

    if (pos == 0 || pos == str.size() - 1) {
        return false; // there is no place for surrounding spaces
    }

    constexpr QLatin1Char lt{'<'}, gt{'>'}, eq{'='}, space{' '};

    const auto c = str[pos];
    Q_ASSERT(c == lt || c == gt);

    // Note: due to the `pos == 0 || pos == str.size() - 1` check above,
    // most conditionals below don't need to check boundaries.
    int operatorEnd = pos + 1;
    if (str[pos + 1] == c)
        ++operatorEnd; // << or >>
    else if (str[pos - 1] == c) {
        --pos; // << or >>
    } else {
        // <=>
        if (c == lt && str[pos + 1] == eq && pos + 2 < str.size() && str[pos + 2] == gt) {
            operatorEnd += 2;
        } else if (c == gt && str[pos - 1] == eq && pos >= 2 && str[pos - 2] == lt) {
            pos -= 2;
        }
    }

    if (operatorEnd - pos < 3 && operatorEnd < str.size() && str[operatorEnd] == eq) {
        ++operatorEnd; // <= or >= or <<= or >>=
    }

    return pos > 0 && str[pos - 1] == space && operatorEnd < str.size() && str[operatorEnd] == space;
}

bool isOperator(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());

    if (isOperatorSurroundedWithSpaces(str, pos)) {
        return true;
    }

    const auto op = QLatin1String("operator");
    if (pos < op.size()) {
        return false;
    }

    const auto c = str[pos];
    Q_ASSERT(c == QLatin1Char('<') || c == QLatin1Char('>'));

    --pos;

    // note: due to the `pos < op.size()` check above, the below conditionals don't need to check boundaries
    if (str[pos] == c) {
        // handle `operator<<` and `operator>>`
        --pos;
    } else if (c == QLatin1Char('>') && str[pos] == QLatin1Char('=') && str[pos - 1] == QLatin1Char('<')) {
        // handle `operator<=>`
        pos -= 2;
    }

    // skip spaces, e.g. `operator <`
    while (pos > 0 && str[pos].isSpace()) {
        --pos;
    }

    auto prefix = str.first(pos + 1);
    if (!prefix.endsWith(op)) {
        return false;
    }

    prefix.chop(op.size());
    return endsWithWordBoundary(prefix);
}

// check for operator-> but don't get confused by operator-->
bool isArrowOperator(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());

    Q_ASSERT(str[pos] == QLatin1Char('>'));
    return pos > 0 && str[pos - 1] == QLatin1Char('-') && (pos == 1 || str[pos - 2] != QLatin1Char('-'));
}

bool isOperatorOrArrowOperator(QStringView str, int pos)
{
    return isOperator(str, pos) || isArrowOperator(str, pos);
}

/// Skips literals enclosed in single or double quotes.
/// No need to support raw string literals, because they cannot appear within a macro parameter list;
/// in other contexts libclang converts them into non-raw string literals in each string that ends up here.
int skipStringOrCharLiteral(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());

    const auto quote = str[pos];
    Q_ASSERT(quote == QLatin1Char('\'') || quote == QLatin1Char('"'));

    const auto end = str.size();
    pos++;
    while (pos < end && (str[pos] != quote || str[pos - 1] == QLatin1Char('\\'))) {
        pos++;
    }
    return pos;
}

/// Skips multi-line comments.
/// No need to support single-line comments, because they cannot appear within a macro parameter list;
/// in other contexts libclang removes comments from each string that ends up here.
int skipComment(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());
    Q_ASSERT(str[pos] == QLatin1Char{'/'});

    if (pos + 1 == str.size() || str[pos + 1] != QLatin1Char{'*'})
        return pos; // not a comment
    pos += 2;

    while (pos < str.size() && (str[pos] != QLatin1Char{'/'} || str[pos - 1] != QLatin1Char{'*'})) {
        ++pos;
    }

    return pos;
}

int trySkipStringOrCharLiteralOrComment(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());

    switch (str[pos].unicode()) {
    case '"':
    case '\'':
        return skipStringOrCharLiteral(str, pos);
    case '/':
        return skipComment(str, pos);
    }
    return pos;
}
} // unnamed namespace

namespace KDevelop {
bool consistsOfWhitespace(QStringView str)
{
    return std::all_of(str.cbegin(), str.cend(), [](QChar c) {
        return c.isSpace();
    });
}

class ParamIteratorPrivate
{
    Q_DISABLE_COPY_MOVE(ParamIteratorPrivate)
public:
    explicit ParamIteratorPrivate(QStringView parens, QStringView source)
        : m_parens(parens)
        , m_source(source)
    {
    }

    const QStringView m_parens;
    const QStringView m_source;
    QStringView m_prefix;
    int m_cur;
    int m_curEnd;
    int m_end;

    QStringView sourceRange(int first, int last) const
    {
        return m_source.sliced(first, last - first);
    }

    int next() const
    {
        return findCommaOrEnd(m_source, m_cur, m_parens[1]);
    }
};

namespace {
QChar fittingClosingNonAngleBracket(QChar openingBracket)
{
    switch (openingBracket.unicode()) {
    case '(':
        return QLatin1Char(')');
    case '[':
        return QLatin1Char(']');
    case '{':
        return QLatin1Char('}');
    default:
        Q_UNREACHABLE();
    }
}

// findClosingNonAngleBracket() and findClosingAngleBracket() have different implementations for the following reason.
// Taking all bracket types into account while looking for a closing angle bracket may improve correctness, because the
// characters of other bracket types are always brackets, not [parts of] operators; distinguishing between angle
// brackets and operators is heuristic and unreliable. For example, in `Foo<(A>B)>` the round brackets help to recognize
// the first '>' character as an operator rather than a closing angle bracket. Conversely, taking all bracket types into
// account while looking for a closing non-angle bracket may adversely affect correctness. For example, in `Foo<(A<B)>`
// the second '<' character would be regarded as an opening angle bracket, which would prevent recognizing the closing
// round bracket.

/// Finds in @p str the position of a fitting closing bracket for the opening bracket @p str[@p pos], e.g. ')' for '('.
/// @return the position of a fitting closing bracket or str.size() if not found.
/// @warning This function does not support angle brackets. Use findClosingAngleBracket() for that.
int findClosingNonAngleBracket(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());
    Q_ASSERT(str[pos] == QLatin1Char{'('} || str[pos] == QLatin1Char{'['} || str[pos] == QLatin1Char{'{'});

    const auto openingBracket = str[pos];
    const auto closingBracket = fittingClosingNonAngleBracket(openingBracket);

    int depth = 1;

    for (++pos; pos < str.size(); ++pos) {
        if (str[pos] == openingBracket) {
            ++depth;
        } else if (str[pos] == closingBracket) {
            if (--depth == 0) {
                return pos;
            }
        } else {
            pos = trySkipStringOrCharLiteralOrComment(str, pos);
        }
    }

    Q_ASSERT(depth > 0);
    return str.size();
}

/// Finds in @p str the position of a fitting closing angle bracket for the opening angle bracket @p str[@p pos] == '<'.
/// @return the position of a fitting closing bracket or str.size() if not found.
int findClosingAngleBracket(QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos < str.size());
    Q_ASSERT(str[pos] == QLatin1Char{'<'});

    int depth = 1;

    for (++pos; pos < str.size(); ++pos) {
        switch (str[pos].unicode()) {
        case '<':
            if (!isOperator(str, pos)) {
                ++depth;
            }
            break;
        case '>':
            if (!isOperatorOrArrowOperator(str, pos)) {
                if (--depth == 0) {
                    return pos;
                }
            }
            break;
        case '(':
        case '[':
        case '{':
            pos = findClosingNonAngleBracket(str, pos);
            break;
        default:
            pos = trySkipStringOrCharLiteralOrComment(str, pos);
        }
    }

    Q_ASSERT(depth > 0);
    return str.size();
}

/// Finds in @p str the position of @p parens[0] or @p parens[2] starting from @p pos at the top level.
/// @return the position of the found symbol or str.size() if not found.
/// @param parens see ParamIterator().
int findOpeningBracketOrEnd(QStringView parens, QStringView str, int pos)
{
    Q_ASSERT(pos >= 0 && pos <= str.size());

    Q_ASSERT(parens.size() == 2 || parens.size() == 3);

    Q_ASSERT(QStringView(u"<([{").contains(parens[0]));
    Q_ASSERT(parens.first(2) == u"<>" || parens[1] == fittingClosingNonAngleBracket(parens[0]));

    Q_ASSERT(parens.size() == 2 || !QStringView(u"<>()[]{}").contains(parens[2]));

    for (; pos < str.size(); ++pos) {
        switch (str[pos].unicode()) {
        // Take into account brackets of all types to skip searched-for symbols within them (i.e. not at the top level).
        case '<':
            if (!isOperator(str, pos)) {
                if (str[pos] == parens[0]) {
                    return pos;
                }
                pos = findClosingAngleBracket(str, pos);
            }
            break;
        case '(':
        case '[':
        case '{':
            if (str[pos] == parens[0]) {
                return pos;
            }
            pos = findClosingNonAngleBracket(str, pos);
            break;
        default:
            if (parens.size() > 2 && str[pos] == parens[2]) {
                return pos;
            }
            pos = trySkipStringOrCharLiteralOrComment(str, pos);
        }
    }

    return str.size();
}
} // unnamed namespace

int findCommaOrEnd(QStringView str, int pos, QChar validEnd)
{
    const auto size = str.size();
    Q_ASSERT(pos >= 0 && pos <= size);

    for (; pos < size; ++pos) {
        switch (str[pos].unicode()) {
        // Take into account brackets of all types, not just the validEnd type, to skip ',' within them.
        case '<':
            if (!isOperator(str, pos)) {
                pos = findClosingAngleBracket(str, pos);
            }
            break;
        case '(':
        case '[':
        case '{':
            pos = findClosingNonAngleBracket(str, pos);
            break;
        case ',':
            return pos;
        default:
            if (str[pos] == validEnd && !(str[pos] == QLatin1Char('>') && isOperatorOrArrowOperator(str, pos))) {
                return pos;
            }
            pos = trySkipStringOrCharLiteralOrComment(str, pos);
        }
    }

    return size;
}

// NOTE: keep in sync with QString overload below
QByteArray formatComment(const QByteArray& comment)
{
    if (comment.isEmpty())
        return comment;

    auto lines = comment.split('\n');
    // remove common leading & trailing chars from the lines
    for (auto& l : lines) {
        // don't trigger repeated temporary allocations here

        // possible comment starts, sorted from longest to shortest
        static const QByteArray startMatches[] = {
            QByteArrayLiteral("//!<"), QByteArrayLiteral("/*!<"), QByteArrayLiteral("/**<"), QByteArrayLiteral("///<"),
            QByteArrayLiteral("///"),  QByteArrayLiteral("//!"),  QByteArrayLiteral("/**"),  QByteArrayLiteral("/*!"),
            QByteArrayLiteral("//"),   QByteArrayLiteral("/*"),   QByteArrayLiteral("/"),    QByteArrayLiteral("*")};

        // possible comment ends, sorted from longest to shortest
        static const QByteArray endMatches[] = {QByteArrayLiteral("**/"), QByteArrayLiteral("*/")};

        l = l.trimmed();

        // check for ends first, as the starting pattern "*" might interfere with the ending pattern
        for (const auto& m : endMatches) {
            if (l.endsWith(m)) {
                l.chop(m.length());
                break;
            }
        }

        for (const auto& m : startMatches) {
            if (l.startsWith(m)) {
                l.remove(0, m.length());
                break;
            }
        }
    }

    QByteArray ret;
    for (const auto& line : std::as_const(lines)) {
        if (!ret.isEmpty())
            ret += '\n';
        ret += line;
    }
    return ret.trimmed();
}

// NOTE: keep in sync with QByteArray overload above
QString formatComment(const QString& comment)
{
    if (comment.isEmpty())
        return comment;

    auto lines = QStringView{comment}.split(QLatin1Char('\n'));

    // remove common leading & trailing chars from the lines
    for (auto& l : lines) {
        // don't trigger repeated temporary allocations here

        // possible comment starts, sorted from longest to shortest
        static const QString startMatches[] = {QStringLiteral("//!<"), QStringLiteral("/*!<"), QStringLiteral("/**<"),
                                               QStringLiteral("///<"), QStringLiteral("///"),  QStringLiteral("//!"),
                                               QStringLiteral("/**"),  QStringLiteral("/*!"),  QStringLiteral("//"),
                                               QStringLiteral("/*"),   QStringLiteral("/"),    QStringLiteral("*")};

        // possible comment ends, sorted from longest to shortest
        static const QString endMatches[] = {QStringLiteral("**/"), QStringLiteral("*/")};

        l = l.trimmed();

        // check for ends first, as the starting pattern "*" might interfere with the ending pattern
        for (const auto& m : endMatches) {
            if (l.endsWith(m)) {
                l.chop(m.length());
                break;
            }
        }

        for (const auto& m : startMatches) {
            if (l.startsWith(m)) {
                l = l.sliced(m.length());
                break;
            }
        }
    }

    QString ret;
    for (const auto line : std::as_const(lines)) {
        if (!ret.isEmpty())
            ret += QLatin1Char('\n');
        ret += line;
    }

    return ret.trimmed();
}

QString removeWhitespace(const QString& str)
{
    return str.simplified().remove(QLatin1Char(' '));
}

ParamIterator::~ParamIterator() = default;

ParamIterator::ParamIterator(QStringView parens, QStringView source, int offset)
    : d_ptr(new ParamIteratorPrivate{parens, source})
{
    Q_D(ParamIterator);

    const auto foundPos = findOpeningBracketOrEnd(parens, source, offset);
    if (foundPos != source.size()) {
        if (parens.size() > 2 && source[foundPos] == parens[2]) {
            //We have to stop the search, because we found an interrupting end-sign before the opening-paren
            d->m_prefix = d->sourceRange(offset, foundPos);
            d->m_curEnd = d->m_end = d->m_cur = foundPos;
            return;
        }

        Q_ASSERT(source[foundPos] == parens[0]);
        //We have a valid prefix before an opening-paren. Take the prefix, and start iterating parameters.
        d->m_cur = foundPos + 1;
        d->m_curEnd = d->next();
        if (d->m_curEnd != d->m_source.length()) {
            d->m_prefix = d->sourceRange(offset, foundPos);
            d->m_end = d->m_source.size();

            if (d->m_source[d->m_curEnd] == d->m_parens[1]) {
                const auto singleParam = d->sourceRange(d->m_cur, d->m_curEnd);
                if (consistsOfWhitespace(singleParam)) {
                    // Only whitespace characters are present between parentheses => assume that
                    // there are zero parameters, not a single empty parameter, and stop iterating.
                    d->m_cur = d->m_end = d->m_curEnd + 1;
                }
            }

            return;
        } // else: the paren was not closed. It might be an identifier like "operator<", so count everything as prefix.
    } // else: we have neither found an ending-character, nor an opening-paren, so take the whole input and end.

    d->m_prefix = d->m_source.sliced(offset);
    d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
}

ParamIterator& ParamIterator::operator ++()
{
    Q_D(ParamIterator);

    Q_ASSERT(*this);

    if (d->m_curEnd >= d->m_source.size()) {
        //We have reached the end-paren. Stop iterating.
        d->m_cur = d->m_end = d->m_curEnd;
    } else if (d->m_source[d->m_curEnd] == d->m_parens[1]) {
        //We have reached the end-paren. Stop iterating.
        d->m_cur = d->m_end = d->m_curEnd + 1;
    } else {
        //Iterate on through parameters
        d->m_cur = d->m_curEnd + 1;
        if (d->m_cur < d->m_source.length()) {
            d->m_curEnd = d->next();
        }
    }
    return *this;
}

QStringView ParamIterator::operator*() const
{
    Q_D(const ParamIterator);

    Q_ASSERT(*this);

    return d->sourceRange(d->m_cur, d->m_curEnd).trimmed();
}

ParamIterator::operator bool() const
{
    Q_D(const ParamIterator);

    return d->m_cur < d->m_end;
}

QStringView ParamIterator::prefix() const
{
    Q_D(const ParamIterator);

    return d->m_prefix;
}

uint ParamIterator::position() const
{
    Q_D(const ParamIterator);

    return ( uint )d->m_cur;
}
}
