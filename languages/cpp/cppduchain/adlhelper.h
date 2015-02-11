/*
    Copyright (C) 2010 Ciprian Ciubotariu <cheepeero@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef CPP_ADLHELPER_H
#define CPP_ADLHELPER_H

#include <QSet>
#include "overloadresolution.h"
#include <language/duchain/types/typesystem.h>

namespace Cpp
{

  using namespace KDevelop;

  class ADLHelper;

/**
 * @brief Performs type traversal by ADLHelper in type traversal.
 */
class ADLTypeVisitor : public KDevelop::TypeVisitor
{
  public:
    explicit ADLTypeVisitor(ADLHelper & helper);

    virtual bool preVisit(const AbstractType * type) override;
    virtual void postVisit(const AbstractType *) override;

    virtual bool visit(const AbstractType* type) override;
    virtual void visit(const KDevelop::IntegralType * type) override;

    virtual bool visit(const KDevelop::PointerType * type) override;
    virtual void endVisit(const KDevelop::PointerType *) override;

    virtual bool visit(const KDevelop::ReferenceType * type) override;
    virtual void endVisit(const KDevelop::ReferenceType *) override;

    virtual bool visit(const KDevelop::FunctionType * type) override;
    virtual void endVisit(const KDevelop::FunctionType *) override;

    virtual bool visit(const KDevelop::StructureType * type) override;
    virtual void endVisit(const KDevelop::StructureType *) override;

    virtual bool visit(const KDevelop::ArrayType * type) override;
    virtual void endVisit(const KDevelop::ArrayType *) override;

  private:
    ADLHelper & m_helper;

    bool seen(const KDevelop::AbstractType* type);
};


/**
 * @brief Used for implementing ADL lookup.
 * See ISO 14882.2003, section 3.4.2 Argument-dependent name lookup [basic.lookup.koenig]
 *
 * Add all function arguments (or their types) to an object.
 * Uninteresting arguments and types are skipped by the helper.
 * If an argument is found to match conditions for ADL it is added to the associated namespace list.
 *
 * Already seen types/classes/function declarations are skipped by the ADLTypeVisitor object.
 */
class ADLHelper
{
  public:
    /**
     * @copydoc Cpp::OverloadResolver::OverloadResolver()
     */
    ADLHelper(DUContextPointer context, TopDUContextPointer topContext);

    /** @brief Adds an function argument for lookup. */
    void addArgument(const OverloadResolver::Parameter & argument);

    /** @brief Adds an function argument type for lookup. */
    void addArgumentType(const AbstractType::Ptr type);

    /** @brief Retrieves the list of associated namespaces . */
    QSet<QualifiedIdentifier> associatedNamespaces() const;

  private:

    /**
     * @brief Adds an associated class type and its associated classes.
     *
     * If T is a class type (including unions), its associated classes are: the class itself; the class of which it is a
     * member, if any; and its direct and indirect base classes. Its associated namespaces are the namespaces
     * in which its associated classes are defined.
     */
    void addAssociatedClass(Declaration * declaration);

    /**
     * @brief Adds an associated namespace by identifier.
     */
    void addAssociatedNamespace(const KDevelop::QualifiedIdentifier & identifier);

    void addBaseClasses(Declaration* decl);

    /** @brief Finds and adds the namespace of a declaration. */
    void addDeclarationScopeIdentifier(Declaration * decl);

  private:
    /** @brief List of namespaces found by the ADL. */
    QSet<QualifiedIdentifier> m_associatedNamespaces;
    
    // AbstractType* or Declaration*
    QSet<const void*> m_alreadyProcessed;

    KDevelop::DUContextPointer m_context;
    KDevelop::TopDUContextPointer m_topContext;

    friend class ADLTypeVisitor;
    ADLTypeVisitor m_typeVisitor;

    /**
     * @brief Flags processing of the arguments of a template type.
     *
     * If equal to 0, we are processing the function call arguments.
     * If greater than 0, we are processing template arguments of one of the function call arguments.
     * The value itself represents the instantiation depth of template arguments.
     */
    int m_templateArgsDepth;

    /**
     * @brief Possibly the function named used as argument by the caller.
     * May be @c NULL .
     * See the comments inside @code ADLTypeVisitor::endVisit(const FunctionType *) @endcode for details.
     */
    IndexedDeclaration m_possibleFunctionName;
};

}

#endif // CPP_ADLHELPER_H
