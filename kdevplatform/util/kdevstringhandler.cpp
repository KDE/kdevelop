/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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

    This file mostly code takes from Qt's QSettings class, the copyright
    header from that file follows:

    ****************************************************************************
    **
    ** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
    ** Contact: Nokia Corporation (qt-info@nokia.com)
    **
    ** This file is part of the QtCore module of the Qt Toolkit.
    **
    ** $QT_BEGIN_LICENSE:LGPL$
    ** Commercial Usage
    ** Licensees holding valid Qt Commercial licenses may use this file in
    ** accordance with the Qt Commercial License Agreement provided with the
    ** Software or, alternatively, in accordance with the terms contained in
    ** a written agreement between you and Nokia.
    **
    ** GNU Lesser General Public License Usage
    ** Alternatively, this file may be used under the terms of the GNU Lesser
    ** General Public License version 2.1 as published by the Free Software
    ** Foundation and appearing in the file LICENSE.LGPL included in the
    ** packaging of this file.  Please review the following information to
    ** ensure the GNU Lesser General Public License version 2.1 requirements
    ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
    **
    ** In addition, as a special exception, Nokia gives you certain
    ** additional rights. These rights are described in the Nokia Qt LGPL
    ** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
    ** package.
    **
    ** GNU General Public License Usage
    ** Alternatively, this file may be used under the terms of the GNU
    ** General Public License version 3.0 as published by the Free Software
    ** Foundation and appearing in the file LICENSE.GPL included in the
    ** packaging of this file.  Please review the following information to
    ** ensure the GNU General Public License version 3.0 requirements will be
    ** met: http://www.gnu.org/copyleft/gpl.html.
    **
    ** If you are unsure which license is appropriate for your use, please
    ** contact the sales department at http://www.qtsoftware.com/contact.
    ** $QT_END_LICENSE$
    **
    ****************************************************************************
*/

#include "kdevstringhandler.h"

#include <QStringList>
#include <QString>
#include <QByteArray>
#include <QChar>
#include <QDataStream>
#include <QVariant>
#include <QRegExp>
#include <QTextDocument>

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
    for (int i = 0; i < input.size(); i++) {
        switch (state) {
        case Normal:
            if (input[i] == escapechar) {
                state = SeenEscape;
            } else if (input[i] == splitchar) {
                result << currentstring;
                currentstring.clear();
            } else {
                currentstring += input[i];
            }
            break;
        case SeenEscape:
            currentstring += input[i];
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

    QString result = QStringLiteral("@Variant(") + QString::fromLatin1(a.constData(), a.size()) + QLatin1Char(')');
    return result;

}

QString htmlToPlainText(const QString& s, HtmlToPlainTextMode mode)
{
    switch (mode) {
    case FastMode: {
        QString result(s);
        result.remove(QRegExp(QStringLiteral("<[^>]+>")));
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
    result.reserve(str.count());

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
        } else {
            text[i] = '\n';
        }
    }
}
