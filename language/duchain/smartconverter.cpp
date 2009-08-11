/*
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "smartconverter.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <KTextEditor/SmartInterface>

#include "../editor/editorintegrator.h"
#include "../editor/hashedstring.h"

#include "ducontext.h"
#include "declaration.h"
#include "use.h"
#include "topducontext.h"
#include "duchain.h"
#include "duchainlock.h"
#include "indexedstring.h"

using namespace KTextEditor;

namespace KDevelop
{

class SmartConverterPrivate
{
public:
  void convertDUChainInternal(const LockedSmartInterface& iface, DUContext* context, bool first = false) const
  {
    if (!first)
      context->setSmartRange(m_editor->createRange(iface, context->range().textRange())->toSmartRange());

    foreach (Declaration* dec, context->localDeclarations()) {
      dec->setSmartRange(m_editor->createRange(iface, dec->range().textRange())->toSmartRange());
      m_editor->exitCurrentRange(iface);
    }

    for(int a = 0; a < context->usesCount(); ++a) {
      context->setUseSmartRange(a, m_editor->createRange(iface, context->uses()[a].m_range.textRange())->toSmartRange());
      m_editor->exitCurrentRange(iface);
    }

    foreach (DUContext* child, context->childContexts())
      convertDUChainInternal(iface, child);

    m_editor->exitCurrentRange(iface);
  }

  void deconvertDUChainInternal(DUContext* context) const
  {
    foreach (Declaration* dec, context->localDeclarations())
      dec->clearSmartRange();

    context->clearUseSmartRanges();

    foreach (DUContext* child, context->childContexts())
      deconvertDUChainInternal(child);

    context->clearSmartRange();
  }

  KDevelop::EditorIntegrator* m_editor;
};

SmartConverter::SmartConverter(KDevelop::EditorIntegrator* editor)
  : d(new SmartConverterPrivate)
{
  d->m_editor = editor;
}

SmartConverter::~SmartConverter()
{
  delete d;
}

void SmartConverter::convertDUChain(DUContext* context) const
{
  if(context->inDUChain()) {
    ENSURE_CHAIN_WRITE_LOCKED
  }

  d->m_editor->setCurrentUrl( context->url(), true );

  LockedSmartInterface iface  = d->m_editor->smart();
  Q_ASSERT(iface);
  if (iface && !context->smartRange()) {
    context->setSmartRange(d->m_editor->topRange(iface, KDevelop::EditorIntegrator::DefinitionUseChain)->toSmartRange());
    if (context->range().textRange() != iface.currentDocument()->documentRange()) {
      kWarning() << "Context range to be converted" << context->range().textRange() << "does not match the document range" << iface.currentDocument()->documentRange();
    }
    //Q_ASSERT(context->range().textRange() == iface.currentDocument()->documentRange());
    Q_ASSERT(context->smartRange() && !context->smartRange()->parentRange() && context->smartRange()->childRanges().isEmpty());

    d->convertDUChainInternal(iface, context, true);
    d->m_editor->exitCurrentRange(iface); //topRange(..) opens a range
  }
}

void SmartConverter::deconvertDUChain(DUContext* context) const
{
  if(context->inDUChain()) {
    ENSURE_CHAIN_WRITE_LOCKED
  }

  d->m_editor->setCurrentUrl( IndexedString(context->url().str()), true );

  LockedSmartInterface iface  = d->m_editor->smart();
  if (iface) {
    d->deconvertDUChainInternal(context);
  }
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
