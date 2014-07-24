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
#include <QChar>
#include <QDataStream>
#include <QVariant>
#include <QRegExp>
#include <QTextDocument>

namespace KDevelop
{
    QString joinWithEscaping( const QStringList& input, const QChar& joinchar, const QChar& escapechar )
    {
        QStringList tmp = input;
        return tmp.replaceInStrings( joinchar, QString( joinchar ) + QString( escapechar ) ).join( joinchar );
    }
    
    QStringList splitWithEscaping( const QString& input, const QChar& splitchar, const QChar& escapechar )
    {
        enum State { Normal, SeenEscape } state;

        state = Normal;

        QStringList result;
        QString currentstring;
        for( int i = 0; i < input.size(); i++ ) {
            switch( state ) {
                case Normal:
                    if( input[i] == escapechar ) {
                        state = SeenEscape;
                    } else if( input[i] == splitchar ) {
                        result << currentstring;
                        currentstring = "";
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
        if( !currentstring.isEmpty() ) {
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
        
        QString result = QLatin1String("@Variant(");
        result += QString::fromLatin1(a.constData(), a.size());
        result += QLatin1Char(')');
        return result;
        
    }

    QString htmlToPlainText(const QString& s, HtmlToPlainTextMode mode)
    {
        switch (mode) {
        case FastMode: {
            QString result(s);
            result.remove(QRegExp("<[^>]+>"));
            return result;
        }
        case CompleteMode: {
            QTextDocument doc;
            doc.setHtml(s);
            return doc.toPlainText();
        }
        }
        return QString(); // never reached
    }
}

