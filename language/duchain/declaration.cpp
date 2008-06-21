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
#include "declaration_p.h"

#include <QByteArray>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include <hashedstring.h>
#include <limits>

#include "topducontext.h"
#include "use.h"
#include "symboltable.h"
#include "forwarddeclaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "identifiedtype.h"
#include "ducontext_p.h"
#include "use_p.h"
#include "declarationid.h"
#include "definitions.h"
#include "uses.h"

using namespace KTextEditor;

namespace KDevelop
{

DeclarationPrivate::DeclarationPrivate() 
  : m_isDefinition(false), m_inSymbolTable(false),  
    m_isTypeAlias(false), m_anonymousInContext(false) 
{
  m_internalContext = 0;
  m_context = 0;
  m_kind = Declaration::Instance;
}
  
  
DeclarationPrivate::DeclarationPrivate( const DeclarationPrivate& rhs ) : DUChainBasePrivate(rhs)
{
  m_identifier = rhs.m_identifier;
  m_type = rhs.m_type;
  m_kind = rhs.m_kind;
  m_isDefinition = rhs.m_isDefinition;
  m_isTypeAlias = rhs.m_isTypeAlias;
  m_inSymbolTable = false;
  m_context = 0;
  m_internalContext = 0;
  m_comment = rhs.m_comment;
  m_anonymousInContext = rhs.m_anonymousInContext;
}
  
Declaration::Kind Declaration::kind() const {
  Q_D(const Declaration);
  return d->m_kind;
}

void Declaration::setKind(Kind kind) {
  Q_D(Declaration);
  d->m_kind = kind;
}

bool Declaration::inDUChain() const {
  Q_D(const Declaration);
  if( d->m_anonymousInContext )
    return false;
  TopDUContext* top = topContext();
  return top && top->inDuChain();
}

Declaration::Declaration( const HashedString& url, const SimpleRange& range, DUContext* context )
  : DUChainBase(*new DeclarationPrivate, url, range)
{
  if(context)
    setContext(context);
}

Declaration::Declaration(const Declaration& rhs) 
  : DUChainBase(*new DeclarationPrivate( *rhs.d_func() )) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

Declaration::Declaration( DeclarationPrivate & dd ) : DUChainBase(dd)
{
}

Declaration::Declaration( DeclarationPrivate & dd, const HashedString& url, const SimpleRange& range )
  : DUChainBase(dd, url, range)
{
//  Q_D(Declaration);
}

Declaration::~Declaration()
{
  Q_D(Declaration);
  // Inserted by the builder after construction has finished.
  if( d->m_internalContext )
    d->m_internalContext->setOwner(0);
  
  if (d->m_inSymbolTable)
    SymbolTable::self()->removeDeclaration(this);

  // context is only null in the test cases
  if (context())
    context()->d_func()->removeDeclaration(this);

  setContext(0);

  if( IdentifiedType* type = dynamic_cast<IdentifiedType*>(d->m_type.data()) )
    if( type->declaration() == this )
      type->setDeclaration(0);

  setAbstractType(AbstractType::Ptr());
  //DUChain::declarationChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

QByteArray Declaration::comment() const {
  Q_D(const Declaration);
  return d->m_comment;
}

void Declaration::setComment(const QByteArray& str) {
  Q_D(Declaration);
  d->m_comment = str;
}

void Declaration::setComment(const QString& str) {
  Q_D(Declaration);
  d->m_comment = str.toUtf8();
}

const Identifier& Declaration::identifier( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  bool wasInSymbolTable = d->m_inSymbolTable;
  
  setInSymbolTable(false);
    
  if( d->m_context && !d->m_anonymousInContext )
    d->m_context->changingIdentifier( this, d->m_identifier, identifier );

  d->m_identifier = identifier;
  
  setInSymbolTable(wasInSymbolTable);
  //DUChain::declarationChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DataType);

  d->m_type = type;

  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DataType);
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  ENSURE_CAN_READ
  
  QualifiedIdentifier ret;
  DUContext* ctx = d_func()->m_context;
  if(ctx)
    ret = ctx->scopeIdentifier(true);
  ret.push(d_func()->m_identifier);
  return ret;
}

// QString Declaration::mangledIdentifier() const
// {
//   //GNU mangling specs from http://theory.uwinnipeg.ca/gnu/gcc/gxxint_15.html
// 
//   if (abstractType())
//     return abstractType()->mangled();
// 
//   // Error...
//   return qualifiedIdentifier().mangled();
// }

DUContext * Declaration::context() const
{
  //ENSURE_CAN_READ Commented out for performance reasons
  return d_func()->m_context;
}

void Declaration::setContext(DUContext* context, bool anonymous)
{
  ENSURE_CAN_WRITE

  setInSymbolTable(false);

  Q_D(Declaration);
  if (d->m_context && context)
    Q_ASSERT(d->m_context->topContext() == context->topContext());

  if (d->m_context) {
    if( !d->m_anonymousInContext ) {
      d->m_context->d_func()->removeDeclaration(this);// if( )
        //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Context, d->m_context);
    }
  }

  d->m_context = context;
  d->m_anonymousInContext = anonymous;

  if (d->m_context) {
    if(!d->m_anonymousInContext) {
      d->m_context->d_func()->addDeclaration(this);
      //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Context, d->m_context);
    }
  }

  if(context && context->inSymbolTable() && !anonymous)
    setInSymbolTable(true);
}

const Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) const {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    const ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

DUContext * Declaration::logicalInternalContext(const TopDUContext* topContext) const {
  ENSURE_CAN_READ

  if(!isDefinition()) {
    Declaration* def = definition();
    if( def )
      return def->internalContext();
  }

  if( d_func()->m_isTypeAlias ) {
    ///If this is a type-alias, return the internal context of the actual type.
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(abstractType().data());
    if( idType && idType->declaration() && idType->declaration() != this )
      return idType->declaration()->logicalInternalContext( topContext );
  }
  
  return internalContext();
}

DUContext * Declaration::internalContext() const
{
  ENSURE_CAN_READ
  return d_func()->m_internalContext;
}

void Declaration::setInternalContext(DUContext* context)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);

  if( context == d->m_internalContext )
    return;

  DUContext* oldInternalContext = d->m_internalContext;
  
  d->m_internalContext = context;

  //Q_ASSERT( !oldInternalContext || oldInternalContext->owner() == this );
  if( oldInternalContext && oldInternalContext->owner() == this )
    oldInternalContext->setOwner(0);
  

  if( d->m_internalContext )
    d->m_internalContext->setOwner(this);
}


bool Declaration::operator ==(const Declaration & other) const
{
  ENSURE_CAN_READ

  return this == &other;
}

QString Declaration::toString() const
{
  return QString("%3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);

  return d->m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  d->m_isDefinition = dd;
//   if (d->m_isDefinition && definition()) {
//     setDefinition(0);
//   }
}

///@todo see whether it would be useful to create an own TypeAliasDeclaration sub-class for this
bool Declaration::isTypeAlias() const {
  Q_D(const Declaration);
  return d->m_isTypeAlias;
}

void Declaration::setIsTypeAlias(bool isTypeAlias) {
  Q_D(Declaration);
  d->m_isTypeAlias = isTypeAlias;
}

DeclarationId Declaration::id() const
{
  ENSURE_CAN_READ
  return DeclarationId(qualifiedIdentifier(), additionalIdentity());
}

Declaration* Declaration::declaration(TopDUContext* topContext) const
{
  ENSURE_CAN_READ
  
  if(!isDefinition())
    return 0;
  
  return DUChain::definitions()->declaration(const_cast<Declaration*>(this), topContext ? topContext : this->topContext());
}

Declaration* Declaration::definition() const
{
  ENSURE_CAN_READ
  if(isDefinition())
    return 0;
  
  return DUChain::definitions()->definition(id());
}

void Declaration::setDefinition(Declaration* definition)
{
  ENSURE_CAN_WRITE

  DUChain::definitions()->setDefinition(id(), definition);
}

bool Declaration::inSymbolTable() const
{
  Q_D(const Declaration);
  return d->m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  Q_D(Declaration);
  if(!d->m_identifier.isEmpty()) {
    if(!d->m_inSymbolTable && inSymbolTable)
      SymbolTable::self()->addDeclaration(this);
  
    else if(d->m_inSymbolTable && !inSymbolTable)
      SymbolTable::self()->removeDeclaration(this);
  }
  d->m_inSymbolTable = inSymbolTable;
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
  Q_D(const Declaration);
  if (d->m_context)
      return d->m_context->topContext();

    return 0;
}

Declaration* Declaration::clone() const  {
  return new Declaration(*this);
}

bool Declaration::isForwardDeclaration() const
{
  return false;
}

bool Declaration::isFunctionDeclaration() const
{
  return false;
}

uint Declaration::additionalIdentity() const
{
  return 0;
}

bool Declaration::equalQualifiedIdentifier(const Declaration* rhs) const {
  ENSURE_CAN_READ
  Q_D(const Declaration);
  if(d->m_identifier != rhs->d_func()->m_identifier)
    return false;
  
  return d->m_context->equalScopeIdentifier(d->m_context);
}

QList<KTextEditor::SmartRange*> Declaration::smartUses() const
{
  ENSURE_CAN_READ
  QSet<KTextEditor::SmartRange*> tempUses;

  //First, search for uses within the own context
  {
    foreach(KTextEditor::SmartRange* range, allSmartUses(topContext(), const_cast<Declaration*>(this)))
      tempUses.insert(range);
  }

  QList<TopDUContext*> useContexts = DUChain::uses()->uses(id());

  foreach(TopDUContext* context, useContexts) {
    foreach(KTextEditor::SmartRange* range, allSmartUses(context, const_cast<Declaration*>(this)))
      tempUses.insert(range);
  }

  return tempUses.toList();
}

QMap<HashedString, QList<SimpleRange> > Declaration::uses() const
{
  ENSURE_CAN_READ
  QMap<HashedString, QMap<SimpleRange, bool> > tempUses;

  //First, search for uses within the own context
  {
    QMap<SimpleRange, bool>& ranges(tempUses[topContext()->url()]);
    foreach(const SimpleRange& range, allUses(topContext(), const_cast<Declaration*>(this)))
      ranges[range] = true;
  }

  QList<TopDUContext*> useContexts = DUChain::uses()->uses(id());

  foreach(TopDUContext* context, useContexts) {
    QMap<SimpleRange, bool>& ranges(tempUses[context->url()]);
    foreach(const SimpleRange& range, allUses(context, const_cast<Declaration*>(this)))
      ranges[range] = true;
  }

  QMap<HashedString, QList<SimpleRange> > ret;

  for(QMap<HashedString, QMap<SimpleRange, bool> >::const_iterator it = tempUses.begin(); it != tempUses.end(); ++it) {
    if(!(*it).isEmpty()) {
      QList<SimpleRange>& list(ret[it.key()]);
      for(QMap<SimpleRange, bool>::const_iterator it2 = (*it).begin(); it2 != (*it).end(); ++it2)
        list << it2.key();
    }
  }
  return ret;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on


