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

#include <editorintegrator.h>
#include "icodehighlighting.h"

#include "ducontext.h"
#include "declaration.h"
#include "definition.h"
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
      context->setTextRange(m_editor->createRange(context->textRange()));

    foreach (Declaration* dec, context->localDeclarations()) {
      dec->setTextRange(m_editor->createRange(dec->textRange()));
      if( m_hl )
        m_hl->highlightDeclaration(dec);
      m_editor->exitCurrentRange();
    }

    foreach (Definition* def, context->localDefinitions()) {
      def->setTextRange(m_editor->createRange(def->textRange()));
      if( m_hl )
        m_hl->highlightDefinition(def);
      m_editor->exitCurrentRange();
    }

    foreach (Use* use, context->uses()) {
      use->setTextRange(m_editor->createRange(use->textRange()));
      if( m_hl )
        m_hl->highlightUse(use);
      m_editor->exitCurrentRange();
    }

    foreach (DUContext* child, context->childContexts())
      convertDUChainInternal(child);

    m_editor->exitCurrentRange();
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

  d->m_editor->setCurrentUrl(context->url());

  if (d->m_editor->smart() && !context->smartRange()) {
    context->setTextRange(d->m_editor->topRange(KDevelop::EditorIntegrator::DefinitionUseChain));

    d->convertDUChainInternal(context, true);
  }
}

void SmartConverter::unconvertDUChain(DUContext* context) const
{
  DUChainWriteLocker lock(DUChain::lock());

  Q_ASSERT(!d->m_hl);
  d->m_editor->setCurrentUrl(context->url());

  if (d->m_editor->smart() && !context->smartRange()) {
    context->setTextRange(d->m_editor->topRange(KDevelop::EditorIntegrator::DefinitionUseChain));

    d->convertDUChainInternal(context, true);
  }
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
