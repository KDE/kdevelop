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
*/

#include "kdevstringhandler.h"

#include <QStringList>
#include <QString>
#include <QChar>

namespace KDevStringHandler
{
    QStringList splitWithEscaping( const QString& input, const QChar& split, const QChar& escapechar )
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
                    } else if( input[i] == split ) {
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
}

