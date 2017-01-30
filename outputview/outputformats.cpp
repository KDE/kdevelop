/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>               *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    return columnGroup < 0 ? 0 : std::max(match.captured(columnGroup).toInt() - 1, 0);
}

}

