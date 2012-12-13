/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CPPJSONTESTS_H
#define CPPJSONTESTS_H

#include <cpptypes.h>
#include <typeutils.h>
#include <cppduchain.h>
#include <templatedeclaration.h>
#include <templateparameterdeclaration.h>

#include <tests/json/jsontesthelpers.h>

/**
 * JSON Object Specification:
 *   InstantiationObject : Mapping of instantiation strings to DeclTestObjects
 *                         Instantiation strings are formatted: "<ClassA, ClassB, ...>::<ClassC, ClassD, ...>"
 *
 * Quick Reference:
 *   instantiatedFrom : DeclTestObject
 *   specializedFrom : DeclTestObject
 *   instantiations : InstantiationObject
 *   realType : TypeTestObject
 *   shortenedType : TypeTestObject
 *   templateContext : CtxtTestObject
 *   templateParameterDefault : string
 */

namespace Cpp
{

namespace DeclarationTests
{

using namespace JsonTestHelpers;
using namespace KDevelop;

///JSON type: DeclTestObject
///@returns whether the declaration is instantiated and the declaration from which it is instantiated passes the tests
DeclarationTest(instantiatedFrom)
{
  const QString NOT_INSTANTIATED = "\"%1\" is not an instantiated declaration.";

  TemplateDeclaration *templDecl = dynamic_cast<TemplateDeclaration*>(decl);
  if (!templDecl)
    return NOT_INSTANTIATED.arg(decl->qualifiedIdentifier().toString());

  Declaration *instFrom = dynamic_cast<Declaration*>(templDecl->instantiatedFrom());
  if (!instFrom)
    return NOT_INSTANTIATED.arg(decl->qualifiedIdentifier().toString());

  return testObject(instFrom, value, "Declaration's instantiatedFrom");
}
///JSON type: DeclTestObject
///@returns whether the declaration is specialized and the declaration from which it is specialized passes the tests
DeclarationTest(specializedFrom)
{
  const QString NOT_SPECIALIZED = "\"%1\" is not a specialized declaration.";

  TemplateDeclaration *templDecl = dynamic_cast<TemplateDeclaration*>(decl);
  if (!templDecl)
    return NOT_SPECIALIZED.arg(decl->qualifiedIdentifier().toString());

  Declaration *specFrom = templDecl->specializedFrom().declaration();
  if (!specFrom)
    return NOT_SPECIALIZED.arg(decl->qualifiedIdentifier().toString());

  return testObject(specFrom, value, "Declaration's specializedFrom");
}
///JSON type: InstantiationObject
///@returns whether the instantiations are correct and the instantiation declaration tests pass
DeclarationTest(instantiations)
{
  VERIFY_TYPE(QVariantMap);
  const QString COUNT_ERROR = "Number of instantiations (%1) doesn't match test data (%2).";
  const QString INST_ERROR = "Declaration has instantiation not found in test data: \"%1\".";
  const QString DECL_ERROR = "Declaration instantiated with \"%1\" didn't pass tests.";
  TemplateDeclaration *templDecl = dynamic_cast<TemplateDeclaration*>(decl);
  TemplateDeclaration::InstantiationsHash declInstantiations = templDecl->instantiations();
  QVariantMap testInstantiations = value.toMap();
  if (testInstantiations.size() != declInstantiations.size())
    return COUNT_ERROR.arg(declInstantiations.size()).arg(testInstantiations.size());

  for (TemplateDeclaration::InstantiationsHash::Iterator it = declInstantiations.begin(); it != declInstantiations.end(); ++it)
  {
    QString instStr = it.key().information().toString();
    if (!testInstantiations.contains(instStr))
      return INST_ERROR.arg(instStr);

    if (!runTests(testInstantiations[instStr].toMap(), dynamic_cast<Declaration*>(it.value())))
      return DECL_ERROR.arg(instStr);
  }
  return SUCCESS;
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's real type pass (see TypeUtils::realType)
DeclarationTest(realType)
{
  return testObject(TypeUtils::realType(decl->abstractType()), value, "Declaration's real type");
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's shortened type pass (see shortenTypeForViewing)
DeclarationTest(shortenedType)
{
  return testObject(shortenTypeForViewing(decl->abstractType()), value, "Declaration's shortened type");
}
///JSON type: CtxtTestObject
///@returns whether the tests for the declaration's templateContext pass
DeclarationTest(templateContext)
{
  const QString NOT_A_TEMPLATE_DECL = "Requested template context, but declaration is not a template declaration.";
  TemplateDeclaration *templDecl = dynamic_cast<TemplateDeclaration*>(decl);
  if (!templDecl)
    return NOT_A_TEMPLATE_DECL;
  return testObject(templDecl->templateContext(decl->topContext()), value, "Declaration's template context");
}
///JSON type: string
///@returns whether the template parameter declaration's default argument matches the given value
DeclarationTest(templateParameterDefault)
{
  const QString NOT_A_TEMPLATE_PARAM_DECL = "Requested template parameters's default parameter, but declaration is not a template parameter declaration.";
  TemplateParameterDeclaration *templDecl = dynamic_cast<TemplateParameterDeclaration*>(decl);
  if (!templDecl)
    return NOT_A_TEMPLATE_PARAM_DECL;
  return compareValues(templDecl->defaultParameter().toString(), value, "Template parameter declaration's default parameter");
}

}

}

#endif //CPPJSONTESTS_H
