/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "outputformats.h"

#include <KLocalizedString>

namespace KDevelop
{

ErrorFormat::ErrorFormat( const QString& regExp, int file, int line, int text, int column )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , columnGroup( column )
    , textGroup( text )
{}

ErrorFormat::ErrorFormat( const QString& regExp, int file, int line, int text, const QString& comp, int column )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , columnGroup( column )
    , textGroup( text )
    , compiler( comp )
{}

ActionFormat::ActionFormat(const QString& _tool, const QString& regExp, int file )
    : expression( regExp )
    , tool( _tool )
    , fileGroup( file )
{
}

ActionFormat::ActionFormat(int file, const QString& regExp)
    : expression( regExp )
    , fileGroup( file )
{
}

int ErrorFormat::columnNumber(const QRegularExpressionMatch& match) const
{
    return columnGroup < 0 ? 0 : std::max(match.capturedView(columnGroup).toInt() - 1, 0);
}

}

