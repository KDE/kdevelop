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

#include "adlhelper.h"
#include <language/duchain/types/alltypes.h>
#include <language/duchain/classdeclaration.h>
#include "templatedeclaration.h"
#include "typeutils.h"

// uncomment to get debugging info on ADL - very expensive on parsing
//#define DEBUG_ADL

using namespace Cpp;
using namespace KDevelop;

ADLTypeVisitor::ADLTypeVisitor(ADLHelper & helper) : m_helper(helper)
{
}

bool ADLTypeVisitor::preVisit(const AbstractType * type)
{
    // the following types are of no interest to ADL
    switch (type->whichType())
    {
    case AbstractType::TypeAbstract:
    case AbstractType::TypeIntegral:
    case AbstractType::TypeDelayed:
    case AbstractType::TypeUnsure:
        return false;
    default:
        return true;
    };
}

void ADLTypeVisitor::postVisit(const AbstractType *)
{
}

bool ADLTypeVisitor::visit(const AbstractType* type)
{
    return !seen(type);
}

void ADLTypeVisitor::visit(const IntegralType *)
{
}

bool ADLTypeVisitor::visit(const PointerType * type)
{
    return !seen(type);
}

void ADLTypeVisitor::endVisit(const PointerType *)
{
    // traversed by PointerType::accept0
}

bool ADLTypeVisitor::visit(const ReferenceType * type)
{
    return !seen(type);
}

void ADLTypeVisitor::endVisit(const ReferenceType *)
{
    // traversed by ReferenceType::accept0
}

bool ADLTypeVisitor::visit(const FunctionType * type)
{
    if (m_helper.m_templateArgsDepth > 0)
        return false;

    return !seen(type);
}

void ADLTypeVisitor::endVisit(const FunctionType * /*type*/)
{
    // return type and argument types are handled by FunctionType::accept0

    // here we process the namespace of the function name (or containing class), if any

    /* at the bottom of 3.4.2.2 we find the following:

    In addition, if the argument is the name or address of a set of overloaded functions and/or function tem-
    plates, its associated classes and namespaces are the union of those associated with each of the members of
    the set: the namespace in which the function or function template is defined and the classes and namespaces
    associated with its (non-dependent) parameter types and return type.
    */

    if (m_helper.m_possibleFunctionName.data() && m_helper.m_possibleFunctionName.data()->isFunctionDeclaration())
    {
        Declaration * declaration = m_helper.m_possibleFunctionName.data();

#ifdef DEBUG_ADL
        kDebug() << "    function name = " << declaration->toString() << " ; identifier = " << declaration->qualifiedIdentifier().toString();
#endif

        // start going towards the global scope until we match an interesting name
        // note that calling addDeclarationScopeIdentifier does not work because for some reason
        // for function names DUContext::scopeIdentifier returns the function name instead of the
        // name of the function's scope
        DUContext* context = declaration->context();
        while (context) {
            if (Declaration* decl = context->owner())
            {
              if (context->type() == DUContext::Namespace)
              {
                  m_helper.addAssociatedNamespace(decl->qualifiedIdentifier());
                  break;
              } else if (context->type() == DUContext::Class) {
                  m_helper.addAssociatedClass(decl);
                  break;
              }
            }
            context = context->parentContext();
        }
    }
}

bool ADLTypeVisitor::visit(const StructureType * type)
{
    return !seen(type);
}

void ADLTypeVisitor::endVisit(const StructureType * type)
{
    // StructureType does not visit base classes etc
    // so the processing is done by ADLHelper
    m_helper.addAssociatedClass(type->declaration(m_helper.m_topContext.data()));
}

bool ADLTypeVisitor::visit(const ArrayType * type)
{
    return !seen(type);
}

void ADLTypeVisitor::endVisit(const ArrayType *)
{
    // traversed by ArrayType::accept0
}

bool ADLTypeVisitor::seen(const KDevelop::AbstractType* type)
{
    if (m_helper.m_alreadyProcessed.contains(type))
        return true;

    m_helper.m_alreadyProcessed.insert(type);
    return false;
}

ADLHelper::ADLHelper(DUContextPointer context, TopDUContextPointer topContext)
        : m_context(context), m_topContext(topContext),
        m_typeVisitor(*this),
        m_templateArgsDepth(0)
{
}

void ADLHelper::addArgument(const OverloadResolver::Parameter & argument)
{
    m_possibleFunctionName = argument.declaration;
    addArgumentType(argument.type);
}

void ADLHelper::addArgumentType(const AbstractType::Ptr typePtr)
{
    if(m_alreadyProcessed.contains(typePtr.data()))
      return;
    
    if (typePtr)
    {
#ifdef DEBUG_ADL
        kDebug() << "    added argument type " << typePtr->toString() << " to ADL lookup";
#endif
        // the enumeration and enumerator types are not part of the TypeVisitor interface
        switch (typePtr->whichType())
        {
        case AbstractType::TypeEnumeration:
        {
            EnumerationType* specificType = fastCast<EnumerationType*>(typePtr.data());
            if (specificType)
            {
                Declaration * enumDecl = specificType->declaration(m_topContext.data());
                addDeclarationScopeIdentifier(enumDecl);
            }
            break;
        }
        case AbstractType::TypeEnumerator:
        {
            if (m_templateArgsDepth == 0)
            {
                EnumeratorType* specificType = fastCast<EnumeratorType*>(typePtr.data());
                if (specificType)
                {
                    // use the enumeration context for the enumerator value declaration to find out the namespace
                    Declaration * enumeratorDecl = specificType->declaration(m_topContext.data());
                    if (enumeratorDecl) {
                        DUContext * enumContext = enumeratorDecl->context();
                        if (enumContext) {
                            addAssociatedNamespace(enumContext->scopeIdentifier(false));
                        }
                    }
                }
            }
            break;
        }
        default:
            typePtr->accept(&m_typeVisitor);
        }
    }
    
    m_alreadyProcessed.insert(typePtr.data());
}

QSet< QualifiedIdentifier > ADLHelper::associatedNamespaces() const
{
    return m_associatedNamespaces;
}

void ADLHelper::addAssociatedClass(Declaration * declaration)
{
    if (!declaration || !m_context || !m_topContext)
        return;

    // from the standard:
    // Typedef names and using-declarations used to specify types do not contribute to this set.
    if (declaration->isTypeAlias())
        return;

    if(m_alreadyProcessed.contains(declaration))
      return;
    
    m_alreadyProcessed.insert(declaration);
    
    addDeclarationScopeIdentifier(declaration);

    /*
    Here comes the standard on template ADL, along with my interpretation:

    This works just like for classes or class members:
      If T is a template-id, its associated namespaces and classes are the namespace in which the template is
      defined; for member templates, the member template's class;

    This means that if the template's argument T is a type, treat it just like any other function argument.
    GCC seems to include fully instantiated templates (which are a type indeed) here, so I followed the same
    interpretation.
      the namespaces and classes associated
      with the types of the template arguments provided for template type parameters (excluding template
      template parameters);

    If the template's argument T is a template, take just its namespace, and not associated classes or
    the namespaces associated with its own arguments.
      the namespaces in which any template template arguments are defined; and the
      classes in which any member templates used as template template arguments are defined.

    This means when we are processing template arguments we need to skip numeric template arguments,
    i.e. enumeration values (not types!), function pointers, integral values etc
      [Note: non-
      type template arguments do not contribute to the set of associated namespaces. ]
    */

    TemplateDeclaration * templateDecl = dynamic_cast<TemplateDeclaration*>(declaration);
    bool isFunctionArg = (m_templateArgsDepth == 0);
    bool isTemplateClassArg = (m_templateArgsDepth > 0 && !templateDecl);
    bool isTemplateTemplateArg = (templateDecl ? (templateDecl->instantiatedFrom() == NULL) : false);
    if (isFunctionArg || (isTemplateClassArg && !isTemplateTemplateArg))
        addBaseClasses(declaration);

    if (templateDecl && !isTemplateTemplateArg)
    {
        ///@todo Probably the parent has to be considered too (see previousInstantiationInformation), ouch
        m_templateArgsDepth++;
        const InstantiationInformation& instantiationInfo = templateDecl->instantiatedWith().information();
        for (unsigned int i = 0, n = instantiationInfo.templateParametersSize(); i < n; ++i)
        {
            const IndexedType & rType = instantiationInfo.templateParameters()[i];
            addArgumentType(rType.abstractType());
        }
        m_templateArgsDepth--;
    }
}

void ADLHelper::addDeclarationScopeIdentifier(Declaration * decl)
{
    if(decl)
        addAssociatedNamespace(decl->context()->scopeIdentifier(false));
}

void ADLHelper::addAssociatedNamespace(const KDevelop::QualifiedIdentifier& identifier)
{
#ifdef DEBUG_ADL
        kDebug() << "    adding namespace " << identifier.identifier().toString();
#endif
    if(identifier.count())
      m_associatedNamespaces.insert(identifier);
}

void ADLHelper::addBaseClasses(Declaration* declaration)
{
    ClassDeclaration * classDecl = dynamic_cast<ClassDeclaration*>(declaration);
    if (classDecl)
    {
        int nBaseClassesCount = classDecl->baseClassesSize();
        for (int i = 0; i < nBaseClassesCount; ++i)
        {
            const BaseClassInstance baseClass = classDecl->baseClasses()[i];
            StructureType::Ptr type = baseClass.baseClass.type<StructureType>();
            if (type)
                addAssociatedClass(type->declaration(m_topContext.data()));
        }
    }
}
