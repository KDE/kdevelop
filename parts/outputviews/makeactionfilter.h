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

#ifndef MakeActionFilter_h
#define MakeActionFilter_h

#include <qobject.h>
#include <qregexp.h>
#include <q3valuelist.h>
#include "outputfilter.h"

class MakeItem;
class ActionItem;

class MakeActionFilter : public QObject, public OutputFilter
{
Q_OBJECT

public:
#ifdef DEBUG
	static void test();
#endif

	MakeActionFilter( OutputFilter& );

	class ActionFormat
        {
           public:
                ActionFormat():m_toolGroup(-1) {}
		ActionFormat( const QString&, const QString&, const char * regExp, int file);
		ActionFormat( const QString&, int tool, int file, const char * regExp);
                QString tool();
                QString file();
                const QString& action() const     {return m_action;}
                bool matches(const QString& line);
           private:
		QString m_action;
		QRegExp m_expression;
		QString m_tool;
                int m_toolGroup;
                int m_fileGroup;
	};
	static ActionFormat* actionFormats();

	void processLine( const QString& line );

signals:
	void item( MakeItem* );

private:
	static ActionItem* matchLine( const QString& line );
};

#endif
