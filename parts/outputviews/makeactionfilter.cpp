/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "makeactionfilter.h"
#include "makeactionfilter.moc"
#include "makeitem.h"

#include <klocale.h>

MakeActionFilter::ActionFormat::ActionFormat( const QString& _action, const QString& _tool, const char * regExp, int file )
	: action( _action )
	, tool( _tool )
	, expression( regExp )
	, fileGroup( file )
{
}

MakeActionFilter::MakeActionFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

const QValueList<MakeActionFilter::ActionFormat>& MakeActionFilter::actionFormats()
{
	static QValueList<ActionFormat> formats
		= QValueList<ActionFormat>()

	<< ActionFormat( i18n("compiling"), "g++", "(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).*`.*`(?:[^/\\s;]*/)+([^/\\s;]+)", 1 )
	<< ActionFormat( i18n("compiling"), "g++", "(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).* -c ([^\\s;]+)", 1 )
	<< ActionFormat( i18n("compiling"), "g++", "(?:g\\+\\+|/bin/sh\\s.*libtool.*--mode=compile).* -c -.*", 1 )
	<< ActionFormat( i18n("generating"), "moc", ".*/moc\\b.*\\s-o\\s([^\\s;]+)", 1 )
	<< ActionFormat( i18n("linking"), "libtool", "/bin/sh\\s.*libtool.*--mode=link .* -o ([^\\s;]+)", 1 )
	<< ActionFormat( i18n("linking"), "g++", "g\\+\\+ .* -o ([^\\s;]+)", 1 )
	<< ActionFormat( i18n("installing"), "", "(?:/usr/bin/install|/bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 );

	return formats;
}

void MakeActionFilter::processLine( const QString& line )
{
	bool hasmatch = false;
	QString act;
	QString file;
	QString tool;
	QValueList<ActionFormat>::const_iterator it = actionFormats().begin();
	for( ; it != actionFormats().end(); ++it )
	{
		QRegExp regExp = (*it).expression;
		if ( regExp.search( line ) == -1 )
			continue;
		hasmatch = true;
		act     = (*it).action;
		file    = regExp.cap( (*it).fileGroup );
		tool    = (*it).tool;
		break;
	}

	if ( hasmatch )
	{
		emit item( new ActionItem( act, file, tool, line ) );
	}
	else
	{
		OutputFilter::processLine( line );
	}
}
