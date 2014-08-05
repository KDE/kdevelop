/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_FUNCTIONDECLARATION_H
#define KDEVPLATFORM_FUNCTIONDECLARATION_H

#include "declaration.h"
#include "abstractfunctiondeclaration.h"
#include "declarationdata.h"

namespace KDevelop
{
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(FunctionDeclarationData, m_defaultParameters, IndexedString)

class KDEVPLATFORMLANGUAGE_EXPORT FunctionDeclarationData : public DeclarationData, public AbstractFunctionDeclarationData
{
  public:
    FunctionDeclarationData()
    {
      initializeAppendedLists();
    }
    FunctionDeclarationData( const FunctionDeclarationData& rhs )
      :DeclarationData( rhs ), AbstractFunctionDeclarationData(rhs)
    {
      initializeAppendedLists();
      copyListsFrom(rhs);
    }
    ~FunctionDeclarationData()
    {
      freeAppendedLists();
    }

    START_APPENDED_LISTS_BASE(FunctionDeclarationData, DeclarationData);
    APPENDED_LIST_FIRST(FunctionDeclarationData, IndexedString, m_defaultParameters);
    END_APPENDED_LISTS(FunctionDeclarationData, m_defaultParameters);
};
/**
 * Represents a single variable definition in a definition-use chain.
 */
typedef MergeAbstractFunctionDeclaration<Declaration, FunctionDeclarationData> FunctionDeclarationBase;
class KDEVPLATFORMLANGUAGE_EXPORT FunctionDeclaration : public FunctionDeclarationBase
{
public:
  FunctionDeclaration(const FunctionDeclaration& rhs);
  FunctionDeclaration(const RangeInRevision& range, DUContext* context);
  FunctionDeclaration(FunctionDeclarationData& data);
  FunctionDeclaration(FunctionDeclarationData& data, const KDevelop::RangeInRevision&);
  virtual ~FunctionDeclaration();

  virtual void setAbstractType(AbstractType::Ptr type);

  virtual QString toString() const;

  virtual bool isFunctionDeclaration() const;
  
  virtual uint additionalIdentity() const;
  
  virtual const IndexedString* defaultParameters() const;
  virtual unsigned int defaultParametersSize() const;
  virtual void addDefaultParameter(const IndexedString& str);
  virtual void clearDefaultParameters();
  
  enum {
    Identity = 12
  };
  
  typedef Declaration Base;

private:
  virtual Declaration* clonePrivate() const;
  DUCHAIN_DECLARE_DATA(FunctionDeclaration)
};
}

#endif // KDEVPLATFORM_FUNCTIONDECLARATION_H
