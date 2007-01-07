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

#include <kdebug.h>

CompileErrorFilter::ErrorFormat::ErrorFormat( const char * regExp, int file, int line, int text )
	: expression( regExp )
	, fileGroup( file )
	, lineGroup( line )
	, textGroup( text )
{
}

CompileErrorFilter::ErrorFormat::ErrorFormat( const char * regExp, int file, int line, int text, QString comp )
	: expression( regExp )
	, fileGroup( file )
	, lineGroup( line )
	, textGroup( text )
	, compiler( comp )
{
}


CompileErrorFilter::CompileErrorFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

CompileErrorFilter::ErrorFormat* CompileErrorFilter::errorFormats()
{
	/// @todo could get these from emacs compile.el
	static ErrorFormat formats[] = {
		// GCC - another case, eg. for #include "pixmap.xpm" which does not exists
		ErrorFormat( "([^: \t]+):([0-9]+):(?:[0-9]+):([^0-9]+)", 1, 2, 3 ),
		// GCC
		ErrorFormat( "([^: \t]+):([0-9]+):([^0-9]+)", 1, 2, 3 ),
		// ICC
		ErrorFormat( "([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" ),
		//libtool link
		ErrorFormat( "(libtool):( link):( warning): ", 0, 0, 0 ),
		// Fortran
		ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 ),
		// Jade
		ErrorFormat( "[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 ),
		// ifort
		ErrorFormat( "fortcom: Error: (.*), line ([0-9]+):(.*)", 1, 2, 3, "intel" ),
		// PGI
		ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" ),
		// PGI (2)
		ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" ),

		ErrorFormat( 0, 0, 0, 0 ) // this one last
	};

	return formats;
}

void CompileErrorFilter::processLine( const QString& line )
{
	bool hasmatch = false;
	QString file;
	int lineNum = 0;
	QString text;
	QString compiler;
	int i = 0;
	bool isWarning = false;
	bool isInstantiationInfo = false;
	ErrorFormat* const errFormats = errorFormats();
	ErrorFormat* format = &errFormats[i];
	while( !format->expression.isEmpty() )
	{
		QRegExp& regExp = format->expression;

		if ( regExp.search( line ) != -1 ) {
			hasmatch = true;
			file    = regExp.cap( format->fileGroup );
			lineNum = regExp.cap( format->lineGroup ).toInt() - 1;
			text    = regExp.cap( format->textGroup );
			compiler = format->compiler;
			QString cap = regExp.cap(3);
			if (cap.contains("warning:", false) || cap.contains("Warnung:", false))
				isWarning = true;
			if (regExp.cap(3).contains("instantiated from", false)) {
				isInstantiationInfo = true;
			}
			break;
		}

		format = &errFormats[++i];
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
		emit item( new ErrorItem( file, lineNum, text, line, isWarning, isInstantiationInfo, compiler ) );
	}
	else
	{
		OutputFilter::processLine( line );
	}
}
