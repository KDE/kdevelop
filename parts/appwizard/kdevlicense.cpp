/* This file is part of the KDE project
   Copyright (C) 2004 Sascha Cunz <sascha@sacu.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kdevlicense.h"

#include <qfile.h>
#include <qdatetime.h>
#include <qregexp.h>

KDevLicense::KDevLicense( const QString& name, const QString& fileName )
	: m_name( name )
{
	readFile( fileName );
}

void KDevLicense::readFile( const QString& fileName )
{
	QFile f(fileName);
	if (!f.open(IO_ReadOnly))
		return;
	QTextStream stream(&f);
	QString str;
	enum { readingText, readingFiles } mode = readingText;
	for(;;)
	{
		str = stream.readLine();
		if( str.isNull() )
			break;
		if( str == "[FILES]" )
			mode = readingFiles;
		else if( str == "[PREFIX]" )
			mode = readingText;
		else if( mode == readingFiles )
		{
			if( !str.isEmpty() )
			{
				m_copyFiles.append( str );
			}
		} else
		m_rawLines.append( str );
	}

}

QString KDevLicense::assemble( KDevFile::CommentingStyle commentingStyle, const QString& author, const QString& email, int leadingSpaces )
{
	// first, build a CPP Style license
	
	QString strFill;
	strFill.fill( ' ', leadingSpaces );
	
	QString str = 
		strFill + "/***************************************************************************\n" +
		strFill + " *   Copyright (C) %1 by %2   *\n" +
		strFill + " *   %3   *\n" +
		strFill + " *                                                                         *\n";
	
	str = str.arg(QDate::currentDate().year()).arg(author.left(45),-45).arg(email.left(67),-67);
	
	QStringList::Iterator it;
	for( it = m_rawLines.begin(); it != m_rawLines.end(); ++it )
	{
		str += QString( "%1 *   %2 *\n").arg( strFill ).arg( *it, -69 );
	}
	
	str += strFill + " ***************************************************************************/\n";

	switch( commentingStyle )
	{
		case KDevFile::CPPStyle:
			return str;
			
		case KDevFile::PascalStyle:
			str.replace(QRegExp("/\\**\n \\*"), "{\n  ");
			str.replace(QRegExp("\\*\n \\*"), " \n  ");
			str.replace(QRegExp(" *\\**/\n"), "}\n");
			return str;
			
		case KDevFile::AdaStyle:
			str.replace(QRegExp("/\\*"), "--");
			str.replace(QRegExp(" \\*"), "--");
			str.replace(QRegExp("\\*/"), "*");
			return str;
			
		case KDevFile::BashStyle:
			str.replace(QRegExp("\\*|/"), "#");
			str.replace(QRegExp("\n ##"), "\n##");
			str.replace(QRegExp("\n #"), "\n# ");
			return str;
	}

	return "currently unknown/unsupported commenting style";
}
