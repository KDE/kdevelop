/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
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
#include "docconfiglistview.h"

#include <klocale.h>

#include "kdevdocumentationplugin.h"

DocConfigListView::DocConfigListView(QWidget *parent, const char *name)
     :KListView(parent, name)
{
    setSorting(-1);
    addColumn(i18n("TOC"));
    addColumn(i18n("Index"));
    addColumn(i18n("Search"));
    addColumn(i18n("Title"));    
//    addColumn(i18n("URL"));
    setColumnWidthMode(0, QListView::Maximum);
    setColumnWidthMode(1, QListView::Maximum);
    setColumnWidthMode(2, QListView::Maximum);
    setColumnWidthMode(3, QListView::Maximum);
//    setColumnWidthMode(4, QListView::Maximum);
    setAllColumnsShowFocus(true);

    connect(this, SIGNAL(clicked(QListViewItem*, const QPoint&, int)),
        this, SLOT(clickedItem(QListViewItem*, const QPoint&, int )));
}

DocConfigListView::~DocConfigListView()
{
}

void DocConfigListView::clickedItem(QListViewItem *item, const QPoint &pnt, int c)
{
    if (!item)
        return;
    ConfigurationItem *cfg = dynamic_cast<ConfigurationItem*>(item);
    if (!cfg)
        return;
    if (c == 0)
    {
        cfg->setContents(!cfg->contents());
        if (!cfg->contents())
            cfg->setIndex(false);
    }
    if ((c == 1) && (cfg->indexPossible()))
        cfg->setIndex(!cfg->index());
    else if ((c == 2)  && (cfg->fullTextSearchPossible()))
        cfg->setFullTextSearch(!cfg->fullTextSearch());
    repaintItem(item);
}

#include "docconfiglistview.moc"
