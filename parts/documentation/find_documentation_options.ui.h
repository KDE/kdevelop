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

KConfig* config = NULL;

void FindDocumentationOptions::init()
{    
 
m_man_item = NULL;
m_info_item = NULL;
m_index_item = NULL;
m_google_item = NULL;
m_contents_item = NULL;
    
 readOptions();
  
 source_list->header()->hide();
 source_list->setSorting(-1);
}

bool FindDocumentationOptions::isMan(QCheckListItem* item)
{
    if ( item == m_man_item && m_man_item->isOn() )
	return true;
    else 
	return false;
}

bool FindDocumentationOptions::isInfo(QCheckListItem* item)
{
    if ( item == m_info_item  && m_info_item->isOn() )
	return true;
    else 
	return false;
}

bool FindDocumentationOptions::isIndex(QCheckListItem* item)
{
    if ( item == m_index_item  && m_index_item->isOn() )
	return true;
    else 
	return false;
}

bool FindDocumentationOptions::isGoogle(QCheckListItem* item)
{
    if ( item == m_google_item && m_google_item->isOn()  )
	return true;
    else 
	return false;
}

bool FindDocumentationOptions::isContents(QCheckListItem* item)
{
    if ( item == m_contents_item && m_contents_item->isOn()  )
	return true;
    else 
	return false;
}

void FindDocumentationOptions::readOptions()
{
    config = kapp->config();
    config->setGroup("DocumentationFinder");
    
    source_list->clear();
    
    for(int i = 4; i > -1; --i  )
    {
      if( config->readPropertyEntry( "Manpage" , 0 ) == i)
      {
        m_man_item  = new QCheckListItem( source_list, i18n("Manual"), QCheckListItem::CheckBox ); 	
        m_man_item->setOn(config->readBoolEntry( "ManpageEnabled" , true));
      }
      if( config->readPropertyEntry( "Info" , 1 ) == i)
      {
        m_info_item = new QCheckListItem( source_list, i18n("Info"), QCheckListItem::CheckBox );
        m_info_item->setOn(config->readBoolEntry( "InfoEnabled" , true));
      }
      if( config->readPropertyEntry( "Index" , 2 ) == i)
      {
        m_index_item = new QCheckListItem( source_list, i18n("Index"), QCheckListItem::CheckBox );
        m_index_item->setOn(config->readBoolEntry( "IndexEnabled" , true));
      }
      if( config->readPropertyEntry( "Google" , 3 ) == i)
      {
        m_google_item = new QCheckListItem( source_list, i18n("Google"), QCheckListItem::CheckBox );	   
        m_google_item->setOn(config->readBoolEntry( "GoogleEnabled" , false)); 
      }
      if( config->readPropertyEntry( "Contents" , 4 ) == i)
      {
        m_contents_item = new QCheckListItem( source_list, i18n("Contents"), QCheckListItem::CheckBox );	   
        m_contents_item->setOn(config->readBoolEntry( "ContentsEnabled" , false)); 
      }      
  }
    
    goto_first_match->setChecked(config->readBoolEntry( "goto_first_match" , false));
}

void FindDocumentationOptions::writeOptions()
{
    config = kapp->config();
    config->setGroup("DocumentationFinder");
    
    config->writeEntry("goto_first_match", goto_first_match->isChecked());
    
    QListViewItemIterator it( source_list );
    int i = 0;
    while ( it.current() ) {
        if ( it.current() == m_man_item )
	{
	    config->writeEntry("Manpage",i);
	    config->writeEntry("ManpageEnabled",m_man_item->isOn());
	}
        else if ( it.current() == m_info_item )
	{
	    config->writeEntry("Info",i);
	    config->writeEntry("InfoEnabled",m_info_item->isOn());
	}
        else if ( it.current() == m_index_item )
	{
	    config->writeEntry("Index",i);
	    config->writeEntry("IndexEnabled",m_index_item->isOn());
	}
        else if ( it.current() == m_google_item )
	{
	    config->writeEntry("Google",i);
	    config->writeEntry("GoogleEnabled",m_google_item->isOn());
	}
        else if ( it.current() == m_contents_item )
	{
	    config->writeEntry("Contents",i);
	    config->writeEntry("ContentsEnabled",m_contents_item->isOn());
	}	
        ++it;
        ++i;
    }  
    
    config->sync();
}

void FindDocumentationOptions::sourceMoveUp()
{
    if (!source_list->currentItem())
        return;
    if (source_list->currentItem() == source_list->firstChild()) 
        return;
    
    QListViewItem *item = source_list->firstChild();
    while (item->nextSibling() != source_list->currentItem())
        item = item->nextSibling();
    item->moveItem( source_list->currentItem()); 
}

void FindDocumentationOptions::sourceMoveDown()
{
   if (!source_list->currentItem())
       return;
   if (source_list->currentItem()->nextSibling() == 0)
        return;

   source_list->currentItem()->moveItem(source_list->currentItem()->nextSibling()); 
}
