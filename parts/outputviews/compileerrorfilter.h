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

#ifndef CompileErrorFilter_h
#define CompileErrorFilter_h

#include <qobject.h>
#include <qregexp.h>
#include <qvaluelist.h>
#include "outputfilter.h"

class MakeItem;

class CompileErrorFilter : public QObject, public OutputFilter
{
Q_OBJECT

public:
	CompileErrorFilter( OutputFilter& );

	struct ErrorFormat
	{
		ErrorFormat() {}
		ErrorFormat( const char *, int, int, int );
		ErrorFormat( const char *, int, int, int, QString );
		QRegExp expression;
		int fileGroup;
		int lineGroup;
		int textGroup;
		QString compiler;
	};
	static ErrorFormat* errorFormats();

	void processLine( const QString& line );

signals:
	void item( MakeItem* );
};

#endif
