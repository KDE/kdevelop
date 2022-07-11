/*
    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMLJSDUCHAINHELPERS_H_
#define QMLJSDUCHAINHELPERS_H_

#include <language/duchain/declaration.h>
#include <qmljs/parser/qmljsast_p.h>

#include "duchainexport.h"

namespace QmlJS
{
using namespace KDevelop;

    /**
     * Act like TypeUtils::mergeTypes except that functions take precedence over
     * everything else (mergeTypes(a, function) returns function).
     */
    KDEVQMLJSDUCHAIN_EXPORT AbstractType::Ptr mergeTypes(AbstractType::Ptr type,
                                                         const AbstractType::Ptr& newType);

    /**
     * QML attribute value, with its source location
     */
    struct QMLAttributeValue
    {
        QString value;
        SourceLocation location;
    };

    /**
     * Find the declaration for the specified identifier.
     *
     * If no declaration was found, it will return a null pointer.
     *
     * @param id The qualified identifier that identifies our node.
     * @param context A pointer to the DUContext of this node.
     *
     * @note This method already acquires a read lock for the DUChain.
     */
    KDEVQMLJSDUCHAIN_EXPORT DeclarationPointer getDeclaration(const QualifiedIdentifier& id,
                                                              const DUContext* context,
                                                              bool searchInParent = true);

    /**
     * Find the declaration for the specified identifier, with "onFoo" replaced
     * with "Foo".
     *
     * This function is an extended version of getDeclaration and can resolve
     * QML slots to their corresponding signals.
     */
    KDEVQMLJSDUCHAIN_EXPORT DeclarationPointer getDeclarationOrSignal(const QualifiedIdentifier& id,
                                                                      const DUContext* context,
                                                                      bool searchInParent = true);

    /**
     * Find an attribute of a QML object
     */
    KDEVQMLJSDUCHAIN_EXPORT AST::Statement* getQMLAttribute(AST::UiObjectMemberList* members,
                                                            const QString &attribute);

    /**
     * Get the value of a QML string, identifier or boolean expression
     */
    KDEVQMLJSDUCHAIN_EXPORT QString getNodeValue(AST::Node* node);

    /**
     * Find the string value of a QML attribute
     */
    KDEVQMLJSDUCHAIN_EXPORT QMLAttributeValue getQMLAttributeValue(AST::UiObjectMemberList* members,
                                                                   const QString& attribute);

    /**
     * Get the internal context of a declaration
     */
    KDEVQMLJSDUCHAIN_EXPORT DUContext* getInternalContext(const DeclarationPointer &declaration);

    /**
     * Get the owner of a context
     *
     * If the context is of type Function and has no owner, then its parent context
     * is used to find the owner.
     */
    KDEVQMLJSDUCHAIN_EXPORT Declaration* getOwnerOfContext(const DUContext* context);

    /**
     * Build a zero-length range on the line of the given SourceLocation
     */
    KDEVQMLJSDUCHAIN_EXPORT RangeInRevision emptyRangeOnLine(const SourceLocation& location);

    /**
     * Import the internal context of a declaration in a context.
     *
     * @note The DUChain write lock must be held
     */
    KDEVQMLJSDUCHAIN_EXPORT void importDeclarationInContext(DUContext* context,
                                                            const DeclarationPointer& declaration);

    /**
     * Import the internal context of "Object" (the Javascript base type) in a context.
     *
     * @note The DUChain write lock must be held
     */
    KDEVQMLJSDUCHAIN_EXPORT void importObjectContext(DUContext* context,
                                                     TopDUContext* topContext);

    /**
     * Return whether an identifier is "prototype" or "__proto__" or any future
     * identifier that should be considered to represent a prototype.
     */
    KDEVQMLJSDUCHAIN_EXPORT bool isPrototypeIdentifier(const QString& identifier);

    /**
     * Return whether a context belongs to a QML file (not a JS one)
     */
    KDEVQMLJSDUCHAIN_EXPORT bool isQmlFile(const DUContext* context);

    KDEVQMLJSDUCHAIN_EXPORT void registerDUChainItems();
    KDEVQMLJSDUCHAIN_EXPORT void unregisterDUChainItems();


} // End of namespace QmlJS

#endif /* QMLJSDUCHAINHELPERS_H_ */
