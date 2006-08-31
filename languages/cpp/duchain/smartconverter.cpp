/*
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
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

#include <kdeveditorintegrator.h>

#include "ducontext.h"
#include "declaration.h"
#include "definition.h"
#include "use.h"

using namespace KTextEditor;

SmartConverter::SmartConverter(KDevEditorIntegrator* editor)
  : m_editor(editor)
{
}

void SmartConverter::convertDUChain(DUContext* context) const
{
  m_editor->setCurrentUrl(context->url());

  if (m_editor->smart()) {
    context->setTextRange(m_editor->topRange(KDevEditorIntegrator::DefinitionUseChain));

    convertDUChainInternal(context, true);
  }
}

void SmartConverter::convertDUChainInternal(DUContext* context, bool first) const
{
  if (!first)
    context->setTextRange(m_editor->createRange(context->textRange()));

  foreach (Declaration* dec, context->localDeclarations()) {
    dec->setTextRange(m_editor->createRange(dec->textRange()));
    m_editor->exitCurrentRange();
  }

  foreach (Definition* def, context->localDefinitions()) {
    def->setTextRange(m_editor->createRange(def->textRange()));
    m_editor->exitCurrentRange();
  }

  foreach (Use* use, context->uses()) {
    use->setTextRange(m_editor->createRange(use->textRange()));
    m_editor->exitCurrentRange();
  }

  foreach (DUContext::UsingNS* usingNS, context->usingNamespaces())
    usingNS->setTextCursor(m_editor->createCursor(usingNS->textCursor()));

  foreach (DUContext* child, context->childContexts())
    convertDUChainInternal(child);

  m_editor->exitCurrentRange();
}

// kate: space-indent on; indent-width 2; replace-tabs on
