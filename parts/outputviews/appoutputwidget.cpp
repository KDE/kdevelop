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

#include <klocale.h>
#include <kdebug.h>
#include <kstatusbar.h>

#include "appoutputviewpart.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"

AppOutputWidget::AppOutputWidget(AppOutputViewPart* part)
    : ProcessWidget(0, "app output widget")
	, m_part(part)
{
	connect(this, SIGNAL(executed(QListBoxItem*)), SLOT(slotRowSelected(QListBoxItem*)));
}


AppOutputWidget::~AppOutputWidget()
{}


void AppOutputWidget::childFinished(bool normal, int status)
{
    ProcessWidget::childFinished(normal, status);
}


void AppOutputWidget::slotRowSelected(QListBoxItem* row)
{
	QRegExp assertMatch("ASSERT: \\\"([^\\\"]+)\\\" in ([^\\( ]+) \\(([\\d]+)\\)");
	if (row && assertMatch.exactMatch(row->text())) {
		m_part->partController()->editDocument(assertMatch.cap(2), assertMatch.cap(3).toInt() - 1);
		m_part->topLevel()->statusBar()->message(i18n("Assertion failed: %1").arg(assertMatch.cap(1)), 10000);
		m_part->topLevel()->lowerView(this);
	}
}

#include "appoutputwidget.moc"
