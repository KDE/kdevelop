/* This file is part of the KDE libraries
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PROJECT_ITEM_QUICKOPEN
#define PROJECT_ITEM_QUICKOPEN

#include <KUrl>
#include <quickopendataprovider.h>
#include <quickopenfilter.h>
#include <duchainpointer.h>

namespace KDevelop {
  class ICore;
}

struct DUChainItem {
  DUChainItem() {
  }
  KDevelop::DeclarationPointer m_item;
  QString m_text;
  QString m_project;
};

class DUChainItemData : public KDevelop::QuickOpenDataBase {
  public:
    DUChainItemData( const DUChainItem& item );
    
    virtual QString text() const;
    virtual QString htmlDescription() const;

    bool execute( QString& filterText );

    virtual bool isExpandable() const;
    virtual QWidget* expandingWidget() const;

    virtual QIcon icon() const;
    
  DUChainItem m_item;
};

/**
 * A QuickOpenDataProvider for file-completion using project-files.
 * It provides all files from all open projects.
 * */

class DUChainItemDataProvider : public KDevelop::QuickOpenDataProviderBase, public KDevelop::Filter<DUChainItem> {
  public:
    enum ItemTypes {
    NoItems = 0,
    Classes = 1,
    Functions = 2,
    AllItemTypes = Classes + Functions
  };
    
    DUChainItemDataProvider();
    virtual void setFilterText( const QString& text );
    virtual void reset();
    virtual uint itemCount() const;
    virtual QList<KDevelop::QuickOpenDataPointer> data( uint start, uint end ) const;
    
    virtual void enableData( const QStringList& items, const QStringList& scopes );

    static QStringList supportedItemTypes();
  private:
  
    //Reimplemented from Base<..>
    virtual QString itemText( const DUChainItem& data ) const;

    ItemTypes m_itemTypes;
};


#endif

