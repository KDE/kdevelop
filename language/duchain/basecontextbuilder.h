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

#ifndef BASECONTEXTBUILDER_H
#define BASECONTEXTBUILDER_H

#include <climits>

#include <QtCore/QMutexLocker>

#include <ktexteditor/smartinterface.h>
#include <ktexteditor/range.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include <editor/editorintegrator.h>

#include "smartconverter.h"
#include "topducontext.h"
#include "duchainpointer.h"
#include "duchainlock.h"
#include "duchain.h"
#include "ducontext.h"
#include "identifier.h"
#include "symboltable.h"
#include "parsingenvironment.h"

class KUrl;

namespace KDevelop
{
class TopDUContext;
class DUContext;
class DUChainBase;

template<typename T>
class KDEVPLATFORMLANGUAGE_EXPORT BaseContextBuilder
{
public:
  BaseContextBuilder()
    : m_editor( new EditorIntegrator )
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_ownsEditorIntegrator( true )
    , m_lastContext( 0 )

  {
  }

  BaseContextBuilder( EditorIntegrator* editor )
    : m_editor( editor )
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_ownsEditorIntegrator( false )
    , m_lastContext( 0 )
  {
  }

  virtual ~BaseContextBuilder()
  {
  }

  

  TopDUContext buildContexts( const KUrl& url, T* node, 
                              const TopDUContextPointer& updateContext
                                    = TopDUContextPointer() )
  {
    m_compilingContexts = true;
    m_editor->setCurrentUrl( KDevelop::HashedString( url.prettyUrl() ) );
  
    TopDUContext* top = 0;
    {
      DUChainWriteLocker lock( DUChain::lock() );
      top = updateContext.data();
      if( top && !top->smartRange() && m_editor->smart() )
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
        m_recompiling = true;
        if( m_compilingContexts )
        {
          if( m_editor->currentDocument() && m_editor->smart() && top->range().textRange() != m_editor->currentDocument()->documentRange() )
          {
            kDebug() << "WARN: top level range changed";
            top->setRange( SimpleRange( m_editor->currentDocument()->documentRange() ) );
          }
        }
      }else
      {
        kDebug() << "compiling";
        top = new TopDUContext( m_editor->currentUrl(),
                                m_editor->currentDocument()
                                    ? SimpleRange( m_editor->currentDocument()->documentRange() )
                                    : SimpleRange( SimpleCursor( 0, 0 ), SimpleCursor( INT_MAX, INT_MAX ) ) );
        top->setSmartRange( m_editor->topRange( EditorIntegrator::DefinitionUseChain ), DocumentRangeObject::Own );
        top->setType( DUContext::Global );
        DUChain::self()->addDocumentChain( IdentifiedFile( url, 0 ), top );
      }
      setEncountered( top );
      setContextOnNode( node, top );
    }
  
    supportBuild( node );
  
    if( m_editor->currentDocument() && m_editor->smart() &&
        top->range().textRange() != m_editor->currentDocument()->documentRange() )
    {
      kDebug() << "WARNING: top level context has wrong size:" << top->range().textRange() << "should be:" << m_editor->currentDocument()->documentRange();
      top->setRange( m_editor->currentDocument()->documentRange() );
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
    m_compilingContexts = false;
    return top;
  }

protected:
  
  virtual void supportBuild( T* node ) = 0;
  virtual void setContextOnNode( T* node, KDevelop::DUContext* ctx ) = 0;
  virtual DUContext* contextFromNode( T* node ) = 0;
  virtual KTextEditor::Range editorFindRange( T* fromRange, T* toRange ) = 0;
  virtual const QualifiedIdentifier identifierForNode( T* ) = 0;

  void smartenContext( TopDUContext* topLevelContext )
  {
    if ( topLevelContext && !topLevelContext->smartRange() && m_editor->smart() )
    {
      SmartConverter conv( m_editor, 0 );
      conv.convertDUChain( topLevelContext );
    }
  }

  KDevelop::DUContext* buildSubContexts( const KUrl& url, T *node,
                                         KDevelop::DUContext* parent )
  {
  //     m_compilingContexts = true;
  //     m_recompiling = false;
      m_editor->setCurrentUrl( HashedString( url.prettyUrl() ) );
      setContextOnNode( node, parent );
      {
          openContext( contextFromNode( node ) );
          m_editor->setCurrentRange( m_editor->topRange( EditorIntegrator::DefinitionUseChain ) );
          visitAstNode( node );
          closeContext();
      }
  
      m_compilingContexts = false;
  
      if ( contextFromNode( node ) == parent )
      {
          kDebug() << "Error in BaseContextBuilder::buildSubContexts(...): du-context was not replaced with new one";
          DUChainWriteLocker lock( DUChain::lock() );
          deleteContextOnNode( node );
      }
  
      return contextFromNode( node );
  }

  void deleteContextOnNode( T* node )
  {
    delete contextFromNode( node );
    setContextFromNode( node, 0 );
  }

  void openContext( DUContext* newContext )
  {
    m_contextStack.push( newContext );
    m_nextContextStack.push( 0 );
  }
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, T* identifier )
  {
    if ( m_compilingContexts )
    {
      DUContext* ret = openContextInternal( editorFindRange( rangeNode ), type, identifier ? identifierForName( identifier ) : QualifiedIdentifier() );
      setContextOnNode( rangeNode, ret );
      return ret;
    }
    else
    {
      openContext( rangeNode->context );
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
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
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }

  DUContext* openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
    {
      DUContext* ret = openContextInternal( editorFindRange( fromRange, toRange ), type, identifier );
      fromRange->context = ret;
      return ret;
    }
    else
    {
      openContext( fromRange->context );
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }
  
  void closeContext()
  {
    {
      DUChainWriteLocker lock( DUChain::lock() );
      currentContext()->cleanIfNotEncountered( m_encountered, m_compilingContexts );
      setEncountered( currentContext() );
    }
  
    m_lastContext = currentContext();
    m_contextStack.pop();
    m_nextContextStack.pop();
    m_editor->exitCurrentRange();
  }
  
  void setEncountered( DUChainBase* item )
  {
    m_encountered.insert( item );
  }

  bool wasEncountered( DUChainBase* item )
  {
    return m_encountered.contains( item );
  }
  
  void setIdentifier( const QString& id )
  {
    m_identifier = Identifier( id );
    m_qIdentifier.clear();
    m_qIdentifier.push( m_identifier );
  }

  QualifiedIdentifier qualifiedIdentifier()
  {
    return m_qIdentifier;
  }
  
  template <typename E> E* editor()
  {
    E* editor = dynamic_cast<E*>( m_editor );
    if( !editor )
      Q_ASSERT_X(false, "editor cast", "Couldn't cast editor to proper return type");
    return editor;
  }
  
private:

  bool recompiling()
  {
    return m_recompiling;
  }

  DUContext * currentContext()
  {
    return m_contextStack.top();
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

  Identifier m_identifier;
  QualifiedIdentifier m_qIdentifier;
  EditorIntegrator* m_editor;
  bool m_compilingContexts : 1;
  bool m_recompiling : 1;
  bool m_ownsEditorIntegrator: 1;
  QStack<int> m_nextContextStack;
  DUContext* m_lastContext;
  QList<DUContext*> m_importedParentContexts;
  QSet<KDevelop::DUChainBase*> m_encountered;
  QStack<KDevelop::DUContext*> m_contextStack;

};

}

#endif
