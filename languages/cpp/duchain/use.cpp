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

#include "use.h"

#include <QWriteLocker>

#include "declaration.h"
#include "ducontext.h"


#include "topducontext.h"
#define ENSURE_CHAIN_READ_LOCKED \
if (!topContext()->deleting()) { \
  bool _ensure_chain_locked = chainLock()->tryLockForWrite(); \
  Q_ASSERT(!_ensure_chain_locked); \
}
#define ENSURE_CHAIN_WRITE_LOCKED \
if (!topContext()->deleting()) { \
  bool _ensure_chain_locked = chainLock()->tryLockForWrite(); \
  Q_ASSERT(!_ensure_chain_locked); \
}

using namespace KTextEditor;

Use::Use(KTextEditor::Range* range, DUContext* context)
  : DUChainBase(context->topContext())
  , KDevDocumentRangeObject(range)
  , m_context(0)
  , m_declaration(0)
  , m_isInternal(true)
{
  if (context)
    setContext(context);
}

Use::~Use()
{
  setContext(0);
}

Declaration* Use::declaration() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_declaration;
}

void Use::setDeclaration(Declaration* declaration)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_declaration)
    // Or otherwise, you could implement internal <--> external use switching :)
    Q_ASSERT(declaration->context()->topContext() == m_declaration->context()->topContext());

  m_declaration = declaration;
}

void Use::setContext(DUContext * context)
{
  if (m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    if (m_isInternal)
      m_context->removeInternalUse(this);
    else
      m_context->removeExternalUse(this);
  }

  m_context = context;

  if (m_context) {
    ENSURE_CHAIN_WRITE_LOCKED

    m_isInternal = !m_declaration || m_context->topContext() == m_declaration->context()->topContext();
    if (m_isInternal)
      m_context->addInternalUse(this);
    else
      m_context->addExternalUse(this);
  }
}

DUContext * Use::context() const
{
  // don't check lock or we recurse

  return m_context;
}

bool Use::isOrphan() const
{
  ENSURE_CHAIN_READ_LOCKED

  return !m_declaration;
}

// kate: indent-width 2;
