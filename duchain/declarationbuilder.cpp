/*************************************************************************************
 *  Copyright (C) 2012 by Aleix Pol <aleixpol@kde.org>                               *
 *  Copyright (C) 2012 by Milian Wolff <mail@milianw.de>                             *
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

#include "declarationbuilder.h"

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/typeutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>

#include "expressionvisitor.h"
#include "parsesession.h"
#include "helper.h"

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
{
    m_session = session;
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url,
                                                 QmlJS::AST::Node* node,
                                                 ReferencedTopDUContext updateContext)
{
    ///TODO: cleanup
    Q_ASSERT(m_session->url() == url);
    return DeclarationBuilderBase::build(url, node, updateContext);
}

/*
 * Functions
 */
void DeclarationBuilder::visitFunction(QmlJS::AST::FunctionExpression* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    FunctionType::Ptr func(new FunctionType);
    QualifiedIdentifier name;
    RangeInRevision range;

    if (node->kind == QmlJS::AST::Node::Kind_FunctionDeclaration) {
        // Only function declarations have an identifier. Expressions are anonymous
        name = QualifiedIdentifier(node->name.toString());
        range = m_session->locationToRange(node->identifierToken);
    }

    {
        DUChainWriteLocker lock;
        FunctionDeclaration *decl = openDeclaration<FunctionDeclaration>(name, range);
        decl->setKind(Declaration::Type);
    }
    openType(func);
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    visitFunction(node);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionExpression* node)
{
    visitFunction(node);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    for (QmlJS::AST::FormalParameterList *plist = node; plist; plist = plist->next) {
        const QualifiedIdentifier name(plist->name.toString());
        const RangeInRevision range = m_session->locationToRange(plist->identifierToken);

        DUChainWriteLocker lock;
        Declaration* dec = openDeclaration<Declaration>(name, range);
        IntegralType::Ptr type(new IntegralType(IntegralType::TypeMixed));
        dec->setType(type);
        closeDeclaration();

        if (FunctionType::Ptr funType = currentType<FunctionType>()) {
            funType->addArgument(type.cast<AbstractType>());
        }
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::ReturnStatement* node)
{
    if (FunctionType::Ptr func = currentType<FunctionType>()) {
        AbstractType::Ptr returnType;

        if (node->expression) {
            returnType = findType(node->expression).type;
        } else {
            returnType = new IntegralType(IntegralType::TypeVoid);
        }

        DUChainWriteLocker lock;

        func->setReturnType(TypeUtils::mergeTypes(func->returnType(), returnType));
    }

    return false;   // findType has already explored node
}

void DeclarationBuilder::endVisitFunction(QmlJS::AST::FunctionExpression*)
{
    FunctionType::Ptr func = currentType<FunctionType>();

    if (func && !func->returnType()) {
        // A function that returns nothing returns void
        DUChainWriteLocker lock;

        func->setReturnType(AbstractType::Ptr(new IntegralType(IntegralType::TypeVoid)));
    }

    closeAndAssignType();
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionDeclaration* node)
{
    DeclarationBuilderBase::endVisit(node);

    endVisitFunction(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionExpression* node)
{
    DeclarationBuilderBase::endVisit(node);

    endVisitFunction(node);
}

/*
 * Variables
 */
bool DeclarationBuilder::visit(QmlJS::AST::VariableDeclaration* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    const QualifiedIdentifier name(node->name.toString());
    const RangeInRevision range = m_session->locationToRange(node->identifierToken);
    const AbstractType::Ptr type = findType(node->expression).type;

    {
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
    }
    openType(type);

    return false;   // findType has already explored node
}

void DeclarationBuilder::endVisit(QmlJS::AST::VariableDeclaration* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

bool DeclarationBuilder::visit(QmlJS::AST::BinaryExpression* node)
{
    if (node->op == QSOperator::Assign) {
        ContextBuilder::ExpressionType leftType = findType(node->left);
        AbstractType::Ptr rightType = findType(node->right).type;

        if (leftType.declaration) {
            // Merge the already-known type of the variable with the new one
            DUChainWriteLocker lock;

            leftType.declaration->setAbstractType(TypeUtils::mergeTypes(leftType.type, rightType));
        }

        return false;   // findType has already explored node
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::CallExpression* node)
{
    ContextBuilder::ExpressionType expr = findType(node->base);
    FunctionType::Ptr func_type = FunctionType::Ptr::dynamicCast(expr.type);
    DUChainWriteLocker lock;

    if (!expr.declaration || !func_type) {
        return DeclarationBuilderBase::visit(node);
    }

    auto func_declaration = expr.declaration.dynamicCast<FunctionDeclaration>();

    if (!func_declaration) {
        return DeclarationBuilderBase::visit(node);
    }

    // Put the argument nodes in a list that has a definite size
    QVector<Declaration *> arguments_decls = func_declaration->internalFunctionContext()->localDeclarations();
    QVector<QmlJS::AST::ArgumentList *> arguments;

    for (auto argument = node->arguments; argument; argument = argument->next) {
        arguments.append(argument);
    }

    // Don't update a function when it is called with the wrong number
    // of arguments
    if (arguments.size() != arguments_decls.size()) {
        return DeclarationBuilderBase::visit(node);
    }

    // Update the types of the function arguments
    FunctionType::Ptr new_func_type(new FunctionType);

    for (int i=0; i<arguments.size(); ++i) {
        QmlJS::AST::ArgumentList *argument = arguments.at(i);
        Declaration *current_declaration = arguments_decls.at(i);
        AbstractType::Ptr current_type = current_declaration->abstractType();

        // Merge the current type of the argument with its type in the call expression
        AbstractType::Ptr call_type = findType(argument->expression).type;
        AbstractType::Ptr new_type = TypeUtils::mergeTypes(current_type, call_type);

        // Update the declaration of the argument and its type in the function type
        current_declaration->setAbstractType(new_type);
        new_func_type->addArgument(new_type);
    }

    // Replace the function's type with the new type having updated arguments
    new_func_type->setReturnType(func_type->returnType());
    func_declaration->setAbstractType(new_func_type.cast<AbstractType>());

    return false;   // findType has already explored node
}

/*
 * Arrays
 */
bool DeclarationBuilder::visit(QmlJS::AST::PropertyNameAndValue* node)
{
    setComment(node);

    if (!node->name || !node->value) {
        return DeclarationBuilderBase::visit(node);
    }

    RangeInRevision range(m_session->locationToRange(node->name->propertyNameToken));
    QualifiedIdentifier name(QmlJS::getNodeValue(node->name));

    // The type of the declaration can either be an enumeration value or the type
    // of its expression
    Declaration* decl = currentDeclaration();
    AbstractType::Ptr type;

    if (decl && decl->abstractType() && decl->abstractType()->whichType() == AbstractType::TypeEnumeration) {
        // This is an enumeration value
        auto value = QmlJS::AST::cast<QmlJS::AST::NumericLiteral*>(node->value);
        EnumeratorType::Ptr enumerator(new EnumeratorType);

        enumerator->setDataType(IntegralType::TypeInt);

        if (value) {
            enumerator->setValue((int)value->value);
        }

        type = AbstractType::Ptr::staticCast(enumerator);
    } else {
        // Normal value
        type = findType(node->value).type;
    }

    // Open the declaration
    {
        DUChainWriteLocker lock;
        openDeclaration<ClassMemberDeclaration>(name, range);
    }
    openType(type);

    return false;   // findType has already explored node->expression
}

void DeclarationBuilder::endVisit(QmlJS::AST::PropertyNameAndValue* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

/*
 * plugin.qmltypes files
 */
void DeclarationBuilder::declareExports(QmlJS::AST::ExpressionStatement *exports,
                                        Declaration* classdecl)
{
    if (!exports || !exports->expression) {
        return;
    }

    auto exportslist = QmlJS::AST::cast<QmlJS::AST::ArrayLiteral*>(exports->expression);

    if (!exportslist) {
        return;
    }

    // Make an alias between each exported name of the component and the component itself
    for (auto it = exportslist->elements; it && it->expression; it = it->next) {
        auto stringliteral = QmlJS::AST::cast<QmlJS::AST::StringLiteral *>(it->expression);

        if (!stringliteral) {
            continue;
        }

        // String literal like "Namespace/Class version".
        QString exportname = stringliteral->value.toString().section(' ', 0, 0).section('/', -1, -1);

        {
            DUChainWriteLocker lock;
            AliasDeclaration* decl = openDeclaration<AliasDeclaration>(
                QualifiedIdentifier(exportname),
                m_session->locationToRange(stringliteral->literalToken)
            );

            decl->setKind(Declaration::Alias);
            decl->setAliasedDeclaration(IndexedDeclaration(classdecl));
        }
        closeDeclaration();
    }
}

void DeclarationBuilder::declareComponent(QmlJS::AST::UiObjectDefinition* node,
                                          const RangeInRevision &range,
                                          const QualifiedIdentifier &name)
{
    QString inherits = QmlJS::getQMLAttributeValue(node->initializer->members, "prototype").value.section('/', -1, -1);

    // Declare the component itself
    StructureType::Ptr type(new StructureType);
    type->setDeclarationId(DeclarationId(name));

    ClassDeclaration* decl;
    {
        DUChainWriteLocker lock;
        decl = openDeclaration<ClassDeclaration>(name, range, DeclarationIsDefinition);

        decl->setKind(Declaration::Type);
        decl->clearBaseClasses();

        if (!inherits.isNull()) {
            BaseClassInstance baseclass;

            baseclass.access = Declaration::Public;
            baseclass.virtualInheritance = false;
            baseclass.baseClass = typeFromClassName(inherits)->indexed();


            decl->addBaseClass(baseclass);
        }
    }
    openType(type);

    // The component may have exports (considered as type aliases)
    declareExports(
        QmlJS::AST::cast<QmlJS::AST::ExpressionStatement*>(QmlJS::getQMLAttribute(node->initializer->members, "exports")),
        decl
    );
}

void DeclarationBuilder::declareMethod(QmlJS::AST::UiObjectDefinition* node,
                                       const RangeInRevision &range,
                                       const QualifiedIdentifier &name,
                                       bool isSlot,
                                       bool isSignal)
{
    QString type_name = QmlJS::getQMLAttributeValue(node->initializer->members, "type").value;
    FunctionType::Ptr type(new FunctionType);

    if (type_name.isNull()) {
        type->setReturnType(typeFromName("void"));
    } else {
        type->setReturnType(typeFromName(type_name));
    }

    {
        DUChainWriteLocker lock;
        ClassFunctionDeclaration* decl = openDeclaration<ClassFunctionDeclaration>(name, range);

        decl->setIsSlot(isSlot);
        decl->setIsSignal(isSignal);
    }
    openType(type);
}

void DeclarationBuilder::declareProperty(QmlJS::AST::UiObjectDefinition* node,
                                         const RangeInRevision &range,
                                         const QualifiedIdentifier &name)
{
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->initializer->members, "type").value);

    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setAbstractType(type);
    }
    openType(type);
}

void DeclarationBuilder::declareParameter(QmlJS::AST::UiObjectDefinition* node,
                                          const RangeInRevision &range,
                                          const QualifiedIdentifier &name)
{
    FunctionType::Ptr function = currentType<FunctionType>();
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->initializer->members, "type").value);

    function->addArgument(type);

    {
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
    }
    openType(type);
}

void DeclarationBuilder::declareEnum(QmlJS::AST::UiObjectDefinition* node,
                                     const RangeInRevision &range,
                                     const QualifiedIdentifier &name)
{
    EnumerationType::Ptr type(new EnumerationType);

    type->setDeclarationId(DeclarationId(name));
    type->setDataType(IntegralType::TypeEnumeration);

    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setKind(Declaration::Type);
        decl->setType(type);                // The type needs to be set here because closeContext is called before closeAndAssignType and needs to know the type of decl
    }
    openType(type);
}

/*
 * UI
 */
void DeclarationBuilder::endVisit(QmlJS::AST::UiImport* node)
{
    QmlJS::AST::UiQualifiedId *part = node->importUri;
    QString uri;

    while (part) {
        uri.append(part->name.toString() + '.');
        part = part->next;
    }

    // Import the file corresponding to the URI
    ReferencedTopDUContext importedContext = m_session->contextOfModule(uri + "qml");

    if (importedContext) {
        DUChainWriteLocker lock;
        currentContext()->addImportedParentContext(
            importedContext,
            m_session->locationToRange(node->importToken).start
        );
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectDefinition* node)
{
    setComment(node);

    // Do not crash if the user has typed an empty object definition
    if (!node->initializer || !node->initializer->members) {
        return DeclarationBuilderBase::visit(node);
    }

    // Instance of special class names may declare classes, enums, methods, etc
    QString baseclass = node->qualifiedTypeNameId->name.toString();

    RangeInRevision range(m_session->locationToRange(node->qualifiedTypeNameId->identifierToken));
    QualifiedIdentifier name(
        QmlJS::getQMLAttributeValue(node->initializer->members, "name").value.section('/', -1, -1)
    );

    if (baseclass == QLatin1String("Component")) {
        // QML component, equivalent to a QML class
        declareComponent(node, range, name);
    } else if (baseclass == QLatin1String("Method") ||
               baseclass == QLatin1String("Signal") ||
               baseclass == QLatin1String("Slot")) {
        // Method (that can also be a signal or a slot)
        declareMethod(node, range, name, baseclass == QLatin1String("Slot"), baseclass == QLatin1String("Signal"));
    } else if (baseclass == QLatin1String("Property")) {
        // A property
        declareProperty(node, range, name);
    } else if (baseclass == QLatin1String("Parameter") && currentType<FunctionType>()) {
        // One parameter of a signal/slot/method
        declareParameter(node, range, name);
    } else if (baseclass == QLatin1String("Enum")) {
        // Enumeration. The "values" key contains a dictionary of name -> number entries.
        declareEnum(node, range, name);
    } else {
        // No special base class, so it is a normal instantiation
        QmlJS::QMLAttributeValue id_attribute = QmlJS::getQMLAttributeValue(node->initializer->members, "id");
        QualifiedIdentifier id(id_attribute.value);
        RangeInRevision range(m_session->locationToRange(id_attribute.location));

        StructureType::Ptr type(new StructureType);
        type->setDeclarationId(DeclarationId(QualifiedIdentifier(baseclass)));

        {
            DUChainWriteLocker lock;
            ClassDeclaration* decl = openDeclaration<ClassDeclaration>(id, range);
            decl->setKind(Declaration::Instance);
        }
        openType(type);
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiObjectDefinition* node)
{
    DeclarationBuilderBase::endVisit(node);

    // Do not crash if the user has typed an empty object definition
    if (node->initializer && node->initializer->members) {
        closeAndAssignType();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectInitializer* node)
{
    const bool ret = DeclarationBuilderBase::visit(node);
    DUChainWriteLocker lock;
    if (currentDeclaration<ClassDeclaration>()) {
        Q_ASSERT(currentContext());
        currentDeclaration()->setInternalContext(currentContext());
    }
    return ret;
}

bool DeclarationBuilder::visit(QmlJS::AST::UiScriptBinding* node)
{
    if (node->qualifiedId && node->qualifiedId->name != QLatin1String("id")) {
        setComment(node);

        const RangeInRevision& range = m_session->locationToRange(node->qualifiedId->identifierToken);
        const QualifiedIdentifier id(node->qualifiedId->name.toString());
        const AbstractType::Ptr type(findType(node->statement).type);

        {
            DUChainWriteLocker lock;
            openDeclaration<ClassMemberDeclaration>(id, range);
        }
        openType(type);

        return false;   // findType has already explored node->statement
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiScriptBinding* node)
{
    DeclarationBuilderBase::endVisit(node);

    if (node->qualifiedId && node->qualifiedId->name != QLatin1String("id")) {
        closeAndAssignType();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    setComment(node);

    const RangeInRevision& range = m_session->locationToRange(node->identifierToken);
    const QualifiedIdentifier id(node->name.toString());
    const AbstractType::Ptr type = findType(node->statement).type;

    {
        DUChainWriteLocker lock;
        openDeclaration<ClassMemberDeclaration>(id, range);
    }
    openType(type);

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiPublicMember* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

/*
 * Utils
 */
void DeclarationBuilder::closeContext()
{
    {
        DUChainWriteLocker lock;
        Declaration* decl = currentDeclaration();
        FunctionDeclaration* function = dynamic_cast<FunctionDeclaration*>(decl);
        DUContext* ctx = currentContext();

        if (function && ctx) {
            if (ctx->type() == DUContext::Function) {
                // This context contains the declarations of the arguments
                function->setInternalFunctionContext(ctx);
            } else {
                // This one contains the body of the function
                Q_ASSERT(ctx->type() == DUContext::Other);
                function->setInternalContext(ctx);
            }
        }

        if (ctx && decl && decl->abstractType() && decl->abstractType()->whichType() == AbstractType::TypeEnumeration)
        {
            // We are closing an enumeration. Its internal context must be of type "Enum"
            ctx->setType(DUContext::Enum);
            decl->setInternalContext(ctx);
        }
    }
    DeclarationBuilderBase::closeContext();
}

void DeclarationBuilder::setComment(QmlJS::AST::Node* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());
}

void DeclarationBuilder::closeAndAssignType()
{
    closeType();
    Declaration* dec = currentDeclaration();
    Q_ASSERT(dec);
    Q_ASSERT(lastType());
    {
        DUChainWriteLocker lock;
        dec->setType(lastType());
    }
    closeDeclaration();
}

AbstractType::Ptr DeclarationBuilder::typeFromName(const QString& name)
{
    auto type = IntegralType::TypeVoid;

    if (name == QLatin1String("string")) {
        type = IntegralType::TypeString;
    } else if (name == QLatin1String("bool")) {
        type = IntegralType::TypeBoolean;
    } else if (name == QLatin1String("int")) {
        type = IntegralType::TypeInt;
    } else if (name == QLatin1String("float")) {
        type = IntegralType::TypeFloat;
    } else if (name == QLatin1String("void")) {
        type = IntegralType::TypeVoid;
    }

    if (type == IntegralType::TypeVoid) {
        // Not a built-in type, but a class
        return typeFromClassName(name);
    } else {
        return AbstractType::Ptr(new IntegralType(type));
    }
}

AbstractType::Ptr DeclarationBuilder::typeFromClassName(const QString& name)
{
    DeclarationPointer decl = QmlJS::getDeclaration(QualifiedIdentifier(name), currentContext());

    if (decl && decl->kind() == Declaration::Type) {
        return decl->abstractType();
    } else {
        StructureType* rs = new StructureType;

        rs->setDeclarationId(DeclarationId(IndexedQualifiedIdentifier(QualifiedIdentifier(name))));

        return AbstractType::Ptr(rs);
    }
}
