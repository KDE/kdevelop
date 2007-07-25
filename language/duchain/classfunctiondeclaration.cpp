/* This  is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "classfunctiondeclaration.h"

#include "ducontext.h"

namespace KDevelop
{

class ClassFunctionDeclarationPrivate
{
public:
  ClassFunctionDeclaration::QtFunctionType m_functionType;
  bool m_isVirtual: 1;
  bool m_isInline: 1;
  bool m_isExplicit: 1;
  QList<QString> m_defaultParameters;
};

ClassFunctionDeclaration::ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs) : ClassMemberDeclaration(rhs), AbstractFunctionDeclaration(), d(new ClassFunctionDeclarationPrivate) {
  d->m_functionType = rhs.d->m_functionType;
  d->m_isVirtual = rhs.d->m_isVirtual;
  d->m_isInline = rhs.d->m_isInline;
  d->m_isExplicit = rhs.d->m_isExplicit;
  d->m_defaultParameters = rhs.d->m_defaultParameters;
}

ClassFunctionDeclaration::ClassFunctionDeclaration(KTextEditor::Range * range, DUContext* context)
  : ClassMemberDeclaration(range, context)
  , d(new ClassFunctionDeclarationPrivate)
{
  d->m_functionType = Normal;
  d->m_isVirtual = false;
  d->m_isInline = false;
  d->m_isExplicit = false;
}

Declaration* ClassFunctionDeclaration::clone() const {
  return new ClassFunctionDeclaration(*this);
}

ClassFunctionDeclaration::~ClassFunctionDeclaration()
{
  delete d;
}

/*bool ClassFunctionDeclaration::isSimilar(KDevelop::CodeItem *other, bool strict ) const
{
  if (!CppClassMemberType::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<ClassFunctionDeclaration*>(other);

  if (isConstant() != func->isConstant())
    return false;

  if (arguments().count() != func->arguments().count())
    return false;

  for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

  return true;
}*/

ClassFunctionDeclaration::QtFunctionType ClassFunctionDeclaration::functionType() const
{
  return d->m_functionType;
}

const QList<QString>& ClassFunctionDeclaration::defaultParameters() const {
  return d->m_defaultParameters;
}

void ClassFunctionDeclaration::addDefaultParameter(const QString& str) {
  d->m_defaultParameters << str;
}

void ClassFunctionDeclaration::setFunctionType(QtFunctionType functionType)
{
  d->m_functionType = functionType;
}

bool ClassFunctionDeclaration::isConversionFunction() const {
  return identifier() == Identifier("operator{...cast...}");
}

bool ClassFunctionDeclaration::isConstructor() const
{
  if (context()->type() == DUContext::Class && context()->localScopeIdentifier().top() == identifier())
    return true;
  return false;
}

bool ClassFunctionDeclaration::isDestructor() const
{
  QString id = identifier().toString();
  return context()->type() == DUContext::Class && id.startsWith('~') && id.mid(1) == context()->localScopeIdentifier().top().toString();
}

bool ClassFunctionDeclaration::isVirtual() const
{
  return d->m_isVirtual;
}

void ClassFunctionDeclaration::setVirtual(bool isVirtual)
{
  d->m_isVirtual = isVirtual;
}

bool ClassFunctionDeclaration::isInline() const
{
  return d->m_isInline;
}

void ClassFunctionDeclaration::setInline(bool isInline)
{
  d->m_isInline = isInline;
}

bool ClassFunctionDeclaration::isExplicit() const
{
  return d->m_isExplicit;
}

void ClassFunctionDeclaration::setExplicit(bool isExplicit)
{
  d->m_isExplicit = isExplicit;
}

void ClassFunctionDeclaration::setFunctionSpecifiers(FunctionSpecifiers specifiers)
{
  d->m_isInline = specifiers & InlineSpecifier;
  d->m_isExplicit = specifiers & ExplicitSpecifier;
  d->m_isVirtual = specifiers & VirtualSpecifier;
}
}
// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
