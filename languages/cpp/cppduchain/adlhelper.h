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

/**
 * @brief Performs type traversal by ADLHelper in type traversal.
 */
class ADLTypeVisitor : public KDevelop::TypeVisitor
{
  public:
    explicit ADLTypeVisitor(ADLHelper & helper);

    virtual bool preVisit(const AbstractType * type);
    virtual void postVisit(const AbstractType *);

    virtual bool visit(const AbstractType* type);
    virtual void visit(const KDevelop::IntegralType * type);

    virtual bool visit(const KDevelop::PointerType * type);
    virtual void endVisit(const KDevelop::PointerType *);

    virtual bool visit(const KDevelop::ReferenceType * type);
    virtual void endVisit(const KDevelop::ReferenceType *);

    virtual bool visit(const KDevelop::FunctionType * type);
    virtual void endVisit(const KDevelop::FunctionType *);

    virtual bool visit(const KDevelop::StructureType * type);
    virtual void endVisit(const KDevelop::StructureType *);

    virtual bool visit(const KDevelop::ArrayType * type);
    virtual void endVisit(const KDevelop::ArrayType *);

  private:
    ADLHelper & m_helper;

    bool seen(const KDevelop::AbstractType* type);
    QSet<const KDevelop::AbstractType*> m_seen;
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
    QSet<Declaration*> associatedNamespaces() const;

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
     * All namespace declarations matching the given identifier are added.
     */
    void addAssociatedNamespace(const KDevelop::QualifiedIdentifier & identifier);

    /** @brief Adds an associated namespace declaration. */
    void addAssociatedNamespace(Declaration * declaration);

    /** @brief Computes all direct and indirect base classes of a class. */
    QList<Declaration *> computeAllBaseClasses(Declaration* decl);

    /** @brief Namespaces associated with the name lookup. */
    QSet<Declaration*> m_associatedNamespaces;

    KDevelop::DUContextPointer m_context;
    KDevelop::TopDUContextPointer m_topContext;

    friend class ADLTypeVisitor;
    ADLTypeVisitor m_typeVisitor;
};

}

#endif // CPP_ADLHELPER_H
