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

#include "appoutputwidget.h"

#include <qregexp.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kapplication.h>
#include <kconfig.h>

#include "appoutputviewpart.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"

AppOutputWidget::AppOutputWidget(AppOutputViewPart* part)
    : ProcessWidget(0, "app output widget")
	, m_part(part)
{
	connect(this, SIGNAL(executed(QListBoxItem*)), SLOT(slotRowSelected(QListBoxItem*)));
	KConfig *config = kapp->config();
	config->setGroup("General Options");
	setFont(config->readFontEntry("Application Font"));
}


AppOutputWidget::~AppOutputWidget()
{}


void AppOutputWidget::childFinished(bool normal, int status)
{
    ProcessWidget::childFinished(normal, status);
}


void AppOutputWidget::slotRowSelected(QListBoxItem* row)
{
	static QRegExp assertMatch("ASSERT: \\\"([^\\\"]+)\\\" in ([^\\( ]+) \\(([\\d]+)\\)");
	static QRegExp lineInfoMatch("\\[([^:]+):([\\d]+)\\]");
	if (row) {
		if (assertMatch.exactMatch(row->text())) {
			m_part->partController()->editDocument(assertMatch.cap(2), assertMatch.cap(3).toInt() - 1);
			m_part->mainWindow()->statusBar()->message(i18n("Assertion failed: %1").arg(assertMatch.cap(1)), 10000);
			m_part->mainWindow()->lowerView(this);

		} else if (lineInfoMatch.search(row->text()) != -1) {
			m_part->partController()->editDocument(lineInfoMatch.cap(1), lineInfoMatch.cap(2).toInt() - 1);
			m_part->mainWindow()->statusBar()->message(row->text(), 10000);
			m_part->mainWindow()->lowerView(this);
		}
	}
}

#include "appoutputwidget.moc"
