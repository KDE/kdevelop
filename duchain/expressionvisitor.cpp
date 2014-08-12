/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "expressionvisitor.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/structuretype.h>
#include <util/path.h>

#include "helper.h"
#include "functiontype.h"
#include "parsesession.h"
#include "frameworks/nodejs.h"

using namespace KDevelop;

ExpressionVisitor::ExpressionVisitor(DUContext* context)
: DynamicLanguageExpressionVisitor(context),
  m_prototypeDepth(0)
{
}

void ExpressionVisitor::postVisit(QmlJS::AST::Node* node)
{
    // Each time a node is closed, decrement the prototype depth. This way,
    // if a "prototype" node has been encountered, ExpressionVisitor can know
    // whether it appeared at the top of the tree ("foo.bar.prototype") or
    // somewhere else ("foo.prototype.bar").
    --m_prototypeDepth;

    QmlJS::AST::Visitor::postVisit(node);
}

bool ExpressionVisitor::isPrototype() const
{
    return m_prototypeDepth == 1;
}

/*
 * Literals
 */
bool ExpressionVisitor::visit(QmlJS::AST::NumericLiteral* node)
{
    int num_int_digits = (int)std::log10(node->value) + 1;

    encounter(
        num_int_digits == (int)node->literalToken.length ?
            IntegralType::TypeInt :
            IntegralType::TypeDouble
    );
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::StringLiteral*)
{
    encounter(IntegralType::TypeString);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::RegExpLiteral*)
{
    encounter(QLatin1String("RegExp"));

    if (lastDeclaration()) {
        instantiateCurrentDeclaration();
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::TrueLiteral*)
{
    encounter(IntegralType::TypeBoolean);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::FalseLiteral*)
{
    encounter(IntegralType::TypeBoolean);
    return false;
}

/*
 * Object and arrays
 */
bool ExpressionVisitor::visit(QmlJS::AST::ArrayLiteral*)
{
    encounter(AbstractType::Ptr(new IntegralType(IntegralType::TypeArray)));
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::ObjectLiteral* node)
{
    encounterObjectAtLocation(node->lbraceToken);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::ArrayMemberExpression* node)
{
    // array["string_literal"] is equivalent to array.string_literal
    auto literal = QmlJS::AST::cast<QmlJS::AST::StringLiteral*>(node->expression);

    if (literal) {
        node->base->accept(this);
        encounterFieldMember(literal->value.toString());
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::FieldMemberExpression* node)
{
    // Find the type of the base, and if this type has a declaration, use
    // its inner context to get the type of the field member
    node->base->accept(this);
    encounterFieldMember(node->name.toString());

    return false;
}

/*
 * Identifiers and common expressions
 */
bool ExpressionVisitor::visit(QmlJS::AST::BinaryExpression* node)
{
    switch (node->op) {
    case QSOperator::BitAnd:
    case QSOperator::BitOr:
    case QSOperator::BitXor:
    case QSOperator::LShift:
    case QSOperator::RShift:
    case QSOperator::URShift:
        encounter(IntegralType::TypeInt);
        break;
    case QSOperator::And:
    case QSOperator::Equal:
    case QSOperator::Ge:
    case QSOperator::Gt:
    case QSOperator::In:
    case QSOperator::InstanceOf:
    case QSOperator::Le:
    case QSOperator::Lt:
    case QSOperator::Or:
    case QSOperator::StrictEqual:
    case QSOperator::StrictNotEqual:
        encounter(IntegralType::TypeBoolean);
        break;
    case QSOperator::Assign:
        node->right->accept(this);
        break;
    default:
        encounterNothing();
        break;
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::IdentifierExpression* node)
{
    encounter(node->name.toString());
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::UiQualifiedId* node)
{
    // "anchors.parent" results in an UiQualifiedId id having a "next" attribute.
    // This node reprensents "anchors", the next one is for "parent"
    encounter(node->name.toString());

    for (node = node->next; node && lastDeclaration(); node = node->next) {
        encounterFieldMember(node->name.toString());
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::ThisExpression* node)
{
    Q_UNUSED(node)
    DUChainReadLocker lock;
    DUContext* paramsContext;
    DUContext* internalContext;
    Declaration* owner;

    // "this" points to the current function (not semantically valid in JS,
    // but this allows ExpressionVisitor to see the declarations of the
    // function's prototype)
    if (m_context->type() == DUContext::Other &&                // Code of the function
        (paramsContext = m_context->parentContext()) &&         // Parameters of the function (this context has the function as owner)
        (owner = QmlJS::getOwnerOfContext(paramsContext)) &&    // The function itself (owner of its parameters)
        (internalContext = QmlJS::getInternalContext(DeclarationPointer(owner))) && // The prototype context of the function
        (owner = QmlJS::getOwnerOfContext(internalContext)) &&  // The function that declared the prototype context (paramsContext may belong to a method of a class)
        owner->abstractType()) {
        encounterLvalue(DeclarationPointer(owner));
        instantiateCurrentDeclaration();
    } else {
        encounterNothing();
    }

    return false;
}

/*
 * Functions
 */
bool ExpressionVisitor::visit(QmlJS::AST::FunctionExpression* node)
{
    encounterObjectAtLocation(node->lparenToken);
    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::CallExpression* node)
{
    // Special-case functions that have a specific meaning in some frameworks
    auto functionIdentifier = QmlJS::AST::cast<QmlJS::AST::IdentifierExpression*>(node->base);

    if (functionIdentifier &&
        node->arguments &&          // One argument
        !node->arguments->next &&   // But not two
        functionIdentifier->name.toString() == QLatin1String("require")) {
        auto moduleName = QmlJS::AST::cast<QmlJS::AST::StringLiteral*>(node->arguments->expression);

        if (moduleName) {
            encounterLvalue(QmlJS::NodeJS::instance().moduleExports(
                moduleName->value.toString(),
                m_context->topContext()->url()
            ));
        } else {
            encounterNothing();
        }

        return false;
    }

    // Find the type of the function called
    node->base->accept(this);

    QmlJS::FunctionType::Ptr func = QmlJS::FunctionType::Ptr::dynamicCast(m_lastType);

    if (func && func->returnType()) {
        encounter(func->returnType());
    } else {
        encounterNothing();
    }

    return false;
}

bool ExpressionVisitor::visit(QmlJS::AST::NewMemberExpression* node)
{
    // Find the type of the function used as constructor, and build a
    // StructureType representing an instance of this function.
    node->base->accept(this);

    if (lastDeclaration()) {
        instantiateCurrentDeclaration();
    } else {
        encounterNothing();
    }

    return false;
}

void ExpressionVisitor::encounterNothing()
{
    encounter(AbstractType::Ptr(), DeclarationPointer());
}

void ExpressionVisitor::encounter(IntegralType::CommonIntegralTypes type)
{
    encounter(AbstractType::Ptr(new IntegralType(type)));
}

void ExpressionVisitor::encounter(const QString& declaration, KDevelop::DUContext* context)
{
    QualifiedIdentifier name(declaration);
    DUChainReadLocker lock;

    if (!encounterParent(declaration) &&
        !encounterDeclarationInContext(name, context) &&
        !(!QmlJS::isQmlFile(m_context) && encounterDeclarationInNodeModule(name, QLatin1String("__builtin_dom"))) &&
        !encounterDeclarationInNodeModule(name, QLatin1String("__builtin_ecmascript")) &&
        !(context == nullptr && encounterGlobalDeclaration(name))) {
        encounterNothing();
    }
}

bool ExpressionVisitor::encounterParent(const QString& declaration)
{
    if (declaration != QLatin1String("parent") ||
        !QmlJS::isQmlFile(m_context)) {
        return false;
    }

    // Go up until we find a class context (the enclosing QML component)
    const DUContext* parent = m_context;
    Declaration* owner;

    while (parent && parent->type() != DUContext::Class) {
        parent = parent->parentContext();
    }

    // Take the parent context of the current QML component, it is its parent
    // component
    if (parent) {
        parent = parent->parentContext();
    }

    // Parent now points to the parent QML component. This is not always what
    // the user wants when typing "parent", but already works well for
    // "anchors.centerIn: parent" and things like that.
    if (parent &&
        (owner = QmlJS::getOwnerOfContext(parent)) &&
        owner->abstractType()) {
        encounterLvalue(DeclarationPointer(owner));
        return true;
    }

    return false;
}

bool ExpressionVisitor::encounterDeclarationInContext(const QualifiedIdentifier& id, DUContext* context)
{
    DeclarationPointer dec = QmlJS::getDeclarationOrSignal(id,
                                                           context ? context : m_context,
                                                           context == nullptr);

    if (dec) {
        encounterLvalue(dec);
        return true;
    }

    return false;
}

bool ExpressionVisitor::encounterDeclarationInNodeModule(const QualifiedIdentifier& id,
                                                         const QString& module)
{
    return encounterDeclarationInContext(
        id,
        QmlJS::getInternalContext(
            QmlJS::NodeJS::instance().moduleExports(module, m_context->url())
        )
    );
}

bool ExpressionVisitor::encounterGlobalDeclaration(const QualifiedIdentifier& id)
{
    // Use the persistent symbol table to find this declaration, even if it is in another file
    uint count = 0;
    const IndexedDeclaration* declarations = nullptr;

    PersistentSymbolTable::self().declarations(IndexedQualifiedIdentifier(id), count, declarations);

    // Explore the declarations and filter-out those that come from a file
    // outside the current directory
    Path currentDir = Path(m_context->topContext()->url().str()).parent();

    for (uint i=0; i<count; ++i) {
        const IndexedDeclaration& decl = declarations[i];
        IndexedTopDUContext declTopContext = decl.indexedTopContext();

        if (!declTopContext.isValid()) {
            continue;
        }

        if (currentDir.isDirectParentOf(Path(declTopContext.url().str()))) {
            encounterLvalue(DeclarationPointer(decl.declaration()));
            return true;
        }
    }

    return false;
}

void ExpressionVisitor::encounterFieldMember(const QString& name)
{
    if (QmlJS::isPrototypeIdentifier(name)) {
        // "prototype" is transparent: "object.prototype.foo" = "object.foo", and
        // "function.prototype" should point to "function".
        m_prototypeDepth = 2;
        return;
    }

    DeclarationPointer declaration = lastDeclaration();
    DUContext* context = QmlJS::getInternalContext(declaration);

    if (context) {
        encounter(name, context);
    } else {
        encounterNothing();
    }
}

void ExpressionVisitor::encounterObjectAtLocation(const QmlJS::AST::SourceLocation& location)
{
    DUChainReadLocker lock;

    // Find the anonymous declaration corresponding to the function. This is
    // the owner of the current context (function expressions create new contexts)
    Declaration* dec = QmlJS::getOwnerOfContext(m_context->topContext()->findContextAt(
        CursorInRevision(location.startLine-1, location.startColumn)
    ));

    if (dec && dec->abstractType()) {
        encounterLvalue(DeclarationPointer(dec));
    } else {
        encounterNothing();
    }
}

void ExpressionVisitor::instantiateCurrentDeclaration()
{
    StructureType::Ptr type = StructureType::Ptr(new StructureType);
    DeclarationPointer decl = lastDeclaration();

    {
        DUChainReadLocker lock;
        auto funcType = QmlJS::FunctionType::Ptr::dynamicCast(decl->abstractType());

        if (funcType) {
            decl = funcType->declaration(topContext());
        }

        type->setDeclaration(decl.data());
    }

    encounter(AbstractType::Ptr::staticCast(type), decl);
}
