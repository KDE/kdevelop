/*
    SPDX-FileCopyrightText: 2006-2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ABSTRACTTYPEBUILDER_H
#define KDEVPLATFORM_ABSTRACTTYPEBUILDER_H

#include "../types/structuretype.h"
#include "../declaration.h"
#include "../duchain.h"
#include "../duchainlock.h"

namespace KDevelop {
/**
 * \short Abstract definition-use chain type builder class
 *
 * The AbstractTypeBuilder is a convenience class template for creating customized
 * definition-use chain type builders from an AST.  It simplifies:
 * - creating and re-using types
 * - creating complex types in a stack
 * - referencing named types
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template <typename T, typename NameT, typename LangugageSpecificTypeBuilderBase>
class AbstractTypeBuilder
    : public LangugageSpecificTypeBuilderBase
{
public:
    /**
     * Returns the list of types that were created in the parsing run, excluding subtypes
     * (ie. returns complete types, not the simple types and intermediate types which
     * went into creating any complex type)
     *
     * Used for unit tests only.
     */
    const QList<AbstractType::Ptr>& topTypes() const
    {
        return m_topTypes;
    }

protected:
    /**
     * Determine the context to search in when named types are requested.
     * You may reimplement this to return a different context if required.
     *
     * \returns the context in which to search for named types.
     */
    virtual DUContext* searchContext() const
    {
        return LangugageSpecificTypeBuilderBase::currentContext();
    }

    /**
     * Notify that a class type was opened.
     *
     * This should be called at the beginning of processing a class-specifier, right after the type for the class was created.
     * The type can be retrieved through currentAbstractType().
     */
    virtual void classTypeOpened(const KDevelop::AbstractType::Ptr&) {}

    /**
     * Perform initialisation at the start of a build, and check that all types
     * that were registered were also used.
     */
    void supportBuild(T* node, DUContext* context = nullptr) override
    {
        m_topTypes.clear();

        LangugageSpecificTypeBuilderBase::supportBuild(node, context);

        Q_ASSERT(m_typeStack.isEmpty());
    }

    /**
     * Retrieve the last type that was encountered.
     * \returns the last encountered type.
     */
    AbstractType::Ptr lastType() const
    {
        return m_lastType;
    }

    /**
     * Set the last encountered type.
     *
     * \param ptr pointer to the last encountered type.
     */
    void setLastType(const AbstractType::Ptr& ptr)
    {
        m_lastType = ptr;
    }

    /// Clear the last encountered type.
    void clearLastType()
    {
        m_lastType = nullptr;
    }

    /**
     * Simulates that the given type was created.
     * After calling, this type will be the last type.
     * */
    void injectType(const AbstractType::Ptr& type)
    {
        openType(type);
        closeType();
    }

    /**
     * Opens the given \a type, and sets it to be the current type.
     */
    void openType(const AbstractType::Ptr& type)
    {
        openAbstractType(type);
    }

    /**
     * Opens the given \a type, and sets it to be the current type.
     */
    void openAbstractType(const AbstractType::Ptr& type)
    {
        m_typeStack.append(type);
    }

    /**
     * Close the current type.
     */
    void closeType()
    {
        m_lastType = currentAbstractType();

        bool replaced = m_lastType != currentAbstractType();

        // And the reference will be lost...
        m_typeStack.pop();

        if (!hasCurrentType() && !replaced)
            m_topTypes.append(m_lastType);
    }

    /// Determine if the type builder is currently parsing a type. \returns true if there is a current type, else returns false.
    inline bool hasCurrentType() { return !m_typeStack.isEmpty(); }

    /**
     * Retrieve the current type being parsed.
     *
     * \warning You must not use this in creating another type definition, as it may not be a registered type.
     *
     * \returns the current abstract type being parsed.
     */
    inline AbstractType::Ptr currentAbstractType()
    {
        if (m_typeStack.isEmpty()) {
            return AbstractType::Ptr();
        } else {
            return m_typeStack.top();
        }
    }

    /**
     * Retrieve the current type being parsed.
     *
     * \warning You must not use this in creating another type definition, as it may not be a registered type.
     *
     * \returns the current type being parsed.
     */
    template<class T2>
    TypePtr<T2> currentType()
    {
        return currentAbstractType().template dynamicCast<T2>();
    }

    /**
     * Search for a type with the identifier given by \a name.
     *
     * \param name the AST node representing the name of the type to open.
     * \param needClass if true, only class types will be searched, if false all named types will be searched.
     *
     * \returns whether a type was found (and thus opened).
     */
    bool openTypeFromName(NameT* name, bool needClass)
    {
        return openTypeFromName(this->identifierForNode(name), name, needClass);
    }

    /**
     * Search for a type with the identifier given by \a name.
     *
     * \param id the identifier of the type for which to search.
     * \param typeNode the AST node representing the type to open.
     * \param needClass if true, only class types will be searched, if false all named types will be searched.
     *
     * \returns whether a type was found (and thus opened).
     */
    bool openTypeFromName(const QualifiedIdentifier& id, T* typeNode, bool needClass)
    {
        bool openedType = false;

        bool delay = false;

        if (!delay) {
            CursorInRevision pos(this->editorFindRange(typeNode, typeNode).start);
            DUChainReadLocker lock(DUChain::lock());

            const QList<Declaration*> dec = searchContext()->findDeclarations(id, pos);

            if (dec.isEmpty())
                delay = true;

            if (!delay) {
                for (Declaration* decl : dec) {
                    if (needClass && !decl->abstractType().dynamicCast<StructureType>())
                        continue;

                    if (decl->abstractType()) {
                        ///@todo only functions may have multiple declarations here
                        //ifDebug( if( dec.count() > 1 ) qDebug() << id.toString() << "was found" << dec.count() << "times" )
                        //qDebug() << "found for" << id.toString() << ":" << decl->toString() << "type:" << decl->abstractType()->toString() << "context:" << decl->context();
                        openedType = true;
                        openType(decl->abstractType());
                        break;
                    }
                }
            }

            if (!openedType)
                delay = true;
        }
        ///@todo What about position?

        /*if(delay) {
           //Either delay the resolution for template-dependent types, or create an unresolved type that stores the name.
           openedType = true;
           openDelayedType(id, name, templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );

           ifDebug( if(templateDeclarationDepth() == 0) qDebug() << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext() )
           }*/
        return openedType;
    }

private:
    Stack<AbstractType::Ptr> m_typeStack;

    AbstractType::Ptr m_lastType;

    QList<AbstractType::Ptr> m_topTypes;
};
}

#endif // KDEVPLATFORM_ABSTRACTTYPEBUILDER_H
