/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTUSEBUILDER_H
#define KDEVPLATFORM_ABSTRACTUSEBUILDER_H

#include "../declaration.h"
#include "../use.h"
#include "../topducontext.h"
#include "../duchain.h"
#include "../duchainlock.h"

#include <util/stack.h>

namespace KDevelop {
/**
 * \short Abstract definition-use chain use builder class
 *
 * The AbstractUseBuilder is a convenience class template for creating customized
 * definition-use chain use builders from an AST.  It simplifies creating or
 * modifying existing \ref Use "Uses"
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template <typename T, typename NameT, typename LanguageSpecificUseBuilderBase>
class AbstractUseBuilder
    : public LanguageSpecificUseBuilderBase
{
public:
    /// Constructor.
    template <class ParseSession>
    AbstractUseBuilder(ParseSession* session) : LanguageSpecificUseBuilderBase(session)
        , m_finishContext(true)
    {
    }

    AbstractUseBuilder()
        : m_finishContext(true)
    {
    }

    /**
     * Iterate an existing duchain, and add, remove or modify uses as determined
     * from the ast.
     *
     * \param node AST node to start visiting.
     */
    void buildUses(T* node)
    {
        auto* top = dynamic_cast<TopDUContext*>(this->contextFromNode(node));

        if (top) {
            DUChainWriteLocker lock(DUChain::lock());
            top->clearUsedDeclarationIndices();
            if (top->features() & TopDUContext::AllDeclarationsContextsAndUses)
                LanguageSpecificUseBuilderBase::setRecompiling(true);
        }

        LanguageSpecificUseBuilderBase::supportBuild(node);
    }

protected:

    struct ContextUseTracker
    {
        QVector<KDevelop::Use> createUses;
    };

    /**
     * Register a new use at the AST node @p name.
     *
     * @param name AST node which both represents a use and the identifier for the declaration which is being used.
     */
    void newUse(NameT* name)
    {
        QualifiedIdentifier id = this->identifierForNode(name);

        RangeInRevision newRange = this->editorFindRange(name, name);

        DUChainReadLocker lock(DUChain::lock());
        QList<Declaration*> declarations = LanguageSpecificUseBuilderBase::currentContext()->findDeclarations(id,
                                                                                                              newRange.start);
        for (Declaration* declaration : std::as_const(declarations)) {
            if (!declaration->isForwardDeclaration()) {
                declarations.clear();
                declarations.append(declaration);
                break;
            }
        }

        // If we don't break, there's no non-forward declaration

        lock.unlock();
        newUse(newRange, !declarations.isEmpty() ? DeclarationPointer(declarations.first()) : DeclarationPointer());
    }

    ///@todo Work this over! We must not pass around "Declaration*" values if the duchain is not locked.

    /**
     * Register a new use for a \a declaration with a \a node.
     *
     * \param node Node which encompasses the use.
     * \param declaration Declaration which is being used. May be null when a declaration cannot be found for the use.
     */
    void newUse(T* node, const KDevelop::DeclarationPointer& declaration)
    {
        newUse(this->editorFindRange(node, node), declaration);
    }

    /**
     * Register a new use.
     *
     * \param newRange Text range which encompasses the use.
     * \param _declaration Declaration which is being used. May be null when a declaration cannot be found for the use.
     */
    void newUse(const RangeInRevision& newRange, const DeclarationPointer& _declaration)
    {
        DUChainWriteLocker lock(DUChain::lock());
        Declaration* declaration = _declaration.data();

        if (!declaration)
            return; // The declaration was deleted in the meantime

        int declarationIndex = LanguageSpecificUseBuilderBase::currentContext()->topContext()->indexForUsedDeclaration(
            declaration);
        int contextUpSteps = 0; //We've got to use the stack here, and not parentContext(), because the order may be different

        {
            /*
             * We need to find a context that this use fits into, which must not necessarily be the current one.
             * The reason are macros like SOME_MACRO(SomeClass), where SomeClass is expanded to be within a
             * sub-context that comes from the macro. That sub-context will have a very small range, and will most
             * probably not be the range of the actual "SomeClass" text, so the "SomeClass" use has to be moved
             * into the context that surrounds the SOME_MACRO invocation.
             * */
            DUContext* newContext = LanguageSpecificUseBuilderBase::currentContext();
            while (!newContext->range().contains(newRange) &&
                   contextUpSteps < (LanguageSpecificUseBuilderBase::contextStack().size() - 1)) {
                ++contextUpSteps;
                newContext =
                    LanguageSpecificUseBuilderBase::contextStack()[LanguageSpecificUseBuilderBase::contextStack().size()
                                                                   - 1 - contextUpSteps];
            }

            if (contextUpSteps) {
                m_finishContext = false;
                openContext(newContext);
                m_finishContext = true;
                currentUseTracker() = m_trackerStack.at(m_trackerStack.size() - contextUpSteps - 2);
            }

            currentUseTracker().createUses << KDevelop::Use(newRange, declarationIndex);
        }

        if (contextUpSteps) {
            Q_ASSERT(
                m_contexts[m_trackerStack.size() - contextUpSteps - 2] ==
                LanguageSpecificUseBuilderBase::currentContext());
            m_trackerStack[m_trackerStack.size() - contextUpSteps - 2] = currentUseTracker();
            m_finishContext = false;
            closeContext();
            m_finishContext = true;
        }
    }

    /**
     * Reimplementation of openContext, to track which uses should be assigned to which context.
     */
    void openContext(KDevelop::DUContext* newContext) override
    {
        LanguageSpecificUseBuilderBase::openContext(newContext);

        ContextUseTracker newTracker;
        m_trackerStack.push(newTracker);
        m_contexts.push(newContext);
    }

    /**
     * Reimplementation of closeContext, to track which uses should be assigned to which context.
     */
    void closeContext() override
    {
        if (m_finishContext) {
            DUChainWriteLocker lock(DUChain::lock());

            this->currentContext()->deleteUses();

            ContextUseTracker& tracker(currentUseTracker());
            for (auto& createUse : tracker.createUses) {
                this->currentContext()->createUse(createUse.m_declarationIndex,
                                                  createUse.m_range);
            }
        }

        LanguageSpecificUseBuilderBase::closeContext();

        m_trackerStack.pop();
        m_contexts.pop();
    }

private:
    inline ContextUseTracker& currentUseTracker() { return m_trackerStack.top(); }
    Stack<ContextUseTracker> m_trackerStack;
    Stack<KDevelop::DUContext*> m_contexts;

    //Whether not encountered uses should be deleted during closeContext()
    bool m_finishContext;
};
}

#endif // KDEVPLATFORM_ABSTRACTUSEBUILDER_H
