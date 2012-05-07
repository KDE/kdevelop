/* This file is part of the KDE libraries
   Copyright (C) 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef CUSTOM_LIST_QUICKOPEN
#define CUSTOM_LIST_QUICKOPEN

#include <KUrl>
#include <language/interfaces/quickopendataprovider.h>
#include <language/interfaces/quickopenfilter.h>
#include <language/editor/simplecursor.h>

namespace KDevelop {
  class ICore;
}

struct CustomItem {
  QString m_identifier;
  QString m_shortText;

  ///@todo make sure this doesn't leak and isn't retrieved multiple times. Currently, we depend on the fact that it is retrieved exactly once.
  QWidget* m_widget;

  KUrl m_executeTargetUrl;
  KDevelop::SimpleCursor m_executeTargetPosition;
};

class CustomItemData : public KDevelop::QuickOpenDataBase {
  public:
    CustomItemData(const CustomItem& item);
    virtual QString text() const;
    virtual QString htmlDescription() const;

    bool execute( QString& filterText );

    virtual bool isExpandable() const;
    virtual QWidget* expandingWidget() const;

    virtual QIcon icon() const;
  private:

    CustomItem m_item;
};

/**
 * A QuickOpenDataProvider for file-completion using project-files.
 * It provides all files from all open projects.
 * */

typedef KDevelop::Filter<CustomItem> CustomItemDataProviderBase;

class CustomItemDataProvider : public KDevelop::QuickOpenDataProviderBase, public CustomItemDataProviderBase {
  public:
    CustomItemDataProvider(const QList<CustomItem>& items);
    virtual void setFilterText( const QString& text );
    virtual void reset();
    virtual uint itemCount() const;
    virtual uint unfilteredItemCount() const;
    virtual QList<KDevelop::QuickOpenDataPointer> data( uint start, uint end ) const;

  private:
  
    //Reimplemented from CustomItemDataProviderBase<..>
    virtual QString itemText( const CustomItem& data ) const;
};


#endif

