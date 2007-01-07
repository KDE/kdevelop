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

#ifndef _APPOUTPUTWIDGET_H_
#define _APPOUTPUTWIDGET_H_

#include <qstringlist.h>
#include <kregexp.h>

#include "processwidget.h"

class AppOutputViewPart;

class AppOutputWidget : public ProcessWidget
{
    Q_OBJECT
friend class AppOutputViewPart;
public:
    AppOutputWidget(AppOutputViewPart* part);
    ~AppOutputWidget();
    // clears our own store and the one of our base ProcessWidget

public slots:
	void slotRowSelected(QListBoxItem* row);
	void insertStdoutLine(const QString &line);
	void insertStderrLine(const QString &line);
	void slotContextMenu(QListBoxItem *, const QPoint &);
	void hideView();
	void clearViewAndContents();
	void clearFilter();
	void editFilter();
	void saveAll();
	void saveFiltered();

private:
	virtual void childFinished(bool normal, int status);
	void saveOutputToFile( bool useFilter );
	void reinsertAndFilter();
	bool filterSingleLine( const QString & line );

	QStringList m_contentList;

	struct OutputFilter
	{
		OutputFilter() : m_isActive(false), m_isRegExp(false), m_caseSensitive(false) {}
		bool m_isActive;
		bool m_isRegExp;
		bool m_caseSensitive;
		QString m_filterString;
	};

	AppOutputViewPart* m_part;
	OutputFilter m_filter;
};

#endif
