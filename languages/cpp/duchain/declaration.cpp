/* This file is part of KDevelop
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

#include "declaration.h"

#include <QReadLocker>
#include <QWriteLocker>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "topducontext.h"
#include "use.h"
#include "definition.h"
#include "cpptypes.h"
#include "symboltable.h"
#include "forwarddeclaration.h"
#include "duchain.h"

using namespace KTextEditor;

Declaration::Declaration(KTextEditor::Range* range, Scope scope, DUContext* context )
  : DUChainBase(context->topContext())
  , KDevDocumentRangeObject(range)
  , m_context(0)
  , m_scope(scope)
  , m_definition(0)
  , m_isDefinition(false)
  , m_inSymbolTable(false)
{
  Q_ASSERT(context);
  setContext(context);
}

Declaration::~Declaration()
{
  // Inserted by the builder after construction has finished.
  if (m_inSymbolTable)
    SymbolTable::self()->removeDeclaration(this);

  foreach (ForwardDeclaration* forward, m_forwardDeclarations)
    forward->setResolved(0);
  Q_ASSERT(m_forwardDeclarations.isEmpty());

  QList<Use*> _uses = uses();
  foreach (Use* use, _uses)
    use->setDeclaration(0);

  if (Definition* def = definition())
    def->setDeclaration(0);

  // context is only null in the test cases
  if (context())
    context()->removeDeclaration(this);

  setContext(0);

  setAbstractType(AbstractType::Ptr());

  DUChain::declarationChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

void Declaration::removeUse( Use* use )
{
  ENSURE_CHAIN_WRITE_LOCKED

  use->setDeclaration(0L);
  m_uses.removeAll(use);

  DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

void Declaration::addUse( Use* use )
{
  ENSURE_CHAIN_WRITE_LOCKED

  use->setDeclaration(this);
  m_uses.append(use);

  DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

const QList< Use* > & Declaration::uses( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_uses;
}

const Identifier& Declaration::identifier( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_identifier = identifier;

  DUChain::declarationChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (CppIdentifiedType* idType = dynamic_cast<CppIdentifiedType*>(m_type.data()))
    idType->setDeclaration(0);

  if (m_type)
    DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DataType);

  m_type = type;

  if (CppIdentifiedType* idType = dynamic_cast<CppIdentifiedType*>(m_type.data()))
    idType->setDeclaration(this);

  if (m_type)
    DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DataType);
}

Declaration::Scope Declaration::scope( ) const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_scope;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  ENSURE_CHAIN_READ_LOCKED

  QualifiedIdentifier ret = context()->scopeIdentifier(true);
  ret.push(identifier());
  return ret;
}

QString Declaration::mangledIdentifier() const
{
  //GNU mangling specs from http://theory.uwinnipeg.ca/gnu/gcc/gxxint_15.html

  if (abstractType())
    return abstractType()->mangled();

  // Error...
  return qualifiedIdentifier().mangled();
}

DUContext * Declaration::context() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_context;
}

void Declaration::setContext(DUContext* context)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_context && context)
    Q_ASSERT(m_context->topContext() == context->topContext());

  if (m_context) {
    m_context->removeDeclaration(this);
    DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Context, m_context);
  }

  m_context = context;

  if (m_context) {
    m_context->addDeclaration(this);
    DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Context, m_context);
  }
}

bool Declaration::operator ==(const Declaration & other) const
{
  ENSURE_CHAIN_READ_LOCKED

  return this == &other;
}

QString Declaration::toString() const
{
  return QString("Declaration: %3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  ENSURE_CHAIN_WRITE_LOCKED

  m_isDefinition = dd;
  if (m_isDefinition && definition()) {
    setDefinition(0);
  }
}

Definition* Declaration::definition() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_definition;
}

void Declaration::setDefinition(Definition* definition)
{
  ENSURE_CHAIN_WRITE_LOCKED

  if (m_definition) {
    m_definition->setDeclaration(0);

    DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DefinitionRelationship, m_definition);
  }

  m_definition = definition;

  if (m_definition) {
    m_definition->setDeclaration(this);
    m_isDefinition = false;

    DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DefinitionRelationship, m_definition);
  }
}

bool Declaration::inSymbolTable() const
{
  return m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  m_inSymbolTable = inSymbolTable;
}

const QList< ForwardDeclaration * > & Declaration::forwardDeclarations() const
{
  ENSURE_CHAIN_READ_LOCKED

  return m_forwardDeclarations;
}

bool Declaration::isForwardDeclaration() const
{
  return false;
}

ForwardDeclaration* Declaration::toForwardDeclaration()
{
  return static_cast<ForwardDeclaration*>(this);
}

const ForwardDeclaration* Declaration::toForwardDeclaration() const
{
  return static_cast<const ForwardDeclaration*>(this);
}
