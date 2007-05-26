/* This  is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "duchain.h"
#include "duchainlock.h"

#include <kglobal.h>

#include "editorintegrator.h"

#include "topducontext.h"

class DUChainPrivate
{
public:
  DUChain instance;
  DUChainLock lock;
  QMap<KUrl, TopDUContext*> m_chains;
  QList<DUChainObserver*> m_observers;
};

K_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)


DUChain::DUChain()
{
}

DUChain::~DUChain()
{
}

DUChain* DUChain::self()
{
  return &sdDUChainPrivate->instance;
}

DUChainLock* DUChain::lock()
{
  return &sdDUChainPrivate->lock;
}

void DUChain::removeDocumentChain( const KUrl & document )
{
  sdDUChainPrivate->m_chains.remove(document);
}

void DUChain::addDocumentChain( const KUrl & document, TopDUContext * chain )
{
  Q_ASSERT(chain);
  sdDUChainPrivate->m_chains.insert(document, chain);
}

TopDUContext * DUChain::chainForDocument( const KUrl & document )
{
  if (sdDUChainPrivate->m_chains.contains(document))
    return sdDUChainPrivate->m_chains[document];
  return 0;
}

void DUChain::clear()
{
  DUChainWriteLocker writeLock(lock());
  foreach (TopDUContext* context, sdDUChainPrivate->m_chains) {
    KDevelop::EditorIntegrator::releaseTopRange(context->textRangePtr());
    delete context;
  }

  sdDUChainPrivate->m_chains.clear();
}

const QList< DUChainObserver * > & DUChain::observers() const
{
  ENSURE_CHAIN_READ_LOCKED

  return sdDUChainPrivate->m_observers;
}

void DUChain::addObserver(DUChainObserver * observer)
{
  ENSURE_CHAIN_WRITE_LOCKED

  Q_ASSERT(!sdDUChainPrivate->m_observers.contains(observer));
  sdDUChainPrivate->m_observers.append(observer);
}

void DUChain::removeObserver(DUChainObserver * observer)
{
  ENSURE_CHAIN_WRITE_LOCKED

  sdDUChainPrivate->m_observers.removeAll(observer);
}

void DUChain::contextChanged(DUContext* context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  foreach (DUChainObserver* observer, self()->observers())
    observer->contextChanged(context, change, relationship, relatedObject);
}

void DUChain::declarationChanged(Declaration* declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  foreach (DUChainObserver* observer, self()->observers())
    observer->declarationChanged(declaration, change, relationship, relatedObject);
}

void DUChain::definitionChanged(Definition* definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  foreach (DUChainObserver* observer, self()->observers())
    observer->definitionChanged(definition, change, relationship, relatedObject);
}

void DUChain::useChanged(Use* use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  foreach (DUChainObserver* observer, self()->observers())
    observer->useChanged(use, change, relationship, relatedObject);
}

#include "duchain.moc"

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
