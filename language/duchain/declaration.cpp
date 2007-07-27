/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "topducontext.h"
#include "use.h"
#include "definition.h"
#include "symboltable.h"
#include "forwarddeclaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "identifiedtype.h"
#include "definition_p.h"
#include "ducontext_p.h"
#include "use_p.h"

#define ENSURE_CAN_WRITE {if( inDUChain()) { ENSURE_CHAIN_WRITE_LOCKED }}
#define ENSURE_CAN_READ { if( inDUChain() ) { ENSURE_CHAIN_READ_LOCKED }}

using namespace KTextEditor;

namespace KDevelop
{

class DeclarationPrivate
{
public:
  DeclarationPrivate() : m_isDefinition(false), m_inSymbolTable(false),  m_isTypeAlias(false), m_anonymousInContext(false) {
  }
  
  DUContext* m_context;
  DUContext* m_internalContext;
  Declaration::Scope m_scope;
  AbstractType::Ptr m_type;
  Identifier m_identifier;
  
  QString m_comment;

  QList<ForwardDeclaration*> m_forwardDeclarations;

  Definition* m_definition;

  QList<Use*> m_uses;

  Declaration::Kind m_kind;

  bool m_isDefinition  : 1;
  bool m_inSymbolTable : 1;
  bool m_isTypeAlias   : 1;
  bool m_anonymousInContext : 1; //Whether the declaration was added into the parent-context anonymously
};

Declaration::Kind Declaration::kind() const {
  return d->m_kind;
}

void Declaration::setKind(Kind kind) {
  d->m_kind = kind;
}

bool Declaration::inDUChain() const {
  if( d->m_anonymousInContext )
    return false;
  TopDUContext* top = topContext();
  return top && top->inDuChain();
}

Declaration::Declaration(KTextEditor::Range* range, Scope scope, DUContext* context )
  : DUChainBase(range)
  , d(new DeclarationPrivate)
{
  d->m_context = 0;
  d->m_internalContext = 0;
  d->m_scope = scope;
  d->m_definition = 0;
  d->m_kind = Instance;
  Q_ASSERT(context);
  setContext(context);
}

Declaration::Declaration(const Declaration& rhs) : DUChainBase(0), d(new DeclarationPrivate) {
  setTextRange(rhs.textRangePtr(), DocumentRangeObject::DontOwn);
  d->m_identifier = rhs.d->m_identifier;
  d->m_type = rhs.d->m_type;
  d->m_scope = rhs.d->m_scope;
  d->m_kind = rhs.d->m_kind;
  d->m_isDefinition = rhs.d->m_isDefinition;
  d->m_isTypeAlias = rhs.d->m_isTypeAlias;
  d->m_inSymbolTable = false;
  d->m_context = 0;
  d->m_internalContext = 0;
  d->m_definition = 0;
}

Declaration::~Declaration()
{
  if( d->m_internalContext )
    d->m_internalContext->setDeclaration(0);
  
  // Inserted by the builder after construction has finished.
  if (d->m_inSymbolTable)
    SymbolTable::self()->removeDeclaration(this);

  foreach (ForwardDeclaration* forward, d->m_forwardDeclarations)
    forward->setResolved(0);
  Q_ASSERT(d->m_forwardDeclarations.isEmpty());

  QList<Use*> _uses = uses();
  foreach (Use* use, _uses)
    use->d->setDeclaration(0);

  if (Definition* def = definition())
    def->d->setDeclaration(0);

  // context is only null in the test cases
  if (context())
    context()->d->removeDeclaration(this);

  setContext(0);

  if( IdentifiedType* type = dynamic_cast<IdentifiedType*>(d->m_type.data()) )
    if( type->declaration() == this )
      type->setDeclaration(0);

  setAbstractType(AbstractType::Ptr());

  DUChain::declarationChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
  delete d;
}

QString Declaration::comment() const {
  return d->m_comment;
}

void Declaration::setComment(const QString& str) {
  d->m_comment = str;
}

void Declaration::removeUse( Use* use )
{
  ENSURE_CAN_WRITE

  use->d->setDeclaration(0L);
  d->m_uses.removeAll(use);

  DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

void Declaration::addUse( Use* use )
{
  ENSURE_CAN_WRITE

  use->d->setDeclaration(this);
  d->m_uses.append(use);

  DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

const QList< Use* > & Declaration::uses( ) const
{
  ENSURE_CAN_READ

  return d->m_uses;
}

const Identifier& Declaration::identifier( ) const
{
  ENSURE_CAN_READ

  return d->m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CAN_WRITE

  d->m_identifier = identifier;

  DUChain::declarationChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  ENSURE_CAN_READ

  return d->m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CAN_WRITE

  if (d->m_type)
    DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DataType);

  d->m_type = type;

  if (d->m_type)
    DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DataType);
}

Declaration::Scope Declaration::scope( ) const
{
  ENSURE_CAN_READ

  return d->m_scope;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  ENSURE_CAN_READ

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
  ENSURE_CAN_READ

  return d->m_context;
}

void Declaration::setContext(DUContext* context, bool anonymous)
{
  ENSURE_CAN_WRITE

  if (d->m_context && context)
    Q_ASSERT(d->m_context->topContext() == context->topContext());

  if (d->m_context) {
    if( d->m_context->d->removeDeclaration(this) )
      DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Context, d->m_context);
  }

  d->m_context = context;
  d->m_anonymousInContext = anonymous;

  if (d->m_context) {
    if(!anonymous) {
      d->m_context->d->addDeclaration(this);
      DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Context, d->m_context);
    }
  }
}

DUContext * Declaration::internalContext() const
{
  ENSURE_CAN_READ
  if( isForwardDeclaration() ) {
    Declaration* declaration = static_cast<const KDevelop::ForwardDeclaration*>(this)->resolved();
    if( !declaration )
      return 0;
    else
      return declaration->internalContext();
  }

  return d->m_internalContext;
}

void Declaration::setInternalContext(DUContext* context)
{
  ENSURE_CAN_WRITE
  
  if( context == d->m_internalContext )
    return;

  DUContext* oldInternalContext = d->m_internalContext;
  
  d->m_internalContext = context;
  
  if( oldInternalContext && oldInternalContext->declaration() == this )
    oldInternalContext->setDeclaration(0);
  

  if( d->m_internalContext )
    d->m_internalContext->setDeclaration(this);
}

bool Declaration::operator ==(const Declaration & other) const
{
  ENSURE_CAN_READ

  return this == &other;
}

QString Declaration::toString() const
{
  return QString("Declaration: %3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  ENSURE_CAN_READ

  return d->m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  ENSURE_CAN_WRITE

  d->m_isDefinition = dd;
  if (d->m_isDefinition && definition()) {
    setDefinition(0);
  }
}

///@todo see whether it would be useful to create an own TypeAliasDeclaration sub-class for this
bool Declaration::isTypeAlias() const {
  return d->m_isTypeAlias;
}

void Declaration::setIsTypeAlias(bool isTypeAlias) {
  d->m_isTypeAlias = isTypeAlias;
}

Definition* Declaration::definition() const
{
  ENSURE_CAN_READ

  return d->m_definition;
}

void Declaration::setDefinition(Definition* definition)
{
  ENSURE_CAN_WRITE

  if (d->m_definition) {
    d->m_definition->d->setDeclaration(0);

    DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DefinitionRelationship, d->m_definition);
  }

  d->m_definition = definition;

  if (d->m_definition) {
    d->m_definition->d->setDeclaration(this);
    d->m_isDefinition = false;

    DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DefinitionRelationship, d->m_definition);
  }
}

bool Declaration::inSymbolTable() const
{
  return d->m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  d->m_inSymbolTable = inSymbolTable;
}

const QList< ForwardDeclaration * > & Declaration::forwardDeclarations() const
{
  ENSURE_CAN_READ

  return d->m_forwardDeclarations;
}

void Declaration::addForwardDeclaration( ForwardDeclaration* declaration)
{
  ENSURE_CAN_WRITE

  d->m_forwardDeclarations.append( declaration );
}

void Declaration::removeForwardDeclaration( ForwardDeclaration* declaration)
{
  ENSURE_CAN_WRITE

  d->m_forwardDeclarations.removeAll( declaration );
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

TopDUContext * Declaration::topContext() const
{
  if (d->m_context)
    return d->m_context->topContext();

  return 0;
}

Declaration* Declaration::clone() const  {
  return new Declaration(*this);
}

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
