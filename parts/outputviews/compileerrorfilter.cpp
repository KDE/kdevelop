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

#include "compileerrorfilter.h"
#include "compileerrorfilter.moc"
#include "makeitem.h"

CompileErrorFilter::ErrorFormat::ErrorFormat( const char * regExp, int file, int line, int text )
	: expression( regExp )
	, fileGroup( file )
	, lineGroup( line )
	, textGroup( text )
{
}

CompileErrorFilter::CompileErrorFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

const QValueList<CompileErrorFilter::ErrorFormat>& CompileErrorFilter::errorFormats()
{
	static QValueList<ErrorFormat> formats
		= QValueList<ErrorFormat>()

	// TODO: could get these from emacs compile.el

	// GCC
	<< ErrorFormat( "([^: \t]+):([0-9]+):(.*)", 1, 2, 3 )
	// Fortran
	<< ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 )
	// Jade
	<< ErrorFormat( "[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 );

	return formats;
}

void CompileErrorFilter::processLine( const QString& line )
{
	bool hasmatch = false;
	QString file;
	int lineNum;
	QString text;
	QValueList<ErrorFormat>::const_iterator it = errorFormats().begin();
	for( ; it != errorFormats().end(); ++it )
	{
		QRegExp regExp = (*it).expression;

		if ( regExp.search( line ) == -1 )
			continue;

		hasmatch = true;
		file    = regExp.cap( (*it).fileGroup );
		lineNum = regExp.cap( (*it).lineGroup ).toInt() - 1;
		text    = regExp.cap( (*it).textGroup );

		break;
	}

	if( hasmatch )
	{
		// Add hacks for error strings you want excluded here
		if( text.find( QString::fromLatin1("(Each undeclared identifier is reported only once") ) >= 0
			|| text.find( QString::fromLatin1("for each function it appears in.)") ) >= 0 )
		hasmatch = false;
	}

	if ( hasmatch )
	{
		emit item( new ErrorItem( file, lineNum, text, line ) );
	}
	else
	{
		OutputFilter::processLine( line );
	}
}
