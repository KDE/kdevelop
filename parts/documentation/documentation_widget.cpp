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
#include "documentation_widget.h"

#include <qlayout.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <klocale.h>
#include <kdebug.h>

#include <kdevcore.h>
#include <kdevdocumentationplugin.h>

#include "documentation_part.h"
#include "contentsview.h"
#include "indexview.h"

DocumentationWidget::DocumentationWidget(DocumentationPart *part)
    :QWidget(0, "documentation widget"), m_part(part)
{
    QVBoxLayout *l = new QVBoxLayout(this, 0, 0);
    
    m_tab = new KTabWidget(this);
    l->addWidget(m_tab);
    
    m_contents = new ContentsView(this);
    m_tab->addTab(m_contents, i18n("Contents"));
    
    m_index = new IndexView(this);
    m_tab->addTab(m_index, i18n("Index"));
    
    connect(m_tab, SIGNAL(currentChanged(QWidget *)), this, SLOT(tabChanged(QWidget* )));
}

DocumentationWidget::~DocumentationWidget()
{
}

void DocumentationWidget::tabChanged(QWidget * w)
{
    if (w == m_index)
    {
        m_part->emitIndexSelected(m_index->indexBox());
    }
}

KListBox *DocumentationWidget::index() const
{
    return m_index->indexBox();
}

#include "documentation_widget.moc"
