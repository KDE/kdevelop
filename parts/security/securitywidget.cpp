/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "securitywidget.h"

#include <klocale.h>
#include <kiconloader.h>

#include <kdevpartcontroller.h>

#include "securitypart.h"

SecurityWidget::SecurityWidget(SecurityPart* part, const char* name)
    :KListView(0, name), m_part(part)
{
    addColumn(i18n("File"));
    addColumn(i18n("Line"));
    addColumn(i18n("Description"));
    addColumn(i18n("Suggestion"));
    
    connect(this, SIGNAL(executed( QListViewItem* )), this, SLOT(gotoLine( QListViewItem* )));
}

void SecurityWidget::clearResultsForFile(const QString &file)
{
    QValueList<QListViewItem*> del;
    QListViewItemIterator it(this);
    while (it.current()) 
    {
        if (it.current()->text(0) == file)
            del.append(it.current());
        ++it;
    }
    for (QValueList<QListViewItem*>::iterator it = del.begin(); it != del.end(); ++it)
        delete *it;
}

void SecurityWidget::reportWarning(const QString &file, int line, const QString &warning, const QString &suggestion)
{
    KListViewItem *it = new KListViewItem(this, file, QString("%1").arg(line), warning, suggestion);
    it->setPixmap(0, SmallIcon("messagebox_info"));
}

void SecurityWidget::reportProblem(const QString &file, int line, const QString &problem, const QString &suggestion)
{
    KListViewItem *it = new KListViewItem(this, file, QString("%1").arg(line), problem, suggestion);
    it->setPixmap(0, SmallIcon("messagebox_critical"));
}

void SecurityWidget::gotoLine(QListViewItem *item)
{
    m_part->partController()->editDocument(KURL::fromPathOrURL(item->text(0)), item->text(1).toInt(), 0);
}

#include "securitywidget.moc"
