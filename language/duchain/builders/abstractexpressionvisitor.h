/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2009 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVPLATFORM_ABSTRACTEXPRESSIONVISITOR_H
#define KDEVPLATFORM_ABSTRACTEXPRESSIONVISITOR_H

#include "abstractcontextbuilder.h"
#include "../types/abstracttype.h"
#include "../declaration.h"
#include "../ducontext.h"

namespace KDevelop
{

template<typename T, typename NameT, typename TokenType, typename LanguageSpecificExpressionVisitorBase>
class AbstractExpressionVisitor : public LanguageSpecificExpressionVisitorBase
{
  public:
    AbstractExpressionVisitor()
    {
    }

    struct Instance {
      Instance() : isInstance(false) {
      }
      Instance( bool is ) : isInstance(is) {
      }
      Instance( DeclarationPointer decl ) : isInstance(true), declaration(decl) {
      }
      Instance( Declaration* decl ) : isInstance(true), declaration(DeclarationPointer(decl)) {
      }
      inline operator bool() const {
        return isInstance;
      }

      bool isInstance;
      DeclarationPointer declaration; //May contain the declaration of the instance, but only when isInstance is true. May also contain type-declaration, which signalizes that this is an instance of that type.
    };

    void parse( T* ast )
    {
      m_lastType = 0;
      m_lastInstance = Instance();
      DUContext* context = LanguageSpecificExpressionVisitorBase::contextFromNode(ast);
      Q_ASSERT(context);
      m_topContext = context->topContext();
      LanguageSpecificExpressionVisitorBase::startVisiting(ast);
      m_topContext = 0;
      flushUse();
    }

    const KDevelop::AbstractType::Ptr lastType() const
    {
      return m_lastType;
    }

    void setLastType(KDevelop::AbstractType::Ptr type)
    {
      m_lastType = type;
    }

    void setInstantiatedType(bool instantiated = true)
    {
      m_lastInstance = Instance(instantiated);
    }

    void setInstantiatedType(KDevelop::AbstractType::Ptr type, bool instantiated = true)
    {
      m_lastType = type;
      m_lastInstance = Instance(instantiated);
    }

    template<class Type>
    void setLastType(TypePtr<Type> type)
    {
      m_lastType = AbstractType::Ptr::staticCast(type);
    }

    const Instance lastInstance() const
    {
      return m_lastInstance;
    }

    void setLastInstance(Declaration* decl)
    {
      m_lastInstance = Instance(decl);
      if (decl)
        m_lastType = decl->abstractType();
      else
        m_lastType = 0;
    }

  protected:
    KDevelop::AbstractType::Ptr lastType()
    {
      return m_lastType;
    }

    Instance lastInstance()
    {
      return m_lastInstance;
    }

    /**
     * Will be called for each relevant sub-node with the resolved type of that expression. This is not guaranteed to be called.
     * There is also no guarantee in which order expressionType() will be called.
     * The du-chain will not be locked in the moment this is called.
     *
     * @param node the AST-Node
     * @param type the type the expression in the AST-node evaluates to
     * @param instance If the expression evaluates to an instance of a type, this contains information about that instance. declaration is only filled for explicitly declared instances.
     * If this is zero, the expression evaluates to a type.
     *
     * Warning:
     * In case of temporary instances, decl will be the declaration of the basic type, not of an instance.
     * Since temporary instances are never declared, there's no other way.
     *
     * examples:
     * the expression "AbstractType::Ptr" evaluates to a type, so @param type would be filled and @param decl would be zero.
     * When the context contains "AbstractType::Ptr ptr;", the expression "ptr" will evaluate to an instance of
     * AbstractType::Ptr, so @param type will contain the type AbstractType::Ptr, and @param decl will point to the declaration of ptr.
     *
     * Problem:
     **/
    virtual void expressionType( T* node, const AbstractType::Ptr& type, Instance instance ) {
      Q_UNUSED(node) Q_UNUSED(type) Q_UNUSED(instance)
    }

    /** The duchain is not locked when this is called */
    virtual void usingDeclaration( T* node, const KDevelop::DeclarationPointer& decl, TokenType start_token = TokenType(), TokenType end_token = TokenType() ) {
      Q_UNUSED(node) Q_UNUSED(start_token) Q_UNUSED(end_token) Q_UNUSED(decl)
    }

    /** Called when there is a problem, with a string for that problem.
     * The default-implementation dumps all relevant information to
     * kDebug.
     * @param node the node the problem is about
     * @param str a string that describes the problem
     */
    virtual void problem( T* node, const QString& str )
    {
#ifdef DUMP_PROBLEMS
      kDebug() << "Problem:" << str;
#endif
    }

    void flushUse() {
      if( m_currentUse.isValid )
        usingDeclaration( m_currentUse.node, m_currentUse.start_token, m_currentUse.end_token, m_currentUse.declaration );
      m_currentUse.isValid = false;
    }

    struct CurrentUse {
      CurrentUse() : isValid(false), start_token(0), end_token(0) {
      }
      bool isValid;
      T* node;
      TokenType start_token, end_token;
      KDevelop::DeclarationPointer declaration;
    } m_currentUse; //This is used to temporarily delay the calling of usingDeclaration.

    KDevelop::TopDUContext* topContext() const
    {
      return m_topContext;
    }

  private:
    KDevelop::AbstractType::Ptr m_lastType;
    Instance m_lastInstance; //Contains whether the last evaluation resulted in an instance, and maybe the instance-declaration

    const KDevelop::TopDUContext* m_source;

    //Whenever a list of declarations is queried, it is stored here. Especially in visitName(...) and findMember(...)
    QList<KDevelop::DeclarationPointer> m_lastDeclarations;

    KDevelop::DUContext* m_currentContext;
    KDevelop::TopDUContext* m_topContext;
};

}

#endif // KDEVPLATFORM_ABSTRACTEXPRESSIONVISITOR_H
