/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTDECLARATIONBUILDER_H
#define KDEVPLATFORM_ABSTRACTDECLARATIONBUILDER_H

#include <typeinfo>

#include "../classfunctiondeclaration.h"
#include "../forwarddeclaration.h"
#include "../types/identifiedtype.h"
#include "../functiondeclaration.h"

namespace KDevelop {
class Declaration;

/**
 * A class which iterates the AST to extract definitions of types.
 */
template <typename T, typename NameT, typename LanguageSpecificDeclarationBuilderBase>
class AbstractDeclarationBuilder
    : public LanguageSpecificDeclarationBuilderBase
{
protected:
    /// Determine if there is currently a declaration open. \returns true if a declaration is open, otherwise false.
    inline bool hasCurrentDeclaration() const { return !m_declarationStack.isEmpty(); }
    /// Access the current declaration. \returns the current declaration, or null if there is no current declaration.
    inline Declaration* currentDeclaration() const
    {
        return m_declarationStack.isEmpty() ? nullptr : m_declarationStack.top();
    }
    /// Access the current declaration, casted to type \a DeclarationType. \returns the current declaration if one exists and is an instance of the given \a DeclarationType.
    template <class DeclarationType>
    inline DeclarationType* currentDeclaration() const
    {
        return m_declarationStack.isEmpty() ? nullptr : dynamic_cast<DeclarationType*>(m_declarationStack.top());
    }

    /// Access the current comment. \returns the current comment, or an empty string if none exists.
    inline const QByteArray& comment() const { return m_lastComment; }
    /// Set the current \a comment. \param comment the new comment.
    inline void setComment(const QByteArray& comment) { m_lastComment = comment; }
    /// Clears the current comment.
    inline void clearComment() { m_lastComment.clear(); }

    enum DeclarationFlags {
        NoFlags    = 0x0,
        DeclarationIsDefinition = 0x1
    };

    /**
     * Register a new declaration with the definition-use chain
     * \param name When this is zero, the identifier given through customName is used
     * \param range provide a valid AST node here if name is null
     * \param flags equal to <b>DeclarationIsDefinition</b> whether the new declaration is also a definition
     * \return the new declaration created
     */
    template <class DeclarationT>
    DeclarationT* openDeclaration(NameT* name, T* range, DeclarationFlags flags = NoFlags)
    {
        DUChainWriteLocker lock(DUChain::lock());

        RangeInRevision newRange = this->editorFindRange(name ? name : range, name ? name : range);

        QualifiedIdentifier id = this->identifierForNode(name);

        return openDeclaration<DeclarationT>(id, newRange, flags);
    }

    /**
     * \copydoc openDeclaration
     *
     * \param id the identifier of the new declaration.
     * \param newRange the range which the identifier for the new declaration occupies.
     * \param flags equal to <b>DeclarationIsDefinition</b> whether the new declaration is also a definition
     * \return the new declaration created
     */
    template <class DeclarationT>
    Q_DECL_DEPRECATED_X("Use openDeclaration(Identifier) instead")
    DeclarationT * openDeclaration(const QualifiedIdentifier &id, const RangeInRevision &newRange,
                                   DeclarationFlags flags = NoFlags)
    {
        if (id.count() > 1) {
            qWarning() << "openDeclaration called with a multi-component QualifiedIdentifier!";
        }
        Identifier localId;

        if (!id.isEmpty()) {
            localId = id.last();
        }

        return openDeclaration<DeclarationT>(localId, newRange, flags);
    }
    /**
     * \copydoc openDeclaration
     *
     * \param localId the identifier of the new declaration.
     * \param newRange the range which the identifier for the new declaration occupies.
     * \param flags equal to <b>DeclarationIsDefinition</b> whether the new declaration is also a definition
     * \return the new declaration created
     */
    template <class DeclarationT>
    DeclarationT* openDeclaration(const Identifier& localId, const RangeInRevision& newRange,
                                  DeclarationFlags flags = NoFlags)
    {
        DeclarationT* declaration = nullptr;

        if (LanguageSpecificDeclarationBuilderBase::recompiling()) {
            // Seek a matching declaration

            const QList<Declaration*> declarations =
                LanguageSpecificDeclarationBuilderBase::currentContext()->findLocalDeclarations(localId,
                                                                                                CursorInRevision::invalid(),
                                                                                                this->topContext(),
                                                                                                AbstractType::Ptr(),
                                                                                                DUContext::NoFiltering);
            for (Declaration* dec : declarations) {
                if (LanguageSpecificDeclarationBuilderBase::wasEncountered(dec))
                    continue;

                if (dec->range() == newRange &&
                    (localId == dec->identifier() || (localId.isUnique() && dec->identifier().isUnique())) &&
                    typeid(*dec) == typeid(DeclarationT)
                    //&& extraDeclarationComparisons()
                ) {
                    // Match
                    declaration = dynamic_cast<DeclarationT*>(dec);
                    break;
                }
            }
        }

        if (!declaration) {
            declaration = new DeclarationT(newRange, LanguageSpecificDeclarationBuilderBase::currentContext());

            if (flags & DeclarationIsDefinition)
                declaration->setDeclarationIsDefinition(true);
            declaration->setIdentifier(localId);
        }

        declaration->setComment(m_lastComment);
        m_lastComment.clear();

        LanguageSpecificDeclarationBuilderBase::setEncountered(declaration);

        openDeclarationInternal(declaration);

        return declaration;
    }

    /// Convenience function. Same as openDeclaration(), but creates the declaration as a definition.
    template <class DeclarationT>
    DeclarationT* openDefinition(NameT* name, T* range)
    {
        return openDeclaration<DeclarationT>(name, range, DeclarationIsDefinition);
    }

    /// Convenience function. Same as openDeclaration(), but creates the declaration as a definition.
    template <class DeclarationT>
    DeclarationT* openDefinition(const QualifiedIdentifier& id, const RangeInRevision& newRange)
    {
        return openDeclaration<DeclarationT>(id, newRange, DeclarationIsDefinition);
    }

    /// Internal function to open the given \a declaration by pushing it onto the declaration stack.
    /// Provided for subclasses who don't want to use the generic openDeclaration() functions.
    void openDeclarationInternal(Declaration* declaration)
    {
        m_declarationStack.push(declaration);
    }

    /// Convenience function. Same as openDeclaration(), but creates a forward declaration.
    ForwardDeclaration* openForwardDeclaration(NameT* name, T* range)
    {
        return openDeclaration<ForwardDeclaration>(name, range);
    }

    /// Set the internal context of a declaration; for example, a class declaration's internal context
    /// is the context inside the brackets: class ClassName { ... }
    void eventuallyAssignInternalContext()
    {
        if (LanguageSpecificDeclarationBuilderBase::lastContext()) {
            DUChainWriteLocker lock(DUChain::lock());

            if (dynamic_cast<ClassFunctionDeclaration*>(currentDeclaration()))
                Q_ASSERT(
                    !static_cast<ClassFunctionDeclaration*>(currentDeclaration())->isConstructor() ||
                    currentDeclaration()->context()->type() == DUContext::Class);

            if (LanguageSpecificDeclarationBuilderBase::lastContext() &&
                (LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Class ||
                 LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Other ||
                 LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Function ||
                 LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Template ||
                 LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Enum ||
                 (LanguageSpecificDeclarationBuilderBase::lastContext()->type() == DUContext::Namespace &&
                  currentDeclaration()->kind() == Declaration::Namespace)
                )) {
                if (!LanguageSpecificDeclarationBuilderBase::lastContext()->owner() ||
                    !LanguageSpecificDeclarationBuilderBase::wasEncountered(LanguageSpecificDeclarationBuilderBase::
                                                                            lastContext()->owner())) {                                                                                                     //if the context is already internalContext of another declaration, leave it alone
                    currentDeclaration()->setInternalContext(LanguageSpecificDeclarationBuilderBase::lastContext());

                    LanguageSpecificDeclarationBuilderBase::clearLastContext();
                }
            }
        }
    }

    /// Close a declaration. Virtual to allow subclasses to perform customisations to declarations.
    virtual void closeDeclaration()
    {
        m_declarationStack.pop();
    }

    /// Abort a declaration, deleting it.
    void abortDeclaration()
    {
        delete m_declarationStack.pop();
    }

private:
    Stack<Declaration*> m_declarationStack;
    QByteArray m_lastComment;
};
}

#endif // KDEVPLATFORM_ABSTRACTDECLARATIONBUILDER_H
