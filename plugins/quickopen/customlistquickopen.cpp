
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

#include "customlistquickopen.h"
#include <QIcon>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>

using namespace KDevelop;

CustomItemData::CustomItemData(const CustomItem& item) : m_item(item) {
}

QString CustomItemData::text() const {
  return m_item.m_identifier;
}

QString CustomItemData::htmlDescription() const {
  return "<small><small>" + m_item.m_shortText + "</small></small>";
}

bool CustomItemData::execute( QString& /*filterText*/ ) {
  ICore::self()->documentController()->openDocument( m_item.m_executeTargetUrl, m_item.m_executeTargetPosition.textCursor() );
  return true;
}

bool CustomItemData::isExpandable() const {
  return true;
}

QWidget* CustomItemData::expandingWidget() const {
  QWidget* ret = m_item.m_widget;
  return ret;
}

QIcon CustomItemData::icon() const {
  return QIcon();
}

CustomItemDataProvider::CustomItemDataProvider(const QList<CustomItem>& items) {
  setItems(items);
  reset();
}

void CustomItemDataProvider::setFilterText( const QString& text ) {
  CustomItemDataProviderBase::setFilter( text );
}

void CustomItemDataProvider::reset() {
  CustomItemDataProviderBase::clearFilter();
}

uint CustomItemDataProvider::itemCount() const {
  return CustomItemDataProviderBase::filteredItems().count();
}

uint CustomItemDataProvider::unfilteredItemCount() const
{
  return CustomItemDataProviderBase::items().count();
}

KDevelop::QuickOpenDataPointer CustomItemDataProvider::data( uint row ) const {
  return KDevelop::QuickOpenDataPointer( new CustomItemData( CustomItemDataProviderBase::filteredItems()[row] ) );
}

QString CustomItemDataProvider::itemText( const CustomItem& data ) const {
  return data.m_identifier;
}

