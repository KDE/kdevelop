/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

/***************************************************************************
 *   Copyright (C) 2004 by Tobias Glaesser                                  *
 *   tobi.web@gmx.de                                                 *
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


QProcess* proc_man = NULL;
QProcess* proc_info = NULL;
QListViewItem* man_item = NULL;
QListViewItem* info_item = NULL;
QListViewItem* index_item = NULL;
QListViewItem* google_item = NULL;
QListViewItem* contents_item = NULL;
QListViewItem* last_item = NULL;
DocumentationWidget* m_widget = NULL;
FindDocumentationOptions* m_options = NULL;
bool first_match_found;

struct DocItem : public QListViewItem
{
    DocItem(QListViewItem* item, const QString& str) : QListViewItem(item,str) {}
    DocItem(QListViewItem* item, QListViewItem* after, const QString& str) : QListViewItem(item, after, str) {}
    KURL url;
};

void FindDocumentation::init()
{
    QWidget* tmp = QApplication::desktop();
    setGeometry(tmp->width()/2 - width()/2, tmp->height()/2 - height()/2, width(), height());
    proc_man = new QProcess( this );
    proc_info = new QProcess( this );
    

    connect( proc_man, SIGNAL(processExited()),
                this, SLOT(procManReadFromStdout()) );
    
    connect( proc_info, SIGNAL(processExited()),
                this, SLOT(procInfoReadFromStdout()) );
    
    m_widget = dynamic_cast<DocumentationWidget*>(parent());
    
    result_list->header()->hide();
    result_list->setSorting(-1);
    
    m_options = new FindDocumentationOptions();
    
}

void FindDocumentation::destroy()
{
    delete m_options;
}

void FindDocumentation::clickOptions()
{
    if( m_options->exec() == QDialog::Accepted )
	m_options->writeOptions();
    else
	m_options->readOptions();
}

void FindDocumentation::setSearchTerm(const QString& term)
{
    search_term->setText(term);
}

void FindDocumentation::startSearch()
{
    //Clear the contents of the listview

    result_list->clear();
    
    last_item = NULL;
    first_match_found = false;
    
    QListViewItem* item = m_options->source_list->firstChild();
    
    while ( item && ( !m_options->goto_first_match->isOn() || !first_match_found )) {
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

}

void FindDocumentation::searchInIndex()
{
     index_item =new QListViewItem(result_list, last_item , "Index");
     index_item->setOpen(true);
     last_item = index_item;
     
    m_widget->part()->emitIndexSelected(m_widget->m_index->indexBox());
    m_widget->m_index->setSearchTerm(search_term->text());
    m_widget->m_index->showIndex(search_term->text());
    
   if(m_widget->m_index->m_index->selectedItem())
    {
     QListBoxText* item = dynamic_cast<QListBoxText*>(m_widget->m_index->m_index->selectedItem());
     DocItem* newitem = NULL;
     while(item)
     {
	 if(!item->text().contains(search_term->text(),false))
	     break;
	 
	 if(newitem)
	 newitem = new DocItem(index_item, newitem, item->text());
	 else
	 newitem = new DocItem(index_item, item->text());
               newitem->url = (dynamic_cast<IndexItem*>(item))->urls().first();
               item = dynamic_cast<QListBoxText*>(item->next());
     }
    }
   
   if(index_item->firstChild() && m_options->goto_first_match->isOn())
   {
       m_widget->part()->partController()->showDocument(dynamic_cast<DocItem*>(index_item->firstChild())->url); 
       first_match_found = true;
   }
}


void FindDocumentation::searchInContents()
{
     contents_item =new QListViewItem(result_list, last_item , "Contents");
     contents_item->setOpen(true);
     last_item = contents_item;

    QListViewItemIterator it( m_widget->m_contents->m_view );
    while ( it.current() ) {
	if( dynamic_cast<DocumentationItem*>(it.current())->type() == DocumentationItem::Collection || dynamic_cast<DocumentationItem*>(it.current())->type() == DocumentationItem::Catalog )
	{
	    dynamic_cast<DocumentationCatalogItem*>( it.current() )->load();
	    dynamic_cast<DocumentationCatalogItem*>( it.current() )->plugin()->setCatalogURL(dynamic_cast<DocumentationCatalogItem*>( it.current() ));
	}
        if ( it.current()->text(0).contains(search_term->text(),false) )
	{
	         DocItem* newitem = new DocItem( contents_item, it.current()->text(0) );
                       newitem->url = dynamic_cast<DocumentationItem*>( it.current() )->url();
	}
        ++it;
    }
 
   if(contents_item->firstChild() && m_options->goto_first_match->isOn())
   {
       m_widget->part()->partController()->showDocument(dynamic_cast<DocItem*>(contents_item->firstChild())->url); 
       first_match_found = true;
   }
}

void FindDocumentation::searchInGoogle()
{
     google_item =new QListViewItem(result_list, last_item  , "Google" );
     google_item->setOpen(true);
     last_item = google_item;
     
     DocItem* newitem = new DocItem(google_item,"First result for: " + search_term->text());
     newitem->url = KURL("http://www.google.com/search?q=" + search_term->text() + "&btnI");          
     newitem = new DocItem(google_item,"All results for: " + search_term->text());
     newitem->url = KURL("http://www.google.com/search?q=" + search_term->text());

   if(google_item->firstChild() && m_options->goto_first_match->isOn())
   {
       m_widget->part()->partController()->showDocument(dynamic_cast<DocItem*>(google_item->firstChild())->url); 
       first_match_found = true;
   }
}

void FindDocumentation::searchInMan()
{
        man_item =new QListViewItem(result_list, last_item , "Manual");
        man_item->setOpen(true);
        last_item = man_item;
    
        proc_man->clearArguments();
        //Search Manpages
        proc_man->addArgument( "man" );
        proc_man->addArgument( "-w" );	
        proc_man->addArgument( search_term->text());

        if ( !proc_man->start() ) {
            // error handling

        }
	
}

void FindDocumentation::searchInInfo()
{
        info_item =new QListViewItem(result_list, last_item  , "Info");
        info_item->setOpen(true);
        last_item = info_item;
	
        proc_info->clearArguments();
        //Search Manpages
        proc_info->addArgument( "info" );
        proc_info->addArgument( "-w" );	
        proc_info->addArgument( search_term->text());

        if ( !proc_info->start() ) {
            // error handling

        }
		
}

void FindDocumentation::procManReadFromStdout()
{
        // Read and process the data.
        // Bear in mind that the data might be output in chunks.
    if(proc_man->exitStatus() != 0 || !proc_man->normalExit())
    {
              proc_man->readStdout();
	proc_man->readStderr();
    }
    else
    {
	QString tmp;
	while(!(tmp = proc_man->readLineStdout()).isNull())
	{
	    DocItem* newitem = new DocItem(man_item,search_term->text());
	    newitem->url = KURL("man://" + tmp);
	}
	
    }

   if(man_item->firstChild() && m_options->goto_first_match->isOn())
   {
       m_widget->part()->partController()->showDocument(dynamic_cast<DocItem*>(man_item->firstChild())->url); 
       first_match_found = true;
   }
    
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
		
	    DocItem* newitem =new DocItem(info_item, tmp);
	    newitem->url = KURL("info:/" + search_term->text());
	}
	
    }

   if(info_item->firstChild() && m_options->goto_first_match->isOn())
   {
       m_widget->part()->partController()->showDocument(dynamic_cast<DocItem*>(info_item->firstChild())->url); 
       first_match_found = true;
   }    
    
}

void FindDocumentation::clickOnItem(QListViewItem * item)
{
if(!item)
    return;
    
if(item == man_item || item == info_item || item == index_item || item == google_item)
    return;

 DocItem* doc_item = dynamic_cast<DocItem*>(item);

 if(item->parent() == man_item ||
    item->parent() == info_item ||
    item->parent() == google_item ||
    item->parent() == index_item ||
    item->parent() == contents_item)
    m_widget->part()->partController()->showDocument(doc_item->url); 
    
}

void FindDocumentation::buttonPressedOnItem(int button, QListViewItem *item, const QPoint &pos, int c)
{
    if ((button != Qt::RightButton) || (!item))
        return;
    DocItem *docItem = dynamic_cast<DocItem*>(item);
    if (!docItem)
        return;
    
    DocUtils::docItemPopup(m_widget->part(), docItem->text(0), docItem->url, pos, true, false);
}
