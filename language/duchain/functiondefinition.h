/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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
#ifndef KDEVPLATFORM_CLASSFUNCTIONDEFINITION_H
#define KDEVPLATFORM_CLASSFUNCTIONDEFINITION_H

#include "functiondeclaration.h"
#include <language/languageexport.h>

using namespace KDevelop;

namespace KDevelop {
  class KDEVPLATFORMLANGUAGE_EXPORT FunctionDefinitionData : public FunctionDeclarationData {
  public:

    //Holds the declaration id for this definition, if this is a definition with separate declaration
    DeclarationId m_declaration;
  };
  ///A FunctionDefinition represents a function-definition that is assigned to a separate function-declaration.
  ///It allows mapping from definition to declaration and from declaration to definition.
  class KDEVPLATFORMLANGUAGE_EXPORT FunctionDefinition : public FunctionDeclaration {
    public:
    FunctionDefinition(const RangeInRevision& range, DUContext* context);
    FunctionDefinition(FunctionDefinitionData& data);
    ~FunctionDefinition();

    /**
    * Find the declaration for this definition, if one exists.
    *
    * @param topContext the top-context from which to search
    * \returns the declaration matching this definition, otherwise null if no matching declaration has been found.
    * */
    Declaration* declaration(const TopDUContext* topContext = 0) const;

    ///Returns true if a Declaration has been assigned to this Definition
    bool hasDeclaration() const;
    
    ///Attaches this definition to the given declaration persistently.
    void setDeclaration(Declaration* declaration);
    
    enum {
      Identity = 21
    };

    /**
    * Find the definition for the given declaration, if one exists.
    *
    * \returns the definition matching this declaration, otherwise null if no matching definition has been found.
    * */
    static FunctionDefinition* definition(const Declaration* decl);
    
  protected:
    FunctionDefinition (const FunctionDefinition& rhs);
  private:
    virtual Declaration* clonePrivate() const;
    DUCHAIN_DECLARE_DATA(FunctionDefinition)
  };
}

#endif
