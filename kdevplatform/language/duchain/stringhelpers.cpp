/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "stringhelpers.h"
#include "safetycounter.h"
#include <debug.h>

#include <QString>
#include <QStringList>
#include <QVarLengthArray>

namespace {
template <typename T>
int strip_impl(const T& str, T& from)
{
    if (str.isEmpty())
        return 0;

    int i = 0;
    int ip = 0;
    int s = from.length();

    for (int a = 0; a < s; a++) {
        if (QChar::fromLatin1(from[a]).isSpace()) {
            continue;
        } else {
            if (from[a] == str[i]) {
                i++;
                ip = a + 1;
                if (i == str.length())
                    break;
            } else {
                break;
            }
        }
    }

    if (ip) {
        from.remove(0, ip);
    }
    return s - from.length();
}

template <typename T>
int rStrip_impl(const T& str, T& from)
{
    if (str.isEmpty())
        return 0;

    int i = 0;
    int ip = from.length();
    int s = from.length();

    for (int a = s - 1; a >= 0; a--) {
        if (QChar::fromLatin1(from[a])
                .isSpace()) { ///@todo Check whether this can cause problems in utf-8, as only one real character is treated!
            continue;
        } else {
            if (from[a] == str[i]) {
                i++;
                ip = a;
                if (i == str.length())
                    break;
            } else {
                break;
            }
        }
    }

    if (ip != from.length()) {
        from = from.left(ip);
    }
    return s - from.length();
}

bool endsWithWordBoundary(QStringView str)
{
    if (str.isEmpty()) {
        return true;
    }
    const auto boundary = str.last();
    return !boundary.isLetterOrNumber() && boundary != QLatin1Char('_');
}

bool isOperator(const QString& str, int pos)
{
    const auto c = str[pos];
    Q_ASSERT(c == QLatin1Char('<') || c == QLatin1Char('>'));

    --pos;

    // handle `operator<<` and `operator>>`
    if (pos > 0 && str[pos] == c) {
        --pos;
    }

    // skip spaces, e.g. `operator <`
    while (pos > 0 && str[pos].isSpace()) {
        --pos;
    }

    auto prefix = QStringView(str).left(pos + 1);
    const auto op = QLatin1String("operator");
    if (!prefix.endsWith(op)) {
        return false;
    }

    prefix.chop(op.size());
    return endsWithWordBoundary(prefix);
}

// check for operator-> but don't get confused by operator-->
bool isArrowOperator(const QString& str, int pos)
{
    Q_ASSERT(str[pos] == QLatin1Char('>'));
    return pos > 0 && str[pos - 1] == QLatin1Char('-') && (pos == 1 || str[pos - 2] != QLatin1Char('-'));
}

int skipStringOrCharLiteral(const QString& str, int pos)
{
    const auto quote = str[pos];
    Q_ASSERT(quote == QLatin1Char('\'') || quote == QLatin1Char('"'));

    const auto end = str.size();
    pos++;
    while (pos < end && (str[pos] != quote || str[pos - 1] == QLatin1Char('\\'))) {
        pos++;
    }
    return pos;
}
}

namespace KDevelop {
class ParamIteratorPrivate
{
public:
    QString m_prefix;
    QString m_source;
    QString m_parens;
    int m_cur;
    int m_curEnd;
    int m_end;

    int next() const
    {
        return findCommaOrEnd(m_source, m_cur, m_parens[1]);
    }
};

bool parenFits(QChar c1, QChar c2)
{
    if (c1 == QLatin1Char('<') && c2 == QLatin1Char('>'))
        return true;
    else if (c1 == QLatin1Char('(') && c2 == QLatin1Char(')'))
        return true;
    else if (c1 == QLatin1Char('[') && c2 == QLatin1Char(']'))
        return true;
    else if (c1 == QLatin1Char('{') && c2 == QLatin1Char('}'))
        return true;
    else
        return false;
}

int findClose(const QString& str, int pos)
{
    int depth = 1;
    QVarLengthArray<QChar, 16> st;
    st.append(str[pos]);

    for (int a = pos + 1; a < str.length(); a++) {
        switch (str[a].unicode()) {
        case '<':
            if (isOperator(str, a))
                break;
            [[fallthrough]];
        case '(':
        case '[':
        case '{':
            st.insert(0, str[a]);
            depth++;
            break;
        case '>':
            if (isOperator(str, a) || isArrowOperator(str, a))
                break;

            [[fallthrough]];
        case ')':
        case ']':
        case '}':
            if (!st.isEmpty() && parenFits(st.front(), str[a])) {
                depth--;
                st.remove(0);
            }
            break;
        case '"':
        case '\'':
            a = skipStringOrCharLiteral(str, a);
            break;
        }

        if (depth == 0) {
            return a;
        }
    }

    return -1;
}

int findCommaOrEnd(const QString& str, int pos, QChar validEnd)
{
    for (int a = pos; a < str.length(); a++) {
        switch (str[a].unicode()) {
        case '"':
        case '\'':
            a = skipStringOrCharLiteral(str, a);
            break;
        case '<':
            if (isOperator(str, a))
                break;
            [[fallthrough]];
        case '(':
        case '[':
        case '{':
            a = findClose(str, a);
            if (a == -1)
                return str.length();
            break;
        case '>':
            if (isOperator(str, a) || isArrowOperator(str, a))
                break;

            [[fallthrough]];
        case ')':
        case ']':
        case '}':
            if (validEnd != QLatin1Char(' ') && validEnd != str[a])
                break;
            [[fallthrough]];
        case ',':
            return a;
        }
    }

    return str.length();
}

QString reverse(const QString& str)
{
    QString ret;
    int len = str.length();
    ret.reserve(len);
    for (int a = len - 1; a >= 0; --a) {
        switch (str[a].unicode()) {
        case '(':
            ret += QLatin1Char(')');
            continue;
        case '[':
            ret += QLatin1Char(']');
            continue;
        case '{':
            ret += QLatin1Char('}');
            continue;
        case '<':
            ret += QLatin1Char('>');
            continue;
        case ')':
            ret += QLatin1Char('(');
            continue;
        case ']':
            ret += QLatin1Char('[');
            continue;
        case '}':
            ret += QLatin1Char('{');
            continue;
        case '>':
            ret += QLatin1Char('<');
            continue;
        default:
            ret += str[a];
            continue;
        }
    }

    return ret;
}

///@todo this hackery sucks
QString escapeForBracketMatching(QString str)
{
    str.replace(QLatin1String("<<"),   QLatin1String("$&"));
    str.replace(QLatin1String(">>"),   QLatin1String("$$"));
    str.replace(QLatin1String("\\\""), QLatin1String("$!"));
    str.replace(QLatin1String("->"),   QLatin1String("$?"));
    return str;
}

QString escapeFromBracketMatching(QString str)
{
    str.replace(QLatin1String("$&"), QLatin1String("<<"));
    str.replace(QLatin1String("$$"), QLatin1String(">>"));
    str.replace(QLatin1String("$!"), QLatin1String("\\\""));
    str.replace(QLatin1String("$?"), QLatin1String("->"));
    return str;
}

void skipFunctionArguments(const QString& str_, QStringList& skippedArguments, int& argumentsStart)
{
    QString withStrings = escapeForBracketMatching(str_);
    QString str = escapeForBracketMatching(clearStrings(str_));

    //Blank out everything that can confuse the bracket-matching algorithm
    QString reversed = reverse(str.left(argumentsStart));
    QString withStringsReversed = reverse(withStrings.left(argumentsStart));
    //Now we should decrease argumentStart at the end by the count of steps we go right until we find the beginning of the function
    SafetyCounter s(1000);

    int pos = 0;
    int len = reversed.length();
    //we are searching for an opening-brace, but the reversion has also reversed the brace
    while (pos < len && s) {
        int lastPos = pos;
        pos = KDevelop::findCommaOrEnd(reversed, pos);
        if (pos > lastPos) {
            QString arg = reverse(withStringsReversed.mid(lastPos, pos - lastPos)).trimmed();
            if (!arg.isEmpty())
                skippedArguments.push_front(escapeFromBracketMatching(arg)); //We are processing the reversed reverseding, so push to front
        }
        if (reversed[pos] == QLatin1Char(')') || reversed[pos] == QLatin1Char('>'))
            break;
        else
            ++pos;
    }

    if (!s) {
        qCDebug(LANGUAGE) << "skipFunctionArguments: Safety-counter triggered";
    }

    argumentsStart -= pos;
}

QString reduceWhiteSpace(const QString& str_)
{
    const QStringRef str = QStringRef(&str_).trimmed();
    QString ret;
    const int len = str.length();
    ret.reserve(len);

    bool hadSpace = false;
    for (const QChar c : str) {
        if (c.isSpace()) {
            hadSpace = true;
        } else {
            if (hadSpace) {
                hadSpace = false;
                ret += QLatin1Char(' ');
            }
            ret += c;
        }
    }

    ret.squeeze();
    return ret;
}

void fillString(QString& str, int start, int end, QChar replacement)
{
    for (int a = start; a < end; a++)
        str[a] = replacement;
}

QString stripFinalWhitespace(const QString& str)
{
    for (int a = str.length() - 1; a >= 0; --a) {
        if (!str[a].isSpace())
            return str.left(a + 1);
    }

    return QString();
}

QString clearComments(const QString& str_, QChar replacement)
{
    QString str(str_);
    QString withoutStrings = clearStrings(str, QLatin1Char('$'));

    int pos = -1, newlinePos = -1, endCommentPos = -1, nextPos = -1, dest = -1;
    while ((pos = str.indexOf(QLatin1Char('/'), pos + 1)) != -1) {
        newlinePos = withoutStrings.indexOf(QLatin1Char('\n'), pos);

        if (withoutStrings[pos + 1] == QLatin1Char('/')) {
            //C style comment
            dest = newlinePos == -1 ? str.length() : newlinePos;
            fillString(str, pos, dest, replacement);
            pos = dest;
        } else if (withoutStrings[pos + 1] == QLatin1Char('*')) {
            //CPP style comment
            endCommentPos = withoutStrings.indexOf(QLatin1String("*/"), pos + 2);
            if (endCommentPos != -1)
                endCommentPos += 2;

            dest = endCommentPos == -1 ? str.length() : endCommentPos;
            while (pos < dest) {
                nextPos = (dest > newlinePos && newlinePos != -1) ? newlinePos : dest;
                fillString(str, pos, nextPos, replacement);
                pos = nextPos;
                if (pos == newlinePos) {
                    ++pos; //Keep newlines intact, skip them
                    newlinePos = withoutStrings.indexOf(QLatin1Char('\n'), pos + 1);
                }
            }
        }
    }
    return str;
}

QString clearStrings(const QString& str_, QChar replacement)
{
    QString str(str_);
    bool inString = false;
    for (int pos = 0; pos < str.length(); ++pos) {
        //Skip cpp comments
        if (!inString && pos + 1 < str.length() && str[pos] == QLatin1Char('/') && str[pos + 1] == QLatin1Char('*')) {
            pos += 2;
            while (pos + 1 < str.length()) {
                if (str[pos] == QLatin1Char('*') && str[pos + 1] == QLatin1Char('/')) {
                    ++pos;
                    break;
                }
                ++pos;
            }
        }
        //Skip cstyle comments
        if (!inString && pos + 1 < str.length() && str[pos] == QLatin1Char('/') && str[pos + 1] == QLatin1Char('/')) {
            pos += 2;
            while (pos < str.length() && str[pos] != QLatin1Char('\n')) {
                ++pos;
            }
        }
        //Skip a character a la 'b'
        if (!inString && str[pos] == QLatin1Char('\'') && pos + 3 <= str.length()) {
            //skip the opening '
            str[pos] = replacement;
            ++pos;

            if (str[pos] == QLatin1Char('\\')) {
                //Skip an escape character
                str[pos] = replacement;
                ++pos;
            }

            //Skip the actual character
            str[pos] = replacement;
            ++pos;

            //Skip the closing '
            if (pos < str.length() && str[pos] == QLatin1Char('\'')) {
                str[pos] = replacement;
            }

            continue;
        }

        bool intoString = false;
        if (str[pos] == QLatin1Char('"') && !inString)
            intoString = true;

        if (inString || intoString) {
            if (inString) {
                if (str[pos] == QLatin1Char('"'))
                    inString = false;
            } else {
                inString = true;
            }

            bool skip = false;
            if (str[pos] == QLatin1Char('\\'))
                skip = true;

            str[pos] = replacement;
            if (skip) {
                ++pos;
                if (pos < str.length())
                    str[pos] = replacement;
            }
        }
    }

    return str;
}

int strip(const QByteArray& str, QByteArray& from)
{
    return strip_impl<QByteArray>(str, from);
}

int rStrip(const QByteArray& str, QByteArray& from)
{
    return rStrip_impl<QByteArray>(str, from);
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
    for (const auto& line : qAsConst(lines)) {
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

    auto lines = comment.splitRef(QLatin1Char('\n'));

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
                l = l.mid(m.length());
                break;
            }
        }
    }

    QString ret;
    for (const auto& line : qAsConst(lines)) {
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

ParamIterator::ParamIterator(const QString& parens, const QString& source, int offset)
    : d_ptr(new ParamIteratorPrivate)
{
    Q_D(ParamIterator);

    d->m_source = source;
    d->m_parens = parens;

    d->m_cur = offset;
    d->m_curEnd = offset;
    d->m_end = d->m_source.length();

    ///The whole search should be stopped when: A) The end-sign is found on the top-level B) A closing-brace of parameters was found
    int parenBegin = d->m_source.indexOf(parens[0], offset);

    //Search for an interrupting end-sign that comes before the found paren-begin
    int foundEnd = -1;
    if (parens.length() > 2) {
        foundEnd = d->m_source.indexOf(parens[2], offset);
        if (foundEnd > parenBegin && parenBegin != -1)
            foundEnd = -1;
    }

    if (foundEnd != -1) {
        //We have to stop the search, because we found an interrupting end-sign before the opening-paren
        d->m_prefix = d->m_source.mid(offset, foundEnd - offset);

        d->m_curEnd = d->m_end = d->m_cur = foundEnd;
    } else {
        if (parenBegin != -1) {
            //We have a valid prefix before an opening-paren. Take the prefix, and start iterating parameters.
            d->m_prefix = d->m_source.mid(offset, parenBegin - offset);
            d->m_cur = parenBegin + 1;
            d->m_curEnd = d->next();
            if (d->m_curEnd == d->m_source.length()) {
                //The paren was not closed. It might be an identifier like "operator<", so count everything as prefix.
                d->m_prefix = d->m_source.mid(offset);
                d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
            }
        } else {
            //We have neither found an ending-character, nor an opening-paren, so take the whole input and end
            d->m_prefix = d->m_source.mid(offset);
            d->m_curEnd = d->m_end = d->m_cur = d->m_source.length();
        }
    }
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

QString ParamIterator::operator *()
{
    Q_D(ParamIterator);

    Q_ASSERT(*this);

    return d->m_source.mid(d->m_cur, d->m_curEnd - d->m_cur).trimmed();
}

ParamIterator::operator bool() const
{
    Q_D(const ParamIterator);

    return d->m_cur < d->m_end;
}

QString ParamIterator::prefix() const
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
