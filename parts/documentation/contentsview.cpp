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
#include "contentsview.h"

#include <qheader.h>

#include <kdevpartcontroller.h>
#include <kdevdocumentationplugin.h>

#include "documentation_widget.h"
#include "documentation_part.h"

ContentsView::ContentsView(DocumentationWidget *parent, const char *name)
     :KListView(parent, name), m_widget(parent)
{
    addColumn("Contents");
    header()->hide();
    setResizeMode(AllColumns);
    setRootIsDecorated(true);
    setSorting(-1);
    
    connect(this, SIGNAL(executed(QListViewItem*, const QPoint&, int )),
        this, SLOT(itemExecuted(QListViewItem*, const QPoint&, int )));
}

ContentsView::~ContentsView()
{
}

void ContentsView::itemExecuted(QListViewItem *item, const QPoint &p, int col)
{
    DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(item);
    if (!docItem)
        return;
    //FIXME: adymo: get rid of "context"
    m_widget->part()->partController()->showDocument(docItem->url(), "1");
}

#include "contentsview.moc"
