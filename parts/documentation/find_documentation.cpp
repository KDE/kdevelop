/***************************************************************************
 *   Copyright (C) 2004 by Tobias Glaesser                                 *
 *   tobi.web@gmx.de                                                       *
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
#include "find_documentation.h"

#include <stdlib.h>
#include <qprocess.h>
#include <qcheckbox.h>
#include <qheader.h>
#include <qapplication.h>

#include <klineedit.h>
#include <klistbox.h>

#include <kdevpartcontroller.h>
#include <kdevdocumentationplugin.h>

#include "documentation_widget.h"
#include "documentation_part.h"
#include "indexview.h"
#include "find_documentation_options.h"
#include "docutils.h"
#include "contentsview.h"

#include <klocale.h>

FindDocumentation::FindDocumentation(DocumentationWidget* parent, const char* name)
    :FindDocumentationBase(parent, name),
    man_item(0), info_item(0), index_item(0), google_item(0), contents_item(0),
    last_item(0), m_widget(parent)
{
    QWidget* tmp = QApplication::desktop();
    setGeometry(tmp->width()/2 - width()/2, tmp->height()/2 - height()/2, width(), height());
    proc_man = new QProcess( this );
    proc_info = new QProcess( this );

    connect( proc_man, SIGNAL(processExited()),
                this, SLOT(procManReadFromStdout()) );

    connect( proc_info, SIGNAL(processExited()),
                this, SLOT(procInfoReadFromStdout()) );

    result_list->header()->hide();
    result_list->setSorting(-1);

    m_options = new FindDocumentationOptions(this);
}

FindDocumentation::~FindDocumentation()
{
}

/*$SPECIALIZATION$*/
void FindDocumentation::buttonPressedOnItem( int button, QListViewItem * item, const QPoint & pos, int // c
                                             )
{
    if ((button != Qt::RightButton) || (!item))
        return;
    DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(item);
    if (!docItem)
        return;

    DocUtils::docItemPopup(m_widget->part(), docItem->text(0), docItem->url(), pos, true, false);
}

void FindDocumentation::clickOnItem( QListViewItem * item )
{
    if(!item)
        return;

    if(item == man_item || item == info_item || item == index_item || item == google_item)
        return;

    DocumentationItem* doc_item = dynamic_cast<DocumentationItem*>(item);

    if(item->parent() == man_item ||
        item->parent() == info_item ||
        item->parent() == google_item ||
        item->parent() == index_item ||
        item->parent() == contents_item)
        m_widget->part()->partController()->showDocument(doc_item->url());
}

void FindDocumentation::procInfoReadFromStdout()
{
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.
    if(proc_info->exitStatus() != 0 || !proc_info->normalExit())
    {
        proc_info->readStdout();
        proc_info->readStderr();
    }
    else
    {
        QString tmp;
        while(!(tmp = proc_info->readLineStdout()).isNull())
        {
            if(tmp[0] == '*')
                break;

            DocumentationItem* newitem = new DocumentationItem(DocumentationItem::Document, info_item, tmp);
            newitem->setURL(KURL("info:/" + search_term->text()));
        }
    }

    if(info_item->firstChild() && m_options->goto_first_match->isOn())
    {
        m_widget->part()->partController()->showDocument(dynamic_cast<DocumentationItem*>(info_item->firstChild())->url());
        first_match_found = true;
    }
}

void FindDocumentation::procManReadFromStdout()
{
    // Read and process the data.
    // Bear in mind that the data might be output in chunks.
    if (proc_man->exitStatus() != 0 || !proc_man->normalExit())
    {
        proc_man->readStdout();
        proc_man->readStderr();
    }
    else
    {
        QString tmp;
        while(!(tmp = proc_man->readLineStdout()).isNull())
        {
            DocumentationItem* newitem = new DocumentationItem(DocumentationItem::Document, man_item, search_term->text());
            newitem->setURL(KURL("man://" + tmp));
        }
    }

    if(man_item->firstChild() && m_options->goto_first_match->isOn())
    {
        m_widget->part()->partController()->showDocument(dynamic_cast<DocumentationItem*>(man_item->firstChild())->url());
        first_match_found = true;
    }
}

void FindDocumentation::searchInInfo()
{
    info_item = new KListViewItem(result_list, last_item, "Info");
    info_item->setOpen(true);
    last_item = info_item;

    proc_info->clearArguments();
    //Search Manpages
    proc_info->addArgument( "info" );
    proc_info->addArgument( "-w" );
    proc_info->addArgument( search_term->text() );

    if ( !proc_info->start() )
    {
        // error handling
    }
}

void FindDocumentation::searchInMan()
{
    man_item = new KListViewItem( result_list, last_item, i18n("Manual") );
    man_item->setOpen(true);
    last_item = man_item;

    proc_man->clearArguments();
    //Search Manpages
    proc_man->addArgument( "man" );
    proc_man->addArgument( "-w" );
    proc_man->addArgument( search_term->text() );

    if ( !proc_man->start() )
    {
        // error handling
    }

}

void FindDocumentation::searchInGoogle()
{
    google_item = new KListViewItem(result_list, last_item, "Google");
    google_item->setOpen(true);
    last_item = google_item;

    DocumentationItem* newitem = new DocumentationItem(DocumentationItem::Document,
        google_item, "First result for: " + search_term->text());
    newitem->setURL(KURL("http://www.google.com/search?q=" + search_term->text() + "&btnI"));
    newitem = new DocumentationItem(DocumentationItem::Document, google_item, "All results for: " + search_term->text());
    newitem->setURL(KURL("http://www.google.com/search?q=" + search_term->text()));

    if (google_item->firstChild() && m_options->goto_first_match->isOn())
    {
        m_widget->part()->partController()->showDocument(dynamic_cast<DocumentationItem*>(google_item->firstChild())->url());
        first_match_found = true;
    }
}

void FindDocumentation::searchInContents()
{
    contents_item = new KListViewItem( result_list, last_item, i18n("Contents") );
    contents_item->setOpen(true);
    last_item = contents_item;

    QListViewItemIterator it( m_widget->m_contents->m_view );
    while ( it.current() )
    {
        DocumentationItem *docItem = dynamic_cast<DocumentationItem*>(it.current());

        if (docItem->type() == DocumentationItem::Catalog)
        {
            DocumentationCatalogItem *catalogItem = dynamic_cast<DocumentationCatalogItem*>(it.current());
            catalogItem->load();
            catalogItem->plugin()->setCatalogURL(catalogItem);
        }
        if (it.current()->text(0).contains(search_term->text(),false))
        {
            DocumentationItem* newitem = new DocumentationItem(DocumentationItem::Document,
                contents_item, it.current()->text(0) );
            newitem->setURL(docItem->url());
        }
        ++it;
    }

    if (contents_item->firstChild() && m_options->goto_first_match->isOn())
    {
        m_widget->part()->partController()->showDocument(dynamic_cast<DocumentationItem*>(contents_item->firstChild())->url());
        first_match_found = true;
    }
}

void FindDocumentation::searchInIndex()
{
    index_item =new KListViewItem(result_list, last_item , "Index");
    index_item->setOpen(true);
    last_item = index_item;

    m_widget->part()->emitIndexSelected(m_widget->m_index->indexBox());
    m_widget->m_index->setSearchTerm(search_term->text());
    m_widget->m_index->showIndex(search_term->text());

    if(m_widget->m_index->m_index->selectedItem())
    {
        IndexItem* item = dynamic_cast<IndexItem*>(m_widget->m_index->m_index->selectedItem());
        DocumentationItem* newitem = 0;
        while(item)
        {
            if(!item->text().contains(search_term->text(),false))
                break;

            IndexItem::List urls = item->urls();
            for (IndexItem::List::const_iterator it = urls.begin(); it != urls.end(); ++it)
            {
                QString text = item->text();
                if (urls.count() > 1)
                    text = (*it).first;
                if(newitem)
                    newitem = new DocumentationItem(DocumentationItem::Document, index_item,
                        newitem, text);
                else
                    newitem = new DocumentationItem(DocumentationItem::Document,
                        index_item, text);

                newitem->setURL((*it).second);
            }

            item = dynamic_cast<IndexItem*>(item->next());
        }
    }

    if(index_item->firstChild() && m_options->goto_first_match->isOn())
    {
        m_widget->part()->partController()->showDocument(
            dynamic_cast<DocumentationItem*>(index_item->firstChild())->url());
        first_match_found = true;
    }
}

void FindDocumentation::startSearch()
{
    //Clear the contents of the listview
    result_list->clear();

    last_item = NULL;
    first_match_found = false;

    QListViewItem* item = m_options->source_list->firstChild();

    while ( item && ( !m_options->goto_first_match->isOn() || !first_match_found ))
    {
        if ( m_options->isMan(dynamic_cast<QCheckListItem*>(item)) )
            searchInMan();
        else if ( m_options->isInfo(dynamic_cast<QCheckListItem*>(item)) )
            searchInInfo();
        else if ( m_options->isIndex(dynamic_cast<QCheckListItem*>(item)) )
            searchInIndex();
        else if ( m_options->isGoogle(dynamic_cast<QCheckListItem*>(item)) )
            searchInGoogle();
        else if ( m_options->isContents(dynamic_cast<QCheckListItem*>(item)) )
            searchInContents();

        item = item->itemBelow();
    }

    //Set the focus to the listview and the first child to the
    //current item of the list, so that you can brows the items with your keyboard
    result_list->setFocus();
    if(result_list->firstChild())
      result_list->setCurrentItem(result_list->firstChild());

}

void FindDocumentation::setSearchTerm( const QString & term )
{
    search_term->setText(term);
}

void FindDocumentation::clickOptions()
{
    if( m_options->exec() == QDialog::Accepted )
        m_options->writeOptions();
    else
        m_options->readOptions();
}

void FindDocumentation::focusInEvent( QFocusEvent * // e
                                      )
{
    search_term->setFocus();
}

#include "find_documentation.moc"
