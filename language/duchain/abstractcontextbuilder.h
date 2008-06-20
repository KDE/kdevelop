/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2006 Roberto Raggi <roberto@kdevelop.org>                   *
 *   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>                      *
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

#ifndef KDEV_ABSTRACTCONTEXTBUILDER_H
#define KDEV_ABSTRACTCONTEXTBUILDER_H

#include <climits>

#include <QtCore/QMutexLocker>

#include <ktexteditor/smartinterface.h>
#include <ktexteditor/range.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "language/editor/editorintegrator.h"
#include "language/duchain/topducontext.h"
#include "language/duchain/duchainpointer.h"
#include "language/duchain/duchainlock.h"
#include "language/duchain/duchain.h"
#include "language/duchain/ducontext.h"
#include "language/duchain/identifier.h"
#include "language/duchain/indexedstring.h"
#include "language/duchain/parsingenvironment.h"
#include "language/duchain/smartconverter.h"

namespace KDevelop
{

template<typename T, typename NameT>
class KDEVPLATFORMLANGUAGE_EXPORT AbstractContextBuilder
{
public:
  AbstractContextBuilder()
    : m_editor( 0 )
    , m_ownsEditorIntegrator(false)
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_lastContext( 0 )
  {
  }

  virtual ~AbstractContextBuilder()
  {
    if (m_ownsEditorIntegrator)
      delete m_editor;
  }
  
  void setEditor(EditorIntegrator* editor, bool ownsEditorIntegrator)
  {
    m_editor = editor;
    m_ownsEditorIntegrator = ownsEditorIntegrator;
  }

  virtual TopDUContext* build( const IndexedString& url, T* node, 
                              const TopDUContextPointer& updateContext
                                    = TopDUContextPointer() )
  {
    m_compilingContexts = true;
    m_editor->setCurrentUrl( url.str() );
  
    TopDUContext* top = 0;
    {
      DUChainWriteLocker lock( DUChain::lock() );
      top = updateContext.data();

      // If the duchain does not have smart ranges associated, create them now from the preexisting simple ranges
      //if( top && !top->smartRange() && hasSmartEditor() )
        //smartenContext(topLevelContext);

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
  /**
   * Support another builder by tracking the current context.
   * @param context the context to use. Must be set when the given node has no context. When it has one attached, this parameter is not needed.
   */
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
  virtual KTextEditor::Range editorFindRangeForContext( T* fromRange, T* toRange )
  {
    return editorFindRange(fromRange, toRange);
  }
  
  virtual QualifiedIdentifier identifierForNode( NameT* ) = 0;
  virtual DUContext* newContext(const SimpleRange& range)
  {
    return new DUContext(editor()->currentUrl(), range, currentContext());
  }

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

  /// Iterates a duchain and creates smart ranges for the objects.  \warning you must hold the duchain write lock to call this function.
  void smartenContext(TopDUContext* topLevelContext) {    
    if( topLevelContext && !topLevelContext->smartRange() && hasSmartEditor() ) {
      //This happens! The problem seems to be that sometimes documents are not added to EditorIntegratorStatic in time.
      //This means that DocumentRanges are created although the document is already loaded, which means that SmartConverter in CppLanguageSupport is not triggered.
      //Since we do not want this to be so fragile, do the conversion here if it isn't converted(instead of crashing).
      // Smart mutex locking is performed by the smart converter.
      SmartConverter conv(m_editor, 0);
      conv.convertDUChain(topLevelContext);
    }
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
          startVisiting( node );
          closeContext();
      }
  
      m_compilingContexts = false;
  
      if ( contextFromNode( node ) == parent )
      {
          kDebug() << "Error in AbstractContextBuilder::buildSubContexts(...): du-context was not replaced with new one";
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
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, NameT* identifier = 0)
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( rangeNode, rangeNode ), type, identifier ? identifierForNode( identifier ) : QualifiedIdentifier() );
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
 
  DUContext* openContext(T* node, const KDevelop::SimpleRange& range, DUContext::ContextType type, NameT* identifier = 0)
  {
    if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
      kDebug() << "opening custom context";
#endif
      DUContext* ret = openContextInternal(range, type, identifier ? identifierForNode(identifier) : QualifiedIdentifier());
      setContextOnNode( node, ret );
      return ret;

    } else {
      openContext( contextFromNode(node) );
      m_editor->setCurrentRange(currentContext()->smartRange());
      return currentContext();
    }
  }
  
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( rangeNode, rangeNode ), type, identifier );
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

  DUContext* openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier = QualifiedIdentifier() )
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
      kDebug() << "opening context with text" << editor()->tokensToStrings( fromRange->start_token, toRange->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( fromRange, toRange ), type, identifier );
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
  
  // Write lock is already held here...
  virtual void closeContext()
  {
    {
      DUChainWriteLocker lock( DUChain::lock() );
      //Remove all slaves that were not encountered while parsing
      if(m_compilingContexts)
        currentContext()->cleanIfNotEncountered( m_encountered );
      setEncountered( currentContext() );
    }
  
    m_lastContext = currentContext();
    m_contextStack.pop();
    m_nextContextStack.pop();
    if(m_editor->smart())
      m_editor->exitCurrentRange();
  }
  
  /**Signalize that a specific item has been encoutered while parsing.
   * All contained items that are not signalized will be deleted at some stage
   * */
  void setEncountered( DUChainBase* item )
  {
    m_encountered.insert( item );
  }

  /**
   * @return whether the given item is in the set of encountered items
   * */
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
  
  EditorIntegrator* editor() const
  {
    return m_editor;
  }
  
  const QStack<DUContext*>& contextStack() const
  {
    return m_contextStack;
  }

  /// TODO: make private again?
  int& nextContextIndex()
  {
    return m_nextContextStack.top();
  }

  virtual DUContext* openContextInternal( const SimpleRange& range, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    Q_ASSERT( m_compilingContexts );
    DUContext* ret = 0L;
    if(range.start > range.end)
      kDebug(9007) << "Bad context-range" << range.textRange();

    {
      if ( recompiling() )
      {
        DUChainReadLocker readLock( DUChain::lock() );
        const QVector<DUContext*>& childContexts = currentContext()->childContexts();

        QMutexLocker smartLock( m_editor->smartMutex() );
        // translated is now in sync with the current state of the document, with whatever changes
        // have occurred since the text was fetched.
        SimpleRange translated = m_editor->translate(range);

        for ( ; nextContextIndex() < childContexts.count(); ++nextContextIndex() )
        {
          DUContext* child = childContexts.at( nextContextIndex() );

          if ( child->range().start > translated.end && child->smartRange() ) {
#ifdef DEBUG_UPDATE_MATCHING
              kDebug() << "range order mismatch, stopping because encountered" << child->range().textRange();
#endif
              break;
          }

          if ( child->type() == type && child->localScopeIdentifier() == identifier && child->range() == translated )
          {
            // No need to have the translated range accurate any more
            // Also we can't unlock after the duchain lock is unlocked
            smartLock.unlock();

            // Match
            ret = child;
            readLock.unlock();
            DUChainWriteLocker writeLock( DUChain::lock() );

            ret->clearImportedParentContexts();
            m_editor->setCurrentRange( ret->smartRange() );
            ++nextContextIndex();
            break;
          }else{
#ifdef DEBUG_UPDATE_MATCHING
            kDebug() << "skipping range" << childContexts.at(nextContextIndex())->localScopeIdentifier() << childContexts.at(nextContextIndex())->range().textRange();
#endif
          }
        }
      }

      if ( !ret )
      {
        DUChainWriteLocker writeLock( DUChain::lock() );

        ret = newContext( SimpleRange( range ) );
        ret->setSmartRange( m_editor->createRange( range.textRange() ), DocumentRangeObject::Own );
        ret->setType( type );

        if (!identifier.isEmpty())
          ret->setLocalScopeIdentifier(identifier);

        ret->setInSymbolTable(type == DUContext::Class || type == DUContext::Namespace || type == DUContext::Global || type == DUContext::Helper || type == DUContext::Enum);

        if( recompiling() )
          kDebug() << "created new context while recompiling for " << identifier.toString() << "(" << ret->range().textRange() << ")";
      }
    }

    m_encountered.insert( ret );
    openContext( ret );
    return ret;
  }

private:
  Identifier m_identifier;
  QualifiedIdentifier m_qIdentifier;
  EditorIntegrator* m_editor;
  bool m_ownsEditorIntegrator: 1;
  bool m_compilingContexts : 1;
  bool m_recompiling : 1;
  QStack<int> m_nextContextStack;
  DUContext* m_lastContext;
  //Here all valid declarations/uses/... will be collected
  QSet<DUChainBase*> m_encountered;
  QStack<DUContext*> m_contextStack;
};

}

#endif
