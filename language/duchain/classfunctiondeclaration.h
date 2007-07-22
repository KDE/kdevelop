/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef CLASSFUNCTIONDECLARATION_H
#define CLASSFUNCTIONDECLARATION_H

#include <classmemberdeclaration.h>
#include <abstractfunctiondeclaration.h>
#include <languageexport.h>

namespace KDevelop
{
/**
 * Represents a single variable definition in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclaration : public ClassMemberDeclaration, public AbstractFunctionDeclaration
{
public:
  ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs);
  ClassFunctionDeclaration(KTextEditor::Range* range, DUContext* context);
  ~ClassFunctionDeclaration();

  enum QtFunctionType
  {
    Normal,
    Signal,
    Slot
  };

  QtFunctionType functionType() const;
  void setFunctionType(QtFunctionType functionType);

  bool isConstructor() const;
  bool isDestructor() const;
  bool isConversionFunction() const;

  enum FunctionSpecifier {
    VirtualSpecifier  = 0x1,
    InlineSpecifier   = 0x2,
    ExplicitSpecifier = 0x4
  };
  Q_DECLARE_FLAGS(FunctionSpecifiers, FunctionSpecifier)

  void setFunctionSpecifiers(FunctionSpecifiers specifiers);

  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  bool isInline() const;
  void setInline(bool isInline);

  bool isExplicit() const;
  void setExplicit(bool isExplicit);

  /**
   * Returns the default-parameters that are set. The last default-parameter matches the last
   * argument of the function, but the returned list will only contain default-values for those
   * arguments that have one, for performance-reasons.
   *
   * So the list may be empty or smaller than the list of function-arguments.
   * */
  virtual const QList<QString>& defaultParameters() const;

  virtual void addDefaultParameter(const QString& str);

  //bool isSimilar(KDevelop::CodeItem *other, bool strict = true) const;

  virtual Declaration* clone() const;
private:
  class ClassFunctionDeclarationPrivate* const d;
};
}
Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::ClassFunctionDeclaration::FunctionSpecifiers)

#endif // CLASSFUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
