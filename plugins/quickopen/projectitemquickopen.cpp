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

#include "projectitemquickopen.h"
#include <language/duchain/topducontext.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/types/structuretype.h>
#include <project/projectmodel.h>
#include <klocale.h>
#include <language/interfaces/iquickopen.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/codemodel.h>

using namespace KDevelop;

ProjectItemDataProvider::ProjectItemDataProvider( KDevelop::IQuickOpen* quickopen ) : m_quickopen(quickopen) {
}

struct SubstringCache {
SubstringCache(QString string = QString()) : substring(string) {
}

inline bool containedIn(Identifier id) {
    int index = id.index();
    QHash<int, bool>::const_iterator it = cache.constFind(index);
    if(it != cache.constEnd())
    return *it;
    bool result = id.identifier().str().contains( substring, Qt::CaseInsensitive );
    cache[index] = result;
    return result;
}
QString substring;
QHash<int, bool> cache;
};

void ProjectItemDataProvider::setFilterText( const QString& text ) {
  
  m_addedItems.clear();

  QStringList search(text.split("::", QString::SkipEmptyParts));
  for(int a = 0; a < search.count(); ++a) {
    if(search[a].endsWith(':')) //Don't get confused while the :: is being typed
      search[a] = search[a].left(search[a].length()-1);
  }
  if(!search.isEmpty() && search.back().endsWith('(')) {
    search.back() = search.back().left(search.back().length()-1);
  }

  if(text.isEmpty() || search.isEmpty()) {
    m_filteredItems = m_currentItems;
    return;
  }
  KDevVarLengthArray<SubstringCache, 5> cache;
  for(int a = 0; a < search.count(); ++a) {
    cache.append(SubstringCache(search.at(a)));
  }

  if(!text.startsWith(m_currentFilter)) {
    m_filteredItems = m_currentItems;
  }

  m_currentFilter = text;

  QVector<CodeModelViewItem> oldFiltered = m_filteredItems;
  m_filteredItems.clear();

  for(int a = 0; a < oldFiltered.size(); ++a) {
    QualifiedIdentifier currentId = oldFiltered[a].m_id;
    int nextMatchPos = 0;

    bool mismatch = false;
    for(int b = 0; b < search.count(); ++b) {
      bool localMatch = false;

      for(int q = nextMatchPos; q < currentId.count(); ++q) {
        if(cache[b].containedIn( currentId.at(q) )) {
          localMatch = true;
          nextMatchPos = q+1;
          break;
        }
      }
      if(!localMatch) {
        mismatch = true;
        break;
      }else{
        continue;
      }
    }
    if(!mismatch)
      m_filteredItems << oldFiltered[a];
  }
}

QList<KDevelop::QuickOpenDataPointer> ProjectItemDataProvider::data( uint start, uint end ) const {

  QList<KDevelop::QuickOpenDataPointer> ret;
  for(uint a = start; a < end; ++a)
    ret << data(a);
  return ret;
}

KDevelop::QuickOpenDataPointer ProjectItemDataProvider::data( uint pos ) const {

  //Check whether this position falls into an appended item-list, else apply the offset
   uint filteredItemOffset = 0;
   for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it) {
     int offsetInAppended = pos - (it.key()+1);
     if(offsetInAppended >= 0 && offsetInAppended < it.value().count()) {
       return it.value()[offsetInAppended];
     }
     if(it.key() >= pos)
       break;
     filteredItemOffset += it.value().count();
   }
//     
//     add += it.value().count();

  uint a = pos - filteredItemOffset;
  if(a > (uint)m_filteredItems.size())
    return KDevelop::QuickOpenDataPointer();

  QList<KDevelop::QuickOpenDataPointer> ret;
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    TopDUContext* ctx = DUChainUtils::standardContextForUrl(m_filteredItems[a].m_file.toUrl());
    if(ctx) {
      QList<Declaration*> decls = ctx->findDeclarations(m_filteredItems[a].m_id, CursorInRevision::invalid(), AbstractType::Ptr(), 0, DUContext::DirectQualifiedLookup);

      //Filter out forward-declarations
      foreach(Declaration* decl, decls)
        if(decl->isForwardDeclaration() && decls.size() > 1)
          decls.removeAll(decl);

      foreach(Declaration* decl, decls) {
        DUChainItem item;
        item.m_item = decl;
        item.m_text = decl->qualifiedIdentifier().toString();
        //item.m_project =  .. @todo fill
        ret << QuickOpenDataPointer(new DUChainItemData(item, true));
      }
      if(decls.isEmpty()) {
        DUChainItem item;
        item.m_text = m_filteredItems[a].m_id.toString();
        //item.m_project =  .. @todo fill
        ret << QuickOpenDataPointer(new DUChainItemData(item));
      }
    }else{
      kDebug() << "Could not find standard-context";
    }

  if(!ret.isEmpty()) {
    QList<KDevelop::QuickOpenDataPointer> append = ret.mid(1);
    if(!append.isEmpty()) {
        QMap<uint, QList<KDevelop::QuickOpenDataPointer> > addMap;
        for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::iterator it = m_addedItems.begin(); it != m_addedItems.end();) {
            if(it.key() == pos) //There already is appended data stored, nothing to do
                return ret.first();
            else if(it.key() > pos) {
                addMap[it.key() + append.count()] = it.value();
                it = m_addedItems.erase(it);
            }else{
                ++it;
            }
        }
   
        m_addedItems.insert(pos, append);

        for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = addMap.constBegin(); it != addMap.constEnd(); ++it)
            m_addedItems.insert(it.key(), it.value());
    }
    return ret.first();
  } else
    return KDevelop::QuickOpenDataPointer();
}

void ProjectItemDataProvider::reset() {
  m_usingFiles = m_quickopen->fileSet();
  m_currentItems.clear();
  m_addedItems.clear();

  KDevelop::DUChainReadLocker lock( DUChain::lock() );
  foreach( const IndexedString &u, m_usingFiles ) {
    uint count;
    const KDevelop::CodeModelItem* items;
    CodeModel::self().items( u, count, items );

    for(uint a = 0; a < count; ++a) {
      if(!items[a].id.isValid() || items[a].kind & CodeModelItem::ForwardDeclaration)
        continue;
      if(((m_itemTypes & Classes) && (items[a].kind & CodeModelItem::Class)) ||
         ((m_itemTypes & Functions) && (items[a].kind & CodeModelItem::Function)))
      {
        QualifiedIdentifier id = items[a].id.identifier();
        
        if (id.isEmpty() || id.at(0).identifier().isEmpty()) {
            // id.isEmpty() not always hit when .toString() is actually empty...
            // anyhow, this makes sure that we don't show duchain items without
            // any name that could be searched for. This happens e.g. in the c++
            // plugin for anonymous structs or sometimes for declarations in macro
            // expressions
            continue;
        }
        m_currentItems << CodeModelViewItem(u, id);
      }
    }
  }

  m_filteredItems = m_currentItems;
  m_currentFilter.clear();
}

uint ProjectItemDataProvider::itemCount() const {
  uint add = 0;
  for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it)
    add += it.value().count();
  return m_filteredItems.count() + add;
}

QStringList ProjectItemDataProvider::supportedItemTypes() {
  QStringList ret;
  ret << i18n("Classes");
  ret << i18n("Functions");
  return ret;
}

void ProjectItemDataProvider::enableData( const QStringList& items, const QStringList& scopes ) {
  if( scopes.contains(i18n("Project")) ) {
    m_itemTypes = NoItems;
    if( items.contains( i18n("Classes") ) )
      m_itemTypes = (ItemTypes)(m_itemTypes | Classes);
    if( items.contains( i18n("Functions") ) )
      m_itemTypes = (ItemTypes)(m_itemTypes | Functions);

  } else {
    m_itemTypes = NoItems;
  }
}
