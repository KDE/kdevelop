/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "basecontextbuilder.h"

#include <climits>

#include <QtCore/QMutexLocker>

#include <ktexteditor/smartinterface.h>
#include <ktexteditor/range.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include <editor/editorintegrator.h>

#include "smartconverter.h"
#include "duchainlock.h"
#include "duchain.h"
#include "ducontext.h"
#include "identifier.h"
#include "symboltable.h"
#include "parsingenvironment.h"

namespace KDevelop
{

class BaseContextBuilderPrivate
{
public:
  BaseContextBuilderPrivate( EditorIntegrator* editor, bool ownsEditor = true )
    : m_editor( editor )
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_ownsEditorIntegrator( ownsEditor )
    , m_lastContext( 0 )
  {
  }

  EditorIntegrator* m_editor;
  bool m_compilingContexts : 1;
  bool m_recompiling : 1;
  bool m_ownsEditorIntegrator: 1;
  QStack<int> m_nextContextStack;
  DUContext* m_lastContext;
  QList<DUContext*> m_importedParentContexts;
  QSet<KDevelop::DUChainBase*> m_encountered;
  QStack<KDevelop::DUContext*> m_contextStack;
  Identifier m_identifier;
  QualifiedIdentifier m_qIdentifier;

  bool recompiling()
  {
    return m_recompiling;
  }

  DUContext * currentContext()
  {
    return m_contextStack.top();
  }

  void openContext( DUContext* newContext )
  {
    m_contextStack.push( newContext );
    m_nextContextStack.push( 0 );
  }
  int& nextContextIndex()
  {
    return m_nextContextStack.top();
  }

  DUContext* openContextInternal( const SimpleRange& range, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    kDebug() << "OpenContextInternal";
    Q_ASSERT( m_compilingContexts );
    DUContext* ret = 0L;
    {
      DUChainReadLocker readLock( DUChain::lock() );

      if ( recompiling() )
      {
        const QVector<DUContext*>& childContexts = currentContext()->childContexts();
        QMutexLocker lock( m_editor->smart() ? m_editor->smart()->smartMutex() : 0 );
        SimpleRange translated = range;

        if ( m_editor->smart() )
          translated = SimpleRange( m_editor->smart()->translateFromRevision( translated.textRange() ) );

        if ( !childContexts.count() )
          kDebug() << "------No Child Contexts while Recompiling-----";

        for ( ; nextContextIndex() < childContexts.count(); ++nextContextIndex() )
        {
          DUContext* child = childContexts.at( nextContextIndex() );

          if ( child->range().start > translated.end && child->smartRange() )
              break;

          if ( child->type() == type && child->localScopeIdentifier() == identifier && child->range() == translated )
          {
            ret = child;
            readLock.unlock();
            DUChainWriteLocker writeLock( DUChain::lock() );

            ret->clearImportedParentContexts();
            m_editor->setCurrentRange( ret->smartRange() );
            break;
          }
        }
      }

      if ( !ret )
      {
        readLock.unlock();
        DUChainWriteLocker writeLock( DUChain::lock() );

        if ( !currentContext() )
          kDebug() << "Current Context is Empty, need to Create a New One";

        ret = new DUContext( m_editor->currentUrl(), SimpleRange( range ), m_contextStack.isEmpty() ? 0 : currentContext() );

        ret->setSmartRange( m_editor->createRange( range.textRange() ), DocumentRangeObject::Own );

        ret->setType( type );

        if ( !identifier.isEmpty() )
        {
          ret->setLocalScopeIdentifier( identifier );

          if ( type == DUContext::Class )
            SymbolTable::self()->addContext( ret );
        }
      }
    }

    m_encountered.insert( ret );
    openContext( ret );
    return ret;
  }
};

template <typename T>
BaseContextBuilder<T>::BaseContextBuilder()
  : d( new BaseContextBuilderPrivate( new EditorIntegrator ) )
{
}

template <typename T>
BaseContextBuilder<T>::BaseContextBuilder( EditorIntegrator* editor )
  : d( new BaseContextBuilderPrivate( editor, false ) )
{
}

template <typename T>
BaseContextBuilder<T>::~BaseContextBuilder()
{
}

template <typename T>
TopDUContext BaseContextBuilder<T>::buildContexts( const KUrl& url, T* node,
    const TopDUContextPointer& updateContext )
{
  d->m_compilingContexts = true;
  d->m_editor->setCurrentUrl( KDevelop::HashedString( url.prettyUrl() ) );

  TopDUContext* top = 0;
  {
    DUChainWriteLocker lock( DUChain::lock() );
    top = updateContext.data();
    if( top && !top->smartRange() && d->m_editor->smart() )
    {
      lock.unlock();
      smartenContext( top );
      lock.lock();
      top = updateContext.data();
    }
    if( top && top->smartRange() )
    {
      if( top && top->smartRange()->parentRange() )
      {
        //somethings wrong, a top level range can't have a parent
        Q_ASSERT(false);
      }
    }
    if( top )
    {
      kDebug() << "re-compiling";
      d->m_recompiling = true;
      if( d->m_compilingContexts )
      {
        if( d->m_editor->currentDocument() && d->m_editor->smart() && top->range().textRange() != d->m_editor->currentDocument()->documentRange() )
        {
          kDebug() << "WARN: top level range changed";
          top->setRange( SimpleRange( d->m_editor->currentDocument()->documentRange() ) );
        }
      }
    }else
    {
      kDebug() << "compiling";
      top = new TopDUContext( d->m_editor->currentUrl(),
                              d->m_editor->currentDocument()
                                  ? SimpleRange( d->m_editor->currentDocument()->documentRange() )
                                  : SimpleRange( SimpleCursor( 0, 0 ), SimpleCursor( INT_MAX, INT_MAX ) ) );
      top->setSmartRange( d->m_editor->topRange( EditorIntegrator::DefinitionUseChain ), DocumentRangeObject::Own );
      top->setType( DUContext::Global );
      DUChain::self()->addDocumentChain( IdentifiedFile( url, 0 ), top );
    }
    setEncountered( top );
    setContextOnNode( node, top );
  }

  supportBuild( node );

  if( d->m_editor->currentDocument() && d->m_editor->smart() &&
      top->range().textRange() != d->m_editor->currentDocument()->documentRange() )
  {
    kDebug() << "WARNING: top level context has wrong size:" << top->range().textRange() << "should be:" << d->m_editor->currentDocument()->documentRange();
    top->setRange( d->m_editor->currentDocument()->documentRange() );
  }
  {
    DUChainReadLocker lock( DUChain::lock() );
    //foreach(DUContext* context, topLevelContext->childContexts());
    kDebug() << "built top-level context with" << top->localDeclarations().count() << "declarations," << top->localDeclarations().count() << " Definitions and" << top->childContexts().size() << "Child-Contexts";

    foreach( DUContext* contexts, top->childContexts() )
    {
      kDebug() << "CHILD:" << contexts->scopeIdentifier( true ) << "Parent:" << ( dynamic_cast<TopDUContext*>( contexts->parentContext() ) ? "top-context" : "" );
    }
  }
  d->m_compilingContexts = false;
  return top;
}

template <typename T>
DUContext* BaseContextBuilder<T>::buildSubContexts( const KUrl& url, T *node, DUContext* parent )
{
//     m_compilingContexts = true;
//     m_recompiling = false;
    d->m_editor->setCurrentUrl( HashedString( url.prettyUrl() ) );
    setContextOnNode( node, parent );
    {
        openContext( contextFromNode( node ) );
        d->m_editor->setCurrentRange( d->m_editor->topRange( EditorIntegrator::DefinitionUseChain ) );
        visitAstNode( node );
        closeContext();
    }

    d->m_compilingContexts = false;

    if ( contextFromNode( node ) == parent )
    {
        kDebug() << "Error in BaseContextBuilder::buildSubContexts(...): du-context was not replaced with new one";
        DUChainWriteLocker lock( DUChain::lock() );
        deleteContextOnNode( node );
    }

    return contextFromNode( node );
}

template <typename T>
void BaseContextBuilder<T>::deleteContextOnNode( T* node )
{
  delete contextFromNode( node );
  setContextFromNode( node, 0 );
}

template <typename T>
void BaseContextBuilder<T>::smartenContext( TopDUContext* topLevelContext )
{
  if ( topLevelContext && !topLevelContext->smartRange() && d->m_editor->smart() )
  {
    SmartConverter conv( d->m_editor, 0 );
    conv.convertDUChain( topLevelContext );
  }
}

template <typename T>
void BaseContextBuilder<T>::openContext( DUContext* newContext )
{
  d->openContext( newContext );
}

template <typename T>
DUContext * BaseContextBuilder<T>::currentContext()
{
    return d->m_contextStack.top();
}

template <typename T>
DUContext* BaseContextBuilder<T>::openContext( T* rangeNode, DUContext::ContextType type, T* identifier )
{
  if ( d->m_compilingContexts )
  {
    DUContext* ret = openContextInternal( editorFindRange( rangeNode ), type, identifier ? identifierForName( identifier ) : QualifiedIdentifier() );
    setContextOnNode( rangeNode, ret );
    return ret;
  }
  else
  {
    openContext( rangeNode->context );
    d->m_editor->setCurrentRange( currentContext()->smartRange() );
    return currentContext();
  }
}

template <typename T>
DUContext* BaseContextBuilder<T>::openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier )
{
  if ( d->m_compilingContexts )
  {
    //kDebug() << "Opening ContextInternal";
    DUContext* ret = openContextInternal( editorFindRange( rangeNode ), type, identifier );
    //kDebug() << "Associating context" ;
    setContextOnNode( rangeNode, ret );
    return ret;
  }
  else
  {
    //kDebug() << "Opening Context associated with node";
    openContext( rangeNode->context );
    d->m_editor->setCurrentRange( currentContext()->smartRange() );
    return currentContext();
  }
}

template <typename T>
DUContext* BaseContextBuilder<T>::openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier )
{
  if ( d->m_compilingContexts )
  {
    DUContext* ret = openContextInternal( editorFindRange( fromRange, toRange ), type, identifier );
    fromRange->context = ret;
    return ret;
  }
  else
  {
    openContext( fromRange->context );
    d->m_editor->setCurrentRange( currentContext()->smartRange() );
    return currentContext();
  }
}

template <typename T>
void BaseContextBuilder<T>::closeContext()
{
  {
    DUChainWriteLocker lock( DUChain::lock() );
    currentContext()->cleanIfNotEncountered( d->m_encountered, d->m_compilingContexts );
    setEncountered( currentContext() );
  }

  d->m_lastContext = currentContext();
  d->m_contextStack.pop();
  d->m_nextContextStack.pop();
  d->m_editor->exitCurrentRange();
}

template <typename T>
void BaseContextBuilder<T>::setEncountered( DUChainBase* item )
{
  d->m_encountered.insert( item );
}

template <typename T>
bool BaseContextBuilder<T>::wasEncountered( DUChainBase* item )
{
  return d->m_encountered.contains( item );
}

template <typename T>
void BaseContextBuilder<T>::setIdentifier( const QString & id )
{
  d->m_identifier = new Identifier( id );
  d->m_qIdentifier.clear();
  d->m_qIdentifier.push( d->m_identifier );
}

template <typename T>
QualifiedIdentifier BaseContextBuilder<T>::qualifiedIdentifier( )
{
  return d->m_qIdentifier;
}
template <typename T>
template <typename E>
E* BaseContextBuilder<T>::editor()
{
  E* editor = dynamic_cast<E*>( d->m_editor );
  if( !editor )
    Q_ASSERT_X(false, "editor cast", "Couldn't cast editor to proper return type");
  return editor;
}

}
