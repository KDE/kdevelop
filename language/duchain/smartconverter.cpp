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

#include <editorintegrator.h>
#include <hashedstring.h>
#include "icodehighlighting.h"

#include "ducontext.h"
#include "declaration.h"
#include "use.h"
#include "topducontext.h"
#include "duchain.h"
#include "duchainlock.h"

using namespace KTextEditor;

namespace KDevelop
{

class SmartConverterPrivate
{
public:
  void convertDUChainInternal(DUContext* context, bool first = false) const
  {
    if (!first)
      context->setSmartRange(m_editor->createRange(context->range().textRange())->toSmartRange());

    foreach (Declaration* dec, context->localDeclarations()) {
      dec->setSmartRange(m_editor->createRange(dec->range().textRange())->toSmartRange());
      if( m_hl )
        m_hl->highlightDeclaration(dec);
      m_editor->exitCurrentRange();
    }

    for(int a = 0; a < context->uses().count(); ++a) {
      context->setUseSmartRange(a, m_editor->createRange(context->uses()[a].m_range.textRange())->toSmartRange());
      m_editor->exitCurrentRange();
    }
    
    if( m_hl )
      m_hl->highlightUses(context);

    foreach (DUContext* child, context->childContexts())
      convertDUChainInternal(child);

    m_editor->exitCurrentRange();
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
  KDevelop::ICodeHighlighting* m_hl;
};

SmartConverter::SmartConverter(KDevelop::EditorIntegrator* editor, KDevelop::ICodeHighlighting* hl)
  : d(new SmartConverterPrivate)
{
  d->m_editor = editor;
  d->m_hl = hl;
}

SmartConverter::~SmartConverter()
{
  delete d;
}

void SmartConverter::convertDUChain(DUContext* context) const
{
  DUChainWriteLocker lock(DUChain::lock());

  d->m_editor->setCurrentUrl( context->url() );

  if (d->m_editor->smart() && !context->smartRange()) {
    QMutexLocker smartLock(d->m_editor->smart()->smartMutex());
    context->setSmartRange(d->m_editor->topRange(KDevelop::EditorIntegrator::DefinitionUseChain)->toSmartRange());
    if (context->range().textRange() != d->m_editor->currentDocument()->documentRange())
      kWarning() << "Context range to be converted" << context->range().textRange() << "does not match the document range" << d->m_editor->currentDocument()->documentRange();
    //Q_ASSERT(context->range().textRange() == d->m_editor->currentDocument()->documentRange());
    Q_ASSERT(context->smartRange() && !context->smartRange()->parentRange() && context->smartRange()->childRanges().isEmpty());

    d->convertDUChainInternal(context, true);
    d->m_editor->exitCurrentRange(); //topRange(..) opens a range
  }
}

void SmartConverter::deconvertDUChain(DUContext* context) const
{
  DUChainWriteLocker lock(DUChain::lock());

  d->m_editor->setCurrentUrl( context->url() );

  if (SmartInterface* smart = d->m_editor->smart()) {
    QMutexLocker smartLock(smart->smartMutex());
    d->deconvertDUChainInternal(context);
  }
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
