/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_DYNAMICLANGUAGEEXPRESSIONVISITOR_H
#define KDEVPLATFORM_DYNAMICLANGUAGEEXPRESSIONVISITOR_H

#include "language/duchain/ducontext.h"
#include "../../languageexport.h"

namespace KDevelop {

/**
 * @brief Provides functionality commonly needed for expression visitors in dynamically typed languages.
 *
 * In languages such as Python, Ruby, PHP and JS an expression visitor is commonly used
 * to find the type of an expression, such as "3", "3+5", "a.b" or "func(arg)".
 * Since that requires almost the same logic, it is also commonly used to find out
 * the declaration of which a type should be updated in -- for example -- an assignment.
 *
 * Your expression visitor should inherit your default AST visitor as well as this class.
 *
 * Consider this Python code as an example:
 * \code
 *   a = 3
 *   b = str(a)
 * \endcode
 * This results in an AST roughly like this:
 * \code
 *   Module [
 *     Assignment {
 *       lhs = Name("a"),
 *       rhs = IntegerLiteral
 *     },
 *     Assignment {
 *       lhs = Name("b"),
 *       rhs = Call {
 *         func = "str",
 *         args = Arguments[ Name("a") ]
 *       }
 *     }
 *   ]
 * \endcode
 * In your expression visitor, you would implement
 *   - visitIntegerLiteral to set the last type to int, e.g. by calling
 *     \code
 *       encounter(AbstractType::Ptr(new IntegralType(IntegralType::TypeInt)))
 *     \endcode
 *     from there
 *   - visitCall to set the last type to the function's return type,
 *     by finding the function which is called in the duchain and calling
 *     encounter() on its return type as above.
 *
 * For parsing either assignment, you could then
 *   - in the declaration builder, create an ExpressionVisitor
 *   - call that visitor's visit() method on the assignment's rhs
 *     (this method is inherited from your language's default visitor)
 *   - set or update the type of the Name on the left side to
 *     the visitor's lastType().
 *
 * A example for a more complex case would be this:
 * \code
 *   a = list()
 *   a[0] = 4
 * \endcode
 * Here, when dealing with the second assignment, in the declaration builder
 * you want to know the declaration of which you have to update the content type.
 * Thus, in your expression visitor, when you visit an index access, you can provide
 * a declaration to encounter() which can later be retrieved using lastDeclaration().
 * This makes it relatively straightforward to handle arbitrarily complex situations such as
 * \code
 *   a.b[0].c[3] = 42
 * \endcode
 * with little to no extra effort.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DynamicLanguageExpressionVisitor {
public:
    /**
     * @brief Construct a new expression visitor in the given @p context.
     *
     * @param context The DUContext the expression visitor resolves names in.
     */
    DynamicLanguageExpressionVisitor(const DUContext* context);

    /**
     * @brief Construct a new expression visitor and copy all fixed properties from @p parent.
     */
    DynamicLanguageExpressionVisitor(DynamicLanguageExpressionVisitor* parent);

    virtual ~DynamicLanguageExpressionVisitor() { };

    /**
     * @brief Return the DUContext this visitor is working on.
     */
    inline const DUContext* context() const {
        return m_context;
    }

    inline const TopDUContext* topContext() const {
        return m_context->topContext();
    }

    /**
     * @brief Retrieve this visitor's last encountered type.
     * This is never a null type.
     */
    inline AbstractType::Ptr lastType() const {
        if ( ! m_lastType ) {
            return unknownType();
        }
        return m_lastType;
    }

    /**
     * @brief Retrieve this visitor's last encountered declaration. May be null.
     */
    inline DeclarationPointer lastDeclaration() const {
        return m_lastDeclaration;
    }

    /**
     * @brief Check whether this visitor thinks its computed result is reliable or not.
     * This can be used to give hints to other builders about whether a problem should be
     * reported to the user or not in some situations. For example, if a member of a variable
     * is accessed which does not seem to exist, you can create a problem if the expression
     * visitor which determined the type of the variable is confident of what it found, and
     * otherwise do nothing.
     */
    inline bool isConfident() const {
        return m_isConfident;
    }

    /**
     * @brief Encounter @p lvalueDeclaration as an lvalue.
     * Calling this function sets the last declaration to @p lvalueDeclaration and the
     * last type to @p lvalueDeclaration 's type.
     * it is intended to be used for e.g. attribute access:
     * \code
     *     a.b = 3
     * \endcode
     * When visiting the AST of the "a.b" expression,
     * the expression visitor should call encounterLvalue on
     * the declaration of the "b" property of "a".
     *
     * Calling this function with decl as @p lvalueDeclaration is the same
     * as calling encounter(decl->abstractType(), decl).
     *
     * @param lvalueDeclaration The declaration to set as last declaration and derive the last type from
     */
    void encounterLvalue(DeclarationPointer lvalueDeclaration);

    /**
     * @brief Encounter the given type and declaration.
     * If @p declaration is null, the visitor's last declaration is cleared.
     *
     * @param type Type to set as the last type
     * @param declaration Declaration to set as the last declaration; null by default
     */
    void encounter(AbstractType::Ptr type, DeclarationPointer declaration=DeclarationPointer());

    /**
     * @brief Set the last type to unknownType() and clear the last declaration.
     */
    void encounterUnknown();

    /**
     * @brief Should return the type to use when no type is known.
     * The default implementation returns IntegralType::TypeMixed.
     * @warning You must not return a null type from this function.
     */
    virtual AbstractType::Ptr unknownType() const;

protected:
    /**
     * @see isConfident
     */
    inline void setConfident(bool confident) {
        m_isConfident = confident;
    }

    /**
     * @brief Reimplement this if you need a hook before a type is encountered.
     * Default implementation does nothing.
     * @param type the type which would be encountered
     * @return the type which will be encountered instead
     */
    virtual AbstractType::Ptr encounterPreprocess(AbstractType::Ptr type);

protected:
    const DUContext* m_context;
    AbstractType::Ptr m_lastType;
    DeclarationPointer m_lastDeclaration;
    bool m_isConfident;
    DynamicLanguageExpressionVisitor* m_parentVisitor;
};

} // namespace KDevelop

#endif // KDEVPLATFORM_DYNAMICLANGUAGEEXPRESSIONVISITOR_H

// kate: space-indent on; indent-width 4;
