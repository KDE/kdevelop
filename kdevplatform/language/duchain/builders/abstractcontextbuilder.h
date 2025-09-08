/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2006 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_ABSTRACTABSTRACTCONTEXTBUILDER_H
#define KDEVPLATFORM_ABSTRACTABSTRACTCONTEXTBUILDER_H

#include <climits>

#include "../topducontext.h"
#include "../duchainpointer.h"
#include "../duchainlock.h"
#include "../duchain.h"
#include "../ducontext.h"
#include "../identifier.h"
#include "../parsingenvironment.h"

#include <serialization/indexedstring.h>
#include <util/stack.h>

namespace KDevelop {
/**
 * \short Abstract definition-use chain context builder class
 *
 * The AbstractContextBuilder is a convenience class template for creating customized
 * definition-use chain context builders from an AST.  It simplifies:
 * - creating or modifying an existing DUContext tree
 * - following a DUContext tree for second and subsequent passes, if required
 * - opening and closing DUContext instances
 * - tracking which DUContext instances are still present when recompiling, and removing DUContexts which no longer exist in the source code.
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template <typename T, typename NameT>
class AbstractContextBuilder
{
public:
    /// Constructor.
    AbstractContextBuilder() : m_compilingContexts(false)
        , m_recompiling(false)
        , m_lastContext(nullptr)
    {
    }

    virtual ~AbstractContextBuilder()
    {
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
    virtual ReferencedTopDUContext build(const IndexedString& url, T* node,
                                         const ReferencedTopDUContext& updateContext
                                             = ReferencedTopDUContext())
    {
        m_compilingContexts = true;
        m_url = url;

        ReferencedTopDUContext top;
        {
            DUChainWriteLocker lock(DUChain::lock());
            top = updateContext.data();

            if (top) {
                m_recompiling = true;
                Q_ASSERT(top->type() == DUContext::Global);
                Q_ASSERT(DUChain::self()->chainForIndex(top->ownIndex()) == top);
            } else
            {
                top = newTopContext(RangeInRevision(CursorInRevision(0, 0), CursorInRevision(INT_MAX, INT_MAX)));
                DUChain::self()->addDocumentChain(top);
                top->setType(DUContext::Global);
            }

            setEncountered(top);
            setContextOnNode(node, top);
        }

        supportBuild(node, top);

        m_compilingContexts = false;
        return top;
    }

protected:
    /**
     * Support another builder by tracking the current context.
     * @param node the given node.
     * @param context the context to use. Must be set when the given node has no context. When it has one attached, this parameter is not needed.
     */
    virtual void supportBuild(T* node, DUContext* context = nullptr)
    {
        if (!context)
            context = contextFromNode(node);

        Q_ASSERT(context);

        openContext(context);

        startVisiting(node);

        closeContext();

        Q_ASSERT(m_contextStack.isEmpty());
    }

    /**
     * Entry point to your visitor.  Reimplement and call the appropriate visit function.
     *
     * \param node AST node to visit.
     */
    virtual void startVisiting(T* node) = 0;

    /**
     * Associate a \a context with a given AST \a node.  Once called on a \a node, the
     * contextFromNode() function should return this \a context when called.
     *
     * \param node AST node to associate
     * \param context DUContext to associate
     */
    virtual void setContextOnNode(T* node, DUContext* context) = 0;

    /**
     * Retrieve an associated DUContext from the given \a node.  Used on second and
     * subsequent passes of the context builder (for supporting other builds)
     *
     * \param node AST node which was previously associated
     * \returns the DUContext which was previously associated
     */
    virtual DUContext* contextFromNode(T* node) = 0;

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
    virtual RangeInRevision editorFindRange(T* fromNode, T* toNode) = 0;

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
    virtual RangeInRevision editorFindRangeForContext(T* fromNode, T* toNode)
    {
        return editorFindRange(fromNode, toNode);
    }

    /**
     * Determine the QualifiedIdentifier which corresponds to the given ast \a node.
     *
     * \param node ast node which represents an identifier
     * \return the qualified identifier determined from \a node
     */
    virtual QualifiedIdentifier identifierForNode(NameT* node) = 0;

    /**
     * Create a new DUContext from the given \a range.
     *
     * This exists so that you can create custom DUContext subclasses for your
     * language if you need to.
     *
     * \param range range for the new context to encompass
     * \returns the newly created context
     */
    virtual DUContext* newContext(const RangeInRevision& range)
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
    virtual TopDUContext* newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file = nullptr)
    {
        return new TopDUContext(m_url, range, file);
    }

    /// Determine the currently open context. \returns the current context.
    inline DUContext* currentContext() const { return m_contextStack.top(); }
    /// Determine the last closed context. \returns the last closed context.
    inline DUContext* lastContext() const { return m_lastContext; }
    /// Clears the last closed context.
    inline void clearLastContext() { m_lastContext = nullptr; }

    inline void setLastContext(DUContext* context) { m_lastContext = context; }

    TopDUContext* topContext() const
    {
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
     * Create child contexts for only a portion of the document.
     *
     * \param node The AST node which corresponds to the context to parse
     * \param parent The DUContext which encompasses the \a node.
     * \returns The DUContext which was reparsed, ie. \a parent.
     */
    DUContext* buildSubContexts(T* node, DUContext* parent)
    {
        //     m_compilingContexts = true;
        //     m_recompiling = false;
        setContextOnNode(node, parent);
        {
            openContext(contextFromNode(node));
            startVisiting(node);
            closeContext();
        }

        m_compilingContexts = false;

        if (contextFromNode(node) == parent) {
            qDebug() <<
                "Error in AbstractContextBuilder::buildSubContexts(...): du-context was not replaced with new one";
            DUChainWriteLocker lock(DUChain::lock());
            deleteContextOnNode(node);
        }

        return contextFromNode(node);
    }

    /**
     * Delete the DUContext which is associated with the given \a node,
     * and remove the association.
     *
     * \param node Node which is associated with the context to delete.
     */
    void deleteContextOnNode(T* node)
    {
        delete contextFromNode(node);
        setContextOnNode(node, nullptr);
    }

    /**
     * Open a context, and create / update it if necessary.
     *
     * \param rangeNode The range which encompasses the context.
     * \param type The type of context to open.
     * \param identifier The range which encompasses the name of this context, if one exists.
     * \returns the opened context.
     */
    DUContext* openContext(T* rangeNode, DUContext::ContextType type, NameT* identifier = nullptr)
    {
        if (m_compilingContexts) {
            DUContext* ret = openContextInternal(editorFindRangeForContext(rangeNode,
                                                                           rangeNode), type,
                                                 identifier ? identifierForNode(identifier) : QualifiedIdentifier());
            setContextOnNode(rangeNode, ret);
            return ret;
        } else
        {
            openContext(contextFromNode(rangeNode));
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
    DUContext* openContext(T* node, const RangeInRevision& range, DUContext::ContextType type,
                           NameT* identifier = nullptr)
    {
        if (m_compilingContexts) {
            DUContext* ret = openContextInternal(range, type, identifier ? identifierForNode(
                                                     identifier) : QualifiedIdentifier());
            setContextOnNode(node, ret);
            return ret;
        } else {
            openContext(contextFromNode(node));
            return currentContext();
        }
    }

    /**
     * Open a context, and create / update it if necessary.
     *
     * \param node The range to associate with the context.
     * \param range A custom range which the context should encompass.
     * \param type The type of context to open.
     * \param id The identifier for this context
     * \returns the opened context.
     */
    DUContext* openContext(T* node, const RangeInRevision& range, DUContext::ContextType type,
                           const QualifiedIdentifier& id)
    {
        if (m_compilingContexts) {
            DUContext* ret = openContextInternal(range, type, id);
            setContextOnNode(node, ret);
            return ret;
        } else {
            openContext(contextFromNode(node));
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
    DUContext* openContext(T* rangeNode, DUContext::ContextType type, const QualifiedIdentifier& identifier)
    {
        if (m_compilingContexts) {
            DUContext* ret = openContextInternal(editorFindRangeForContext(rangeNode, rangeNode), type, identifier);
            setContextOnNode(rangeNode, ret);
            return ret;
        } else
        {
            openContext(contextFromNode(rangeNode));
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
    DUContext* openContext(T* fromRange, T* toRange, DUContext::ContextType type,
                           const QualifiedIdentifier& identifier = QualifiedIdentifier())
    {
        if (m_compilingContexts) {
            DUContext* ret = openContextInternal(editorFindRangeForContext(fromRange, toRange), type, identifier);
            setContextOnNode(fromRange, ret);
            return ret;
        } else
        {
            openContext(contextFromNode(fromRange));
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
    virtual void openContext(DUContext* newContext)
    {
        m_contextStack.push(newContext);
        m_nextContextStack.push(0);
    }

    /**
     * This can be used to temporarily change the current context.
     * \param ctx The context to be injected
     * */
    void injectContext(DUContext* ctx)
    {
        openContext(ctx);
    }

    /**
     * Use this to close the context previously injected with injectContext.
     * */
    void closeInjectedContext()
    {
        m_contextStack.pop();
        m_nextContextStack.pop();
    }

    /**
     * Close the current DUContext.  When recompiling, this function will remove any
     * contexts that were not encountered in this passing run.
     * \note The DUChain write lock is already held here.
     */
    virtual void closeContext()
    {
        {
            DUChainWriteLocker lock(DUChain::lock());
            //Remove all slaves that were not encountered while parsing
            if (m_compilingContexts)
                currentContext()->cleanIfNotEncountered(m_encountered);
            setEncountered(currentContext());

            m_lastContext = currentContext();
        }

        m_contextStack.pop();
        m_nextContextStack.pop();
    }

    /**
     * Remember that a specific item has been encountered while parsing.
     * All items that are not encountered will be deleted at some stage.
     *
     * \param item duchain item that was encountered.
     * */
    void setEncountered(DUChainBase* item)
    {
        m_encountered.insert(item);
    }

    /**
     * Determine whether the given \a item is in the set of encountered items.
     *
     * @return true if the \a item has been encountered, otherwise false.
     * */
    bool wasEncountered(DUChainBase* item)
    {
        return m_encountered.contains(item);
    }

    /**
     * Set the current identifier to \a id.
     *
     * \param id the new current identifier.
     */
    void setIdentifier(const QString& id)
    {
        m_identifier = Identifier(id);
        m_qIdentifier.push(m_identifier);
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
     * Retrieve the current context stack.  This function is not expected
     * to be used often and may be phased out.
     *
     * \todo Audit whether access to the context stack is still required, and provide
     *       replacement functionality if possible.
     */
    const Stack<DUContext*>& contextStack() const
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

    virtual DUContext* openContextInternal(const RangeInRevision& range, DUContext::ContextType type,
                                           const QualifiedIdentifier& identifier)
    {
        Q_ASSERT(m_compilingContexts);
        DUContext* ret = nullptr;

        {
            if (recompiling()) {
                DUChainReadLocker readLock(DUChain::lock());
                const QVector<DUContext*>& childContexts = currentContext()->childContexts();

                int currentIndex = nextContextIndex();
                const auto indexedIdentifier = IndexedQualifiedIdentifier(identifier);

                for (; currentIndex < childContexts.count(); ++currentIndex) {
                    DUContext* child = childContexts.at(currentIndex);
                    RangeInRevision childRange = child->range();

                    if (child->type() != type) {
                        continue;
                    }
                    // We cannot update a contexts local scope identifier, that will break many other parts, like e.g.
                    // the CodeModel of child contexts or declarations.
                    // For unnamed child-ranges, we still do range-comparison, because we cannot distinguish them in other ways
                    if ((!identifier.isEmpty() && child->indexedLocalScopeIdentifier() == indexedIdentifier)
                        || (identifier.isEmpty() && child->indexedLocalScopeIdentifier().isEmpty() &&
                            !childRange.isEmpty() && childRange == range)) {
                        // Match
                        ret = child;
                        readLock.unlock();
                        DUChainWriteLocker writeLock(DUChain::lock());

                        ret->clearImportedParentContexts();
                        ++currentIndex;
                        break;
                    }
                }

                if (ret)
                    nextContextIndex() = currentIndex; //If we had a match, jump forward to that position
                ///@todo We should also somehow make sure we don't get quadratic worst-case effort while updating.
            }

            if (!ret) {
                DUChainWriteLocker writeLock(DUChain::lock());

                ret = newContext(range);
                ret->setType(type);

                if (!identifier.isEmpty())
                    ret->setLocalScopeIdentifier(identifier);

                setInSymbolTable(ret);
            } else {
                DUChainWriteLocker writeLock(DUChain::lock());
                Q_ASSERT(ret->localScopeIdentifier() == identifier);
                if (ret->parentContext())
                    ret->setRange(range);
            }
        }

        m_encountered.insert(ret);
        openContext(ret);
        return ret;
    }

    ///This function should call context->setInSymbolTable(..) with an appropriate decision. The duchain is write-locked when this is called.
    virtual void setInSymbolTable(DUContext* context)
    {
        if (!context->parentContext()->inSymbolTable()) {
            context->setInSymbolTable(false);
            return;
        }
        DUContext::ContextType type = context->type();
        context->setInSymbolTable(
            type == DUContext::Class || type == DUContext::Namespace || type == DUContext::Global || type == DUContext::Helper ||
            type == DUContext::Enum);
    }

    /// @returns the current url/path to the document we are parsing
    IndexedString document() const
    {
        return m_url;
    }

private:

    Identifier m_identifier;
    IndexedString m_url;
    QualifiedIdentifier m_qIdentifier;
    bool m_compilingContexts : 1;
    bool m_recompiling : 1;
    Stack<int> m_nextContextStack;
    DUContext* m_lastContext;
    //Here all valid declarations/uses/... will be collected
    QSet<DUChainBase*> m_encountered;
    Stack<DUContext*> m_contextStack;
};
}

#endif
