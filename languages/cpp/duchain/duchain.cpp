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

#include <QMutexLocker>

#include <kstaticdeleter.h>

#include "kdeveditorintegrator.h"

#include "topducontext.h"

static KStaticDeleter<DUChain> sd;

void DUChain::removeDocumentChain( const KUrl & document )
{
  m_chains.remove(document);
}

void DUChain::addDocumentChain( const KUrl & document, TopDUContext * chain )
{
  Q_ASSERT(chain);
  m_chains.insert(document, chain);
}

TopDUContext * DUChain::chainForDocument( const KUrl & document )
{
  if (m_chains.contains(document))
    return m_chains[document];
  return 0;
}

DUChain* DUChain::s_chain = 0;

DUChain * DUChain::self( )
{
  if (!s_chain)
    sd.setObject(s_chain, new DUChain());

  return s_chain;
}

void DUChain::clear()
{
  foreach (TopDUContext* context, m_chains) {
    KDevEditorIntegrator::releaseTopRange(context->textRangePtr());
    delete context;
  }

  m_chains.clear();
}

#include "duchain.moc"

// kate: indent-width 2;
