/*
    SPDX-FileCopyrightText: 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_IASTCONTAINER_H
#define KDEVPLATFORM_IASTCONTAINER_H

#include <language/languageexport.h>

#include <QExplicitlySharedDataPointer>

namespace KDevelop {
class COMPILE_ERROR_Ast_Top_Node_Was_Not_Defined;

/**
 * \brief Represents A single parse session that created a language-specific AST for a single translation unit
 *
 * This class is mainly just used as a tag to keep pointers to an AST of a translation unit(file)
 * in a KDevelop::TopDUContext, and passed over to Refactoring plugins so they can manipulate
 * their language-specific AstChangeSets.
 */
class KDEVPLATFORMLANGUAGE_EXPORT IAstContainer
    : public QSharedData
{
public:
    virtual ~IAstContainer();
    using Ptr = QExplicitlySharedDataPointer<IAstContainer>;

    /// Derived classes must redefine this typedef for CodeGenerator to work
    /// with this AST
    using TopAstNode = COMPILE_ERROR_Ast_Top_Node_Was_Not_Defined;
};
}

#endif // KDEVPLATFORM_IASTCONTAINER_H
