/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later

    This file mostly code takes from Qt's QSettings class, the copyright
    header from that file follows:

    SPDX-FileCopyrightText: 2009 Nokia Corporation and /or its subsidiary(-ies). <qt-info@nokia.com>

    This file is part of the QtCore module of the Qt Toolkit.
    SPDX-License-Identifier: LGPL-2.1-only WITH LicenseRef-Qt-LGPL-exception-1.0 OR GPL-3.0-only OR LicenseRef-Qt-Commercial
*/

#include "kdevstringhandler.h"

#include <QByteArray>
#include <QChar>
#include <QDataStream>
#include <QIODevice>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QTextDocument>
#include <QVariant>

#include <algorithm>
#include <cctype>

namespace KDevelop {
QString joinWithEscaping(const QStringList& input, QChar joinchar, QChar escapechar)
{
    QStringList tmp = input;
    return tmp.replaceInStrings(joinchar, QString(joinchar) + QString(escapechar)).join(joinchar);
}

QStringList splitWithEscaping(const QString& input, QChar splitchar, QChar escapechar)
{
    enum State { Normal, SeenEscape } state;

    state = Normal;

    QStringList result;
    QString currentstring;
    for (const QChar c : input) {
        switch (state) {
        case Normal:
            if (c == escapechar) {
                state = SeenEscape;
            } else if (c == splitchar) {
                result << currentstring;
                currentstring.clear();
            } else {
                currentstring += c;
            }
            break;
        case SeenEscape:
            currentstring += c;
            state = Normal;
            break;
        }
    }

    if (!currentstring.isEmpty()) {
        result << currentstring;
    }
    return result;
}

QVariant stringToQVariant(const QString& s)
{
    // Taken from qsettings.cpp, stringToVariant()
    if (s.startsWith(QLatin1Char('@'))) {
        if (s.endsWith(QLatin1Char(')'))) {
            if (s.startsWith(QLatin1String("@Variant("))) {
                QByteArray a(s.toLatin1().mid(9));
                QDataStream stream(&a, QIODevice::ReadOnly);
                stream.setVersion(QDataStream::Qt_4_4);
                QVariant result;
                stream >> result;
                return result;
            }
        }
    }
    return QVariant();

}

QString qvariantToString(const QVariant& variant)
{
    // Taken from qsettings.cpp, variantToString()
    QByteArray a;
    {
        QDataStream s(&a, QIODevice::WriteOnly);
        s.setVersion(QDataStream::Qt_4_4);
        s << variant;
    }

    QString result = QLatin1String("@Variant(") + QString::fromLatin1(a.constData(), a.size()) + QLatin1Char(')');
    return result;

}

QString htmlToPlainText(const QString& s, HtmlToPlainTextMode mode)
{
    switch (mode) {
    case FastMode: {
        static const QRegularExpression htmlTagRegex(QStringLiteral("<[^>]+>"));
        QString result(s);
        result.remove(htmlTagRegex);
        return result;
    }
    case CompleteMode: {
        QTextDocument doc;
        doc.setHtml(s);
        return doc.toPlainText();
    }
    }
    return QString();     // never reached
}

QByteArray escapeJavaScriptString(const QByteArray& str)
{
    // The special symbols that have to be escaped are listed e.g. here:
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String#escape_sequences
    QByteArray result;
    result.reserve(str.size());

    for (char ch : str) {
        switch (ch) {
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\v':
            result += "\\v";
            break;
        case '\0':
            result += "\\0";
            break;
        case '\'':
        case '"':
        case '\\':
            result += '\\';
            [[fallthrough]];
        default:
            result += ch;
        }
    }

    return result;
}
}

int KDevelop::findAsciiIdentifierLength(QStringView str)
{
    if (str.isEmpty()) {
        return 0;
    }

    constexpr ushort maxAscii{127};
    const auto firstChar = str[0].unicode();
    const bool isIdentifier = firstChar <= maxAscii
                                && (std::isalpha(firstChar) || firstChar == '_');
    if (!isIdentifier) {
        return 0;
    }

    const auto partOfIdentifier = [=](QChar character) {
        const auto u = character.unicode();
        return u <= maxAscii && (std::isalnum(u) || u == '_');
    };
    return std::find_if_not(str.cbegin() + 1, str.cend(), partOfIdentifier) - str.cbegin();
}

KDevelop::VariableMatch KDevelop::matchPossiblyBracedAsciiVariable(QStringView str)
{
    if (str.isEmpty()) {
        return {};
    }

    if (str[0].unicode() == '{') {
        const auto nameLength = findAsciiIdentifierLength(str.sliced(1));
        if (nameLength == 0) {
            return {};
        }
        const auto closingBraceIndex = 1 + nameLength;
        if (closingBraceIndex < str.size() && str[closingBraceIndex].unicode() == '}') {
            return {nameLength + 2, str.sliced(1, nameLength).toString()};
        }
    } else {
        const auto nameLength = findAsciiIdentifierLength(str);
        if (nameLength != 0) {
            return {nameLength, str.first(nameLength).toString()};
        }
    }

    return {};
}

QString KDevelop::stripAnsiSequences(const QString& str)
{
    if (str.isEmpty()) {
        return QString(); // fast path
    }

    enum {
        PLAIN,
        ANSI_START,
        ANSI_CSI,
        ANSI_SEQUENCE,
        ANSI_WAITING_FOR_ST,
        ANSI_ST_STARTED
    } state = PLAIN;

    QString result;
    result.reserve(str.size());

    for (const QChar c : str) {
        const auto val = c.unicode();
        switch (state) {
        case PLAIN:
            if (val == 27) // 'ESC'
                state = ANSI_START;
            else if (val == 155) // equivalent to 'ESC'-'['
                state = ANSI_CSI;
            else
                result.append(c);
            break;
        case ANSI_START:
            if (val == 91) // [
                state = ANSI_CSI;
            else if (val == 80 || val == 93 || val == 94 || val == 95) // 'P', ']', '^' and '_'
                state = ANSI_WAITING_FOR_ST;
            else if (val >= 64 && val <= 95)
                state = PLAIN;
            else
                state = ANSI_SEQUENCE;
            break;
        case ANSI_CSI:
            if (val >= 64 && val <= 126) // Anything between '@' and '~'
                state = PLAIN;
            break;
        case ANSI_SEQUENCE:
            if (val >= 64 && val <= 95) // Anything between '@' and '_'
                state = PLAIN;
            break;
        case ANSI_WAITING_FOR_ST:
            if (val == 7) // 'BEL'
                state = PLAIN;
            else if (val == 27) // 'ESC'
                state = ANSI_ST_STARTED;
            break;
        case ANSI_ST_STARTED:
            if (val == 92) // '\'
                state = PLAIN;
            else
                state = ANSI_WAITING_FOR_ST;
            break;
        }
    }

    return result;
}

void KDevelop::normalizeLineEndings(QByteArray& text)
{
    for (int i = 0, s = text.size(); i < s; ++i) {
        if (text[i] != '\r') {
            continue;
        }
        if (i + 1 < s && text[i + 1] == '\n') {
            text.remove(i, 1);
            s -= 1;
        } else {
            text[i] = '\n';
        }
    }
}
