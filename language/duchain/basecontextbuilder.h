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

#include "language/duchain/topducontext.h"
#include "language/duchain/duchainpointer.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/duchain.h"
#include "language/duchain/ducontext.h"
#include "language/duchain/identifier.h"
#include "language/duchain/indexedstring.h"
#include "language/duchain/parsingenvironment.h"

namespace KDevelop
{

template<typename T>
class KDEVPLATFORMLANGUAGE_EXPORT BaseContextBuilder
{
public:
  BaseContextBuilder( EditorIntegrator* editor, bool ownsEditorIntegrator )
    : m_editor( editor )
    , m_ownsEditorIntegrator(ownsEditorIntegrator)
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_lastContext( 0 )
  {
  }

  virtual ~BaseContextBuilder()
  {
    if (m_ownsEditorIntegrator)
      delete m_editor;
  }

  TopDUContext* buildContexts( const IndexedString& url, T* node, 
                              const TopDUContextPointer& updateContext
                                    = TopDUContextPointer() )
  {
    m_compilingContexts = true;
    m_editor->setCurrentUrl( url.str() );
  
    TopDUContext* top = 0;
    {
      DUChainWriteLocker lock( DUChain::lock() );
      top = updateContext.data();
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
  
    supportBuild( node, top );
  
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
  
  virtual void supportBuild( T* node, DUContext* context = 0 )
  {
    if (!context)
      context = contextFromNode(node);

    openContext( context );

    m_editor->setCurrentUrl(currentContext()->url());

    m_editor->setCurrentRange(currentContext()->smartRange());

    startVisiting(node);

    closeContext();

    Q_ASSERT(m_contextStack.isEmpty());
  }

  virtual void startVisiting( T* node ) = 0;
  virtual void setContextOnNode( T* node, DUContext* ctx ) = 0;
  virtual DUContext* contextFromNode( T* node ) = 0;
  virtual KTextEditor::Range editorFindRange( T* fromRange, T* toRange ) = 0;
  virtual QualifiedIdentifier identifierForNode( T* ) const = 0;

  inline DUContext* currentContext() const { return m_contextStack.top(); }
  inline DUContext* lastContext() const { return m_lastContext; }
  inline void clearLastContext() { m_lastContext = 0; }
  
  /// Returns true if we are recompiling a definition-use chain
  inline bool recompiling() const { return m_recompiling; }
  inline void setRecompiling(bool recomp) { m_recompiling = recomp; }

  inline bool compilingContexts() const { return m_compilingContexts; }
  inline void setCompilingContexts(bool compilingContexts) { m_compilingContexts = compilingContexts; }
  
  bool hasSmartEditor() const
  {
    return m_editor->smart();
  }

  DUContext* buildSubContexts( const KUrl& url, T *node,
                                         DUContext* parent )
  {
  //     m_compilingContexts = true;
  //     m_recompiling = false;
      m_editor->setCurrentUrl( HashedString( url.pathOrUrl() ) );
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

  virtual void openContext( DUContext* newContext )
  {
    m_contextStack.push( newContext );
    m_nextContextStack.push( 0 );
  }
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, T* identifier )
  {
    if ( m_compilingContexts )
    {
      DUContext* ret = openContextInternal( editorFindRange( rangeNode, rangeNode ), type, identifier ? identifierForNode( identifier ) : QualifiedIdentifier() );
      setContextOnNode( rangeNode, ret );
      return ret;
    }
    else
    {
      openContext( contextFromNode(rangeNode) );
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
    {
      //kDebug() << "Opening ContextInternal";
      DUContext* ret = openContextInternal( editorFindRange( rangeNode, rangeNode ), type, identifier );
      //kDebug() << "Associating context" ;
      setContextOnNode( rangeNode, ret );
      return ret;
    }
    else
    {
      //kDebug() << "Opening Context associated with node";
      openContext( contextFromNode(rangeNode) );
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }

  DUContext* openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
    {
      DUContext* ret = openContextInternal( editorFindRange( fromRange, toRange ), type, identifier );
      setContextOnNode( fromRange, ret );
      return ret;
    }
    else
    {
      openContext( contextFromNode(fromRange) );
      m_editor->setCurrentRange( currentContext()->smartRange() );
      return currentContext();
    }
  }
  
  virtual void closeContext()
  {
    {
      DUChainWriteLocker lock( DUChain::lock() );
      //currentContext()->cleanIfNotEncountered( m_encountered );
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
    m_qIdentifier.push( m_identifier );
  }

  QualifiedIdentifier qualifiedIdentifier() const
  {
    return m_qIdentifier;
  }
  
  void clearQualifiedIdentifier()
  {
    m_qIdentifier.clear();
  }
  
  template <typename E> E* editor()
  {
    E* editor = dynamic_cast<E*>( m_editor );
    if( !editor )
      Q_ASSERT_X(false, "editor cast", "Couldn't cast editor to proper return type");
    return editor;
  }
  
private:
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

          if ( type != DUContext::Class && type != DUContext::Global && DUContext::Namespace )
            ret->setInSymbolTable(false);
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
  bool m_ownsEditorIntegrator: 1;
  bool m_compilingContexts : 1;
  bool m_recompiling : 1;
  QStack<int> m_nextContextStack;
  DUContext* m_lastContext;
  QList<DUContext*> m_importedParentContexts;
  QSet<DUChainBase*> m_encountered;
  QStack<DUContext*> m_contextStack;
};

}

#endif
