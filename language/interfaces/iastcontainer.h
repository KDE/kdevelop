/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

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
   Boston, MA 02110-1301, USA.*/

#ifndef KDEVPLATFORM_IASTCONTAINER_H
#define KDEVPLATFORM_IASTCONTAINER_H

#include <ksharedptr.h>

#include "../languageexport.h"

namespace KDevelop
{
class COMPILE_ERROR_Ast_Top_Node_Was_Not_Defined;

/**
 * \brief Represents A single parse session that created a language-specific AST for a single translation unit
 *
 * This class is mainly just used as a tag to keep pointers to an AST of a translation unit(file)
 * in a KDevelop::TopDUContext, and passed over to Refactoring plugins so they can manipulate
 * their language-specific AstChangeSets.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IAstContainer : public QSharedData
{
  public:
    virtual ~IAstContainer();
    typedef QExplicitlySharedDataPointer<IAstContainer> Ptr;
    
    /// Derived classes must redifine this typedef for CodeGenerator to work
    /// with this AST
    typedef COMPILE_ERROR_Ast_Top_Node_Was_Not_Defined TopAstNode;
};

}


#endif // KDEVPLATFORM_IASTCONTAINER_H
