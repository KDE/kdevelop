/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeitem.h"

#include <qstylesheet.h>

#include <klocale.h>

MakeItem::MakeItem()
{
}

MakeItem::MakeItem( const QString& text )
	: m_text( text )
{
}

MakeItem::~MakeItem()
{
}

QString MakeItem::color( bool bright_bg )
{
	switch ( type() )
	{
	case Error:
		return bright_bg ? "maroon" : "red";
	case Diagnostic:
		return bright_bg ? "black" : "white";
	default:
		return bright_bg ? "navy" : "blue";
	}
}

QString MakeItem::icon()
{
	switch ( type() )
	{
	case Error:
		return "<img src=\"error\"></img><nobr> </nobr>";
	case Diagnostic:
		return "<img src=\"warning\"></img><nobr> </nobr>";
	default:
		return "<img src=\"message\"></img><nobr> </nobr>";
	}
}

QString MakeItem::text( EOutputLevel )
{
	return QStyleSheet::escape( m_text );
}

QString MakeItem::formattedText( EOutputLevel level, bool bright_bg )
{
    return QString("<code>").append( icon() ).append("<font color=\"").append( color( bright_bg) ).append(
		"\">").append( text(level) ).append("</font></code>").append( br() );
}

QString MakeItem::br()
{
    // Qt >= 3.1 doesn't need a <br>.
#if QT_VERSION < 0x040000
    static const QString br = QString::fromLatin1( qVersion() ).section( ".", 1, 1 ).toInt() > 0 ? "" : "<br>";
#else
    static const QString br;
#endif
    return br;
}

ErrorItem::ErrorItem( const QString& fn, int ln, const QString& tx, const QString& line )
	: MakeItem( line )
	, fileName(fn)
	, lineNum(ln)
	, m_error(tx)
{}

bool ErrorItem::append( const QString& text )
{
	if ( !text.startsWith("   ") )
		return false;
	m_text += text;
	m_error += text;
	m_error = m_error.simplifyWhiteSpace();
	m_text = m_text.simplifyWhiteSpace();
	return true;
}

ExitStatusItem::ExitStatusItem( bool normalExit, int exitStatus )
	: m_normalExit( normalExit )
	, m_exitStatus( exitStatus )
{
}

QString ExitStatusItem::text( EOutputLevel )
{
	if ( m_normalExit && m_exitStatus )
		return i18n("*** Exited with status: %1 ***").arg( m_exitStatus );
	if ( m_normalExit )
		return i18n("*** Success ***");
	return i18n("*** Compilation aborted ***");
}

bool DirectoryItem::m_showDirectoryMessages = true;

QString EnteringDirectoryItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
		return i18n("Entering directory %1").arg( directory );
	return m_text;
}

QString ExitingDirectoryItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
		return i18n("Leaving directory %1").arg( directory );
	return m_text;
}

QString ActionItem::text( EOutputLevel outputLevel )
{
	if ( outputLevel < eFull )
	{
		if ( m_tool.isEmpty() )
			return QString(m_action).append("<b>").append(m_file).append("</b>");
		return QString(m_action).append("<b>").append(m_file).append("</b>").append(" (").append(m_tool).append(")");
	}
	return MakeItem::text( outputLevel );
}
