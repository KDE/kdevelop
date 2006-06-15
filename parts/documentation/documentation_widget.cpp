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
#include <qtoolbox.h>

#include <klineedit.h>
#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>

#include <kdevcore.h>
#include <kdevdocumentationplugin.h>

#include "documentation_part.h"
#include "contentsview.h"
#include "indexview.h"
#include "searchview.h"
#include "bookmarkview.h"
#include "find_documentation.h"

DocumentationWidget::DocumentationWidget(DocumentationPart *part)
    :QWidget(0, "documentation widget"), m_part(part)
{
    QVBoxLayout *l = new QVBoxLayout(this, 0, 0);
    
    m_tab = new QToolBox(this);
    l->addWidget(m_tab);
    
    m_contents = new ContentsView(this);
    m_tab->addItem(m_contents, i18n("Contents"));
    
    m_index = new IndexView(this);
    m_tab->addItem(m_index, i18n("Index"));
    
    m_finder = new FindDocumentation(this);
    m_tab->addItem(m_finder,i18n("Finder"));
    
    m_search = new SearchView(m_part, this);
    m_tab->addItem(m_search, i18n("Search"));
    
    m_bookmarks = new BookmarkView(this);
    m_tab->addItem(m_bookmarks, i18n("Bookmarks"));
    
    connect(m_tab, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
}

DocumentationWidget::~DocumentationWidget()
{
    KConfig *config = kapp->config();
    config->setGroup("Documentation");
    config->writeEntry("LastPage", m_tab->currentIndex());
}

void DocumentationWidget::tabChanged(int t)
{
    if (!m_tab->item(t))
        return;
    m_tab->item(t)->setFocus();
    if (m_tab->item(t) == m_index)
        m_part->emitIndexSelected(m_index->indexBox());
}

IndexBox *DocumentationWidget::index() const
{
    return m_index->indexBox();
}

void DocumentationWidget::searchInDocumentation()
{
    m_tab->setCurrentItem(m_search);
    m_search->askSearchTerm();
}

void DocumentationWidget::searchInDocumentation(const QString &term)
{
    m_tab->setCurrentItem(m_search);
    m_search->setSearchTerm(term);
    m_search->search();
}

void DocumentationWidget::lookInDocumentationIndex()
{
    m_tab->setCurrentItem(m_index);
    m_index->askSearchTerm();
}

void DocumentationWidget::lookInDocumentationIndex(const QString &term)
{
    m_tab->setCurrentItem(m_index);
    m_index->setSearchTerm(term);
    //adymo: let's allow the user to press enter here ;)
//    m_index->searchInIndex();
}

void DocumentationWidget::findInDocumentation(const QString &term)
{
   m_tab->setCurrentItem(m_finder);
   m_finder->setSearchTerm(term);
   m_finder->startSearch();
}

void DocumentationWidget::findInDocumentation()
{
   m_tab->setCurrentItem(m_finder);
   m_finder->search_term->setFocus();
}

void DocumentationWidget::focusInEvent(QFocusEvent */*e*/)
{
    if (m_tab->currentItem())
        m_tab->currentItem()->setFocus();
}

KListView * DocumentationWidget::contents( ) const
{
    return m_contents->view();
}

#include "documentation_widget.moc"
