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

#ifndef ABSTRACTABSTRACTCONTEXTBUILDER_H
#define ABSTRACTABSTRACTCONTEXTBUILDER_H

#include <climits>

#include <QtCore/QMutexLocker>

#include <ktexteditor/smartinterface.h>
#include <ktexteditor/range.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "../../editor/editorintegrator.h"
#include "../topducontext.h"
#include "../duchainpointer.h"
#include "../duchainlock.h"
#include "../duchain.h"
#include "../ducontext.h"
#include "../identifier.h"
#include "../indexedstring.h"
#include "../parsingenvironment.h"
#include "../smartconverter.h"

namespace KDevelop
{
/**
 * \short Abstract definition-use chain context builder class
 *
 * The AbstractContextBuilder is a convenience class template for creating customized
 * definition-use chain context builders from an AST.  It simplifies:
 * - use of your editor integrator
 * - creating or modifying an existing DUContext tree
 * - following a DUContext tree for second and subsequent passes, if required
 * - opening and closing DUContext instances
 * - tracking which DUContext instances are still present when recompiling, and removing DUContexts which no longer exist in the source code.
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template<typename T, typename NameT>
class AbstractContextBuilder
{
public:
  /// Constructor.
  AbstractContextBuilder()
    : m_editor( 0 )
    , m_ownsEditorIntegrator(false)
    , m_compilingContexts( false )
    , m_recompiling( false )
    , m_lastContext( 0 )
  {
  }

  /// Destructor.  Deletes the editor integrator, if one was created specifically for this builder only.
  virtual ~AbstractContextBuilder()
  {
    if (m_ownsEditorIntegrator)
      delete m_editor;
  }

  /**
   * Associates an editor integrator with this builder.
   *
   * \param editor EditorIntegrator instance to use
   * \param ownsEditorIntegrator set to true if this builder created the editor integrator (and should thus delete it later),
   *                             or false if the editor integrator is owned by another object.
   */
  void setEditor(EditorIntegrator* editor, bool ownsEditorIntegrator)
  {
    m_editor = editor;
    m_ownsEditorIntegrator = ownsEditorIntegrator;
  }

  /**
   * Entry point for building a definition-use chain with this builder.
   *
   * This function determines whether we are updating a chain, or creating a new one.  If we are
   * creating a new chain, a new TopDUContext is created and registered with DUChain.
   *
   * \param url Url of the document being parsed.
   * \param node AST node to start building from.
   * \param updateContext TopDUContext to update if a duchain was previously created for this url, otherwise pass a null pointer.
   *
   * \returns the newly created or updated TopDUContext pointer.
   */
  virtual ReferencedTopDUContext build( const IndexedString& url, T* node,
                              ReferencedTopDUContext updateContext
                                    = ReferencedTopDUContext(), bool useSmart = true )
  {
    m_compilingContexts = true;
    m_editor->setCurrentUrl( url, useSmart );

    ReferencedTopDUContext top;
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
//         kDebug() << "re-compiling";
        m_recompiling = true;
        if( m_compilingContexts )
        {
          LockedSmartInterface iface = m_editor->smart();
          if( iface && top->range().textRange() != iface.currentDocument()->documentRange() )
          {
            //Happens if the context wasn't smart
            top->setRange( SimpleRange( iface.currentDocument()->documentRange() ) );
          }
        }
      }else
      {
//         kDebug() << "compiling";
        {
          LockedSmartInterface iface = m_editor->smart();
          top = newTopContext( iface.currentDocument()
                                      ? SimpleRange( iface.currentDocument()->documentRange() )
                                      : SimpleRange( SimpleCursor( 0, 0 ), SimpleCursor( INT_MAX, INT_MAX ) ) );
          top->setSmartRange( m_editor->topRange( iface, EditorIntegrator::DefinitionUseChain ), DocumentRangeObject::Own );
          top->setType( DUContext::Global );
        }
        DUChain::self()->addDocumentChain( top );
      }
      setEncountered( top );
      setContextOnNode( node, top );
    }

    supportBuild( node, top );

    {
      LockedSmartInterface iface = m_editor->smart();
      if( iface && top->range().textRange() != iface.currentDocument()->documentRange() )
      {
        kDebug() << "WARNING: top level context has wrong size:" << top->range().textRange() << "should be:" << iface.currentDocument()->documentRange();
        top->setRange( iface.currentDocument()->documentRange() );
      }
    }
    {
      /*DUChainReadLocker lock( DUChain::lock() );
      //foreach(DUContext* context, topLevelContext->childContexts());
      kDebug() << "built top-level context with" << top->localDeclarations().count() << "declarations," << top->localDeclarations().count() << " Definitions and" << top->childContexts().size() << "Child-Contexts";

      foreach( DUContext* contexts, top->childContexts() )
      {
        kDebug() << "CHILD:" << contexts->scopeIdentifier( true ) << "Parent:" << ( dynamic_cast<TopDUContext*>( contexts->parentContext() ) ? "top-context" : "" );
      }*/
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

    //The url must be set before supportBuild is called, together with the decision
    //whether smart-ranges shold be created
    if(m_editor->currentUrl() != currentContext()->url())
      m_editor->setCurrentUrl(currentContext()->url(), true);

    {
      LockedSmartInterface iface = m_editor->smart();
      m_editor->setCurrentRange(iface, currentContext()->smartRange());
    }

    startVisiting(node);

    closeContext();

    Q_ASSERT(m_contextStack.isEmpty());
  }

  /**
   * Entry point to your visitor.  Reimplement and call the appropriate visit function.
   *
   * \param node AST node to visit.
   */
  virtual void startVisiting( T* node ) = 0;

  /**
   * Associate a \a context with a given AST \a node.  Once called on a \a node, the
   * contextFromNode() function should return this \a context when called.
   *
   * \param node AST node to associate
   * \param context DUContext to associate
   */
  virtual void setContextOnNode( T* node, DUContext* context ) = 0;

  /**
   * Retrieve an associated DUContext from the given \a node.  Used on second and
   * subsequent passes of the context builder (for supporting other builds)
   *
   * \param node AST node which was previously associated
   * \returns the DUContext which was previously associated
   */
  virtual DUContext* contextFromNode( T* node ) = 0;

  /**
   * Retrieves a text range from the given nodes.
   *
   * As editor integrators have to be extended to determine ranges from AST nodes,
   * this function must be reimplemented to allow generic retrieving of rangs from nodes.
   *
   * \param fromNode the AST node to start from (on the start boundary)
   * \param toNode the AST node to end at (on the end boundary)
   *
   * \returns the text range encompassing the given AST node(s)
   */
  virtual KTextEditor::Range editorFindRange( T* fromNode, T* toNode ) = 0;

  /**
   * Retrieve a text range for the given nodes.  This is a special function required
   * by c++ support as a different range may need to be retrieved depending on
   * whether macros are involved.  It is not usually required to implement this
   * function separately to editorFindRange() for other languages.
   *
   * \param fromNode the AST node to start from (on the start boundary)
   * \param toNode the AST node to end at (on the end boundary)
   *
   * \returns the text range encompassing the given AST node(s)
   */
  virtual KTextEditor::Range editorFindRangeForContext( T* fromNode, T* toNode )
  {
    return editorFindRange(fromNode, toNode);
  }

  /**
   * Determine the QualifiedIdentifier which corresponds to the given ast \a node.
   *
   * \param node ast node which represents an identifier
   * \return the qualified identifier determined from \a node
   */
  virtual QualifiedIdentifier identifierForNode( NameT* node ) = 0;

  /**
   * Create a new DUContext from the given \a range.
   *
   * This exists so that you can create custom DUContext subclasses for your
   * language if you need to.
   *
   * \param range range for the new context to encompass
   * \returns the newly created context
   */
  virtual DUContext* newContext(const SimpleRange& range)
  {
    return new DUContext(range, currentContext());
  }

  /**
   * Create a new TopDUContext from the given \a range.
   *
   * This exists so that you can create custom TopDUContext subclasses for your
   * language if you need to.
   *
   * \returns the newly created context
   */
  virtual TopDUContext* newTopContext(const SimpleRange& range, ParsingEnvironmentFile* file = 0)
  {
    return new TopDUContext(m_editor->currentUrl(), range, file);
  }

  /// Determine the currently open context. \returns the current context.
  inline DUContext* currentContext() const { return m_contextStack.top(); }
  /// Determine the last closed context. \returns the last closed context.
  inline DUContext* lastContext() const { return m_lastContext; }
  /// Clears the last closed context.
  inline void clearLastContext() { m_lastContext = 0; }

  inline void setLastContext(DUContext* context) { m_lastContext = context; }

  TopDUContext* topContext() const {
    return currentContext()->topContext();
  }

  /**
   * Determine if we are recompiling an existing definition-use chain, or if
   * a new chain is being created from scratch.
   *
   * \returns true if an existing duchain is being updated, otherwise false.
   */
  inline bool recompiling() const { return m_recompiling; }

  /**
   * Tell the context builder whether we are recompiling an existing definition-use chain, or if
   * a new chain is being created from scratch.
   *
   * \param recomp set to true if an existing duchain is being updated, otherwise false.
   */
  inline void setRecompiling(bool recomp) { m_recompiling = recomp; }

  /**
   * Determine whether this pass will create DUContext instances.
   *
   * On the first pass of definition-use chain compiling, DUContext instances
   * are created to represent contexts in the source code.  These contexts are
   * associated with their AST nodes at the time (see setContextOnNode()).
   *
   * On second and subsequent passes, the contexts already exist and thus can be
   * retrieved through contextFromNode().
   *
   * \returns true if compiling contexts (ie. 1st pass), otherwise false.
   */
  inline bool compilingContexts() const { return m_compilingContexts; }

  /**
   * Sets whether we need to create ducontexts, ie. if this is the first pass.
   *
   * \sa compilingContexts()
   */
  inline void setCompilingContexts(bool compilingContexts) { m_compilingContexts = compilingContexts; }

  /**
   * Create child contexts for only a portion of the document at \a url.
   *
   * \param url The url of the document to parse
   * \param node The AST node which corresponds to the context to parse
   * \param parent The DUContext which encompasses the \a node.
   * \returns The DUContext which was reparsed, ie. \a parent.
   */
  DUContext* buildSubContexts( const KUrl& url, T *node,
                                         DUContext* parent )
  {
  //     m_compilingContexts = true;
  //     m_recompiling = false;
      {
        DUChainReadLocker lock( DUChain::lock() );
        m_editor->setCurrentUrl( IndexedString( url.pathOrUrl() ), (bool)parent->smartRange() );
      }
      setContextOnNode( node, parent );
      {
          openContext( contextFromNode( node ) );
          {
            LockedSmartInterface iface = m_editor->smart();
            m_editor->setCurrentRange( iface, m_editor->topRange( iface, EditorIntegrator::DefinitionUseChain ) );
          }
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

  /**
   * Delete the DUContext which is associated with the given \a node,
   * and remove the association.
   *
   * \param node Node which is associated with the context to delete.
   */
  void deleteContextOnNode( T* node )
  {
    delete contextFromNode( node );
    setContextOnNode( node, 0 );
  }

  /**
   * Open a context, and create / update it if necessary.
   *
   * \param rangeNode The range which encompasses the context.
   * \param type The type of context to open.
   * \param identifier The range which encompasses the name of this context, if one exists.
   * \returns the opened context.
   */
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, NameT* identifier = 0)
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
    //kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( rangeNode, rangeNode ), type, identifier ? identifierForNode( identifier ) : QualifiedIdentifier() );
      setContextOnNode( rangeNode, ret );
      return ret;
    }
    else
    {
      openContext( contextFromNode(rangeNode) );
      {
        LockedSmartInterface iface = editor()->smart();
        editor()->setCurrentRange(iface, currentContext()->smartRange());
      }
      return currentContext();
    }
  }

  /**
   * Open a context, and create / update it if necessary.
   *
   * \param node The range to associate with the context.
   * \param range A custom range which the context should encompass.
   * \param type The type of context to open.
   * \param identifier The range which encompasses the name of this context, if one exists.
   * \returns the opened context.
   */
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
      {
        LockedSmartInterface iface = editor()->smart();
        editor()->setCurrentRange(iface, currentContext()->smartRange());
      }
      return currentContext();
    }
  }

  /**
   * Open a context, and create / update it if necessary.
   *
   * \param node The range to associate with the context.
   * \param range A custom range which the context should encompass.
   * \param type The type of context to open.
   * \param identifier The identifier for this context
   * \returns the opened context.
   */
  DUContext* openContext(T* node, const KDevelop::SimpleRange& range, DUContext::ContextType type, QualifiedIdentifier id)
  {
    if (m_compilingContexts) {
#ifdef DEBUG_UPDATE_MATCHING
      kDebug() << "opening custom context";
#endif
      DUContext* ret = openContextInternal(range, type, id);
      setContextOnNode( node, ret );
      return ret;

    } else {
      openContext( contextFromNode(node) );
      {
        LockedSmartInterface iface = editor()->smart();
        editor()->setCurrentRange(iface, currentContext()->smartRange());
      }
      return currentContext();
    }
  }

  /**
   * Open a context, and create / update it if necessary.
   *
   * \param rangeNode The range which encompasses the context.
   * \param type The type of context to open.
   * \param identifier The identifier which corresponds to the context.
   * \returns the opened context.
   */
  DUContext* openContext( T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
    //kDebug() << "opening context with text" << editor()->tokensToStrings( rangeNode->start_token, rangeNode->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( rangeNode, rangeNode ), type, identifier );
      setContextOnNode( rangeNode, ret );
      return ret;
    }
    else
    {
      //kDebug() << "Opening Context associated with node";
      openContext( contextFromNode(rangeNode) );
      {
        LockedSmartInterface iface = editor()->smart();
        editor()->setCurrentRange(iface, currentContext()->smartRange());
      }
      return currentContext();
    }
  }

  /**
   * Open a context, and create / update it if necessary.
   *
   * \param fromRange The range which starts the context.
   * \param toRange The range which ends the context.
   * \param type The type of context to open.
   * \param identifier The identifier which corresponds to the context.
   * \returns the opened context.
   */
  DUContext* openContext( T* fromRange, T* toRange, DUContext::ContextType type, const QualifiedIdentifier& identifier = QualifiedIdentifier() )
  {
    if ( m_compilingContexts )
    {
#ifdef DEBUG_UPDATE_MATCHING
      //kDebug() << "opening context with text" << editor()->tokensToStrings( fromRange->start_token, toRange->end_token );
#endif
      DUContext* ret = openContextInternal( editorFindRangeForContext( fromRange, toRange ), type, identifier );
      setContextOnNode( fromRange, ret );
      return ret;
    }
    else
    {
      openContext( contextFromNode(fromRange) );
      {
        LockedSmartInterface iface = editor()->smart();
        editor()->setCurrentRange(iface, currentContext()->smartRange());
      }
      return currentContext();
    }
  }

  /**
   * Open a newly created or previously existing context.
   *
   * The open context is put on the context stack, and becomes the new
   * currentContext().
   *
   * \warning When you call this, you also have to open a range! If you want to re-use
   * the range associated to the context, use injectContext
   *
   * \param newContext Context to open.
   */
  virtual void openContext( DUContext* newContext )
  {
    m_contextStack.push( newContext );
    m_nextContextStack.push( 0 );
  }

  /**
   * This can be used to temporarily change the current context.
   * \param range The range that will be used as new current range, or zero(then the range associated to the context is used)
   * */
  void injectContext( const LockedSmartInterface& iface, DUContext* ctx, KTextEditor::SmartRange* range = 0 ) {
    openContext( ctx );
    m_editor->setCurrentRange( iface, range ? range : ctx->smartRange() );
  }

  /**
   * Use this to close the context previously injected with injectContext.
   * */
  void closeInjectedContext(const LockedSmartInterface& iface) {
    m_contextStack.pop();
    m_nextContextStack.pop();
    if(m_editor->smart())
      m_editor->exitCurrentRange(iface);
  }

  /**
   * Close the current DUContext.  When recompiling, this function will remove any
   * contexts that were not encountered in this passing run.
   * \note The DUChain write lock is already held here.
   */
  virtual void closeContext()
  {
    {
      DUChainWriteLocker lock( DUChain::lock() );
      //Remove all slaves that were not encountered while parsing
      if(m_compilingContexts)
        currentContext()->cleanIfNotEncountered( m_encountered );
      setEncountered( currentContext() );

      m_lastContext = currentContext();
    }

    m_contextStack.pop();
    m_nextContextStack.pop();
    if(LockedSmartInterface iface = m_editor->smart())
      m_editor->exitCurrentRange(iface);
  }

  /**
   * Remember that a specific item has been encoutered while parsing.
   * All items that are not encountered will be deleted at some stage.
   *
   * \param item duchain item that was encountered.
   * */
  void setEncountered( DUChainBase* item )
  {
    m_encountered.insert( item );
  }

  /**
   * Determine whether the given \a item is in the set of encountered items.
   *
   * @return true if the \a item has been encountered, otherwise false.
   * */
  bool wasEncountered( DUChainBase* item )
  {
    return m_encountered.contains( item );
  }

  /**
   * Set the current identifier to \a id.
   *
   * \param id the new current identifier.
   */
  void setIdentifier( const QString& id )
  {
    m_identifier = Identifier( id );
    m_qIdentifier.push( m_identifier );
  }

  /**
   * Determine the current identifier.
   * \returns the current identifier.
   */
  QualifiedIdentifier qualifiedIdentifier() const
  {
    return m_qIdentifier;
  }

  /**
   * Clears the current identifier.
   */
  void clearQualifiedIdentifier()
  {
    m_qIdentifier.clear();
  }

  /**
   * Retrieve the associated editor integrator.
   *
   * \returns the editor integrator being used by this builder.
   */
  EditorIntegrator* editor() const
  {
    return m_editor;
  }

  /**
   * Retrieve the current context stack.  This function is not expected
   * to be used often and may be phased out.
   *
   * \todo Audit whether access to the context stack is still required, and provide
   *       replacement functionality if possible.
   */
  const QStack<DUContext*>& contextStack() const
  {
    return m_contextStack;
  }

  /**
   * Access the index of the child context which has been encountered.
   *
   * \todo further delineate the role of this function and rename / document better.
   * \todo make private again?
   */
  int& nextContextIndex()
  {
    return m_nextContextStack.top();
  }

  /**
   * Open a context, either creating it if it does not exist, or referencing a previously existing
   * context if already encountered in a previous duchain parse run (when recompiling()).
   *
   * \param range The range of the context.
   * \param type The type of context to create.
   * \param identifier The identifier which corresponds to the context.
   * \returns the opened context.
   */
  virtual DUContext* openContextInternal( const SimpleRange& range, DUContext::ContextType type, const QualifiedIdentifier& identifier )
  {
    Q_ASSERT( m_compilingContexts );
    DUContext* ret = 0L;
    if(range.start > range.end)
    {
      kDebug() << "Bad context-range" << range.textRange();
    }

    {
      if ( recompiling() )
      {
        DUChainReadLocker readLock( DUChain::lock() );
        const QVector<DUContext*>& childContexts = currentContext()->childContexts();

        LockedSmartInterface iface = m_editor->smart();
        // translated is now in sync with the current state of the document, with whatever changes
        // have occurred since the text was fetched.
        SimpleRange translated = m_editor->translate(iface, range);

//         if(iface)
//           kDebug() << "translated by" << (translated.start.textCursor() - range.start.textCursor()) << (translated.end.textCursor() - range.end.textCursor()) << "to revision" << iface->currentRevision();

        int currentIndex = nextContextIndex();
        int lookingAhead = 0;

        for ( ; currentIndex < childContexts.count(); ++currentIndex )
        {
          DUContext* child = childContexts.at( currentIndex );

//           if ( child->range().start > translated.end && child->smartRange() ) {
// #ifdef DEBUG_UPDATE_MATCHING
//               kDebug() << "While searching" << identifier << translated.textRange() << "(from" << range.textRange() << ") stopping because found" << child->localScopeIdentifier() << child->range().textRange();
// #endif
//               break;
//           }

	  //For unnamed child-ranges, we still do range-comparison, because we cannot distinguish them in other ways
          if ( child->type() == type && child->localScopeIdentifier() == identifier && (!identifier.isEmpty() || child->range() == translated) )
          {
            if(child->range() != translated && child->smartRange()) {
              kDebug() << "range mismatch while updating context. Range:" << child->range().textRange() << "should be:" << translated.textRange();
              break;
            }
            // No need to have the translated range accurate any more
            // Also we can't unlock after the duchain lock is unlocked
            iface.unlock();

            // Match
            ret = child;
            readLock.unlock();
            DUChainWriteLocker writeLock( DUChain::lock() );

            ret->clearImportedParentContexts();
            m_editor->setCurrentRange( iface, ret->smartRange() );
            ++currentIndex;
            break;
          }else{
#ifdef DEBUG_UPDATE_MATCHING
          if(child->type() != type)
            kDebug() << "type mismatch" << child->type() << type;
          if(child->localScopeIdentifier() != identifier)
            kDebug() << "identifier mismatch" << child->localScopeIdentifier() << identifier;
          if(translated != child->range())
            kDebug() << "range mismatch" << child->range().textRange() << translated.textRange();

            kDebug() << "skipping range" << childContexts.at(currentIndex)->localScopeIdentifier() << childContexts.at(currentIndex)->range().textRange();
#endif
          if ( child->range().start > translated.end && child->smartRange() && (currentIndex+1 == childContexts.count() || (childContexts.at(currentIndex+1)->localScopeIdentifier() != identifier || childContexts.at(currentIndex+1)->type() != type)) ) {
            ++lookingAhead;
            const int maxLookahead = 5;
            if(lookingAhead > maxLookahead)
              break; //Don't move the currentIndex too far
          }
          }
        }
        if(ret)
          nextContextIndex() = currentIndex; //If we had a match, jump forward to that position
        else
          ++nextContextIndex();   //If we did not have a match, just increment by 1
      }

      if ( !ret )
      {
        DUChainWriteLocker writeLock( DUChain::lock() );

        ret = newContext( SimpleRange( range ) );
        {
          LockedSmartInterface iface = m_editor->smart();
          ret->setSmartRange( m_editor->createRange( iface, range.textRange() ), DocumentRangeObject::Own );
        }
        ret->setType( type );

        if (!identifier.isEmpty())
          ret->setLocalScopeIdentifier(identifier);

        setInSymbolTable(ret);

//         if( recompiling() )
//           kDebug() << "created new context while recompiling for " << identifier.toString() << "(" << ret->range().textRange() << ")";
      }
    }

    m_encountered.insert( ret );
    openContext( ret );
    return ret;
  }

  ///This function should call context->setInSymbolTable(..) with an appropriate decision. The duchain is write-locked when this is called.
  virtual void setInSymbolTable(DUContext* context) {
    if(!context->parentContext()->inSymbolTable()) {
      context->setInSymbolTable(false);
      return;
    }
    DUContext::ContextType type = context->type();
    context->setInSymbolTable(type == DUContext::Class || type == DUContext::Namespace || type == DUContext::Global || type == DUContext::Helper || type == DUContext::Enum);
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
