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
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/typeutils.h>
#include <language/duchain/declaration.h>
#include <language/duchain/aliasdeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>

#include "expressionvisitor.h"
#include "parsesession.h"
#include "functiondeclaration.h"
#include "helper.h"

#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
: m_prebuilding(false)
{
    m_session = session;
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url,
                                                 QmlJS::AST::Node* node,
                                                 ReferencedTopDUContext updateContext)
{
    Q_ASSERT(m_session->url() == url);

    // The declaration builder needs to run twice, so it can resolve uses of e.g. functions
    // which are called before they are defined (which is easily possible, due to JS's dynamic nature).
    if (!m_prebuilding) {
        kDebug() << "building, but running pre-builder first";
        DeclarationBuilder* prebuilder = new DeclarationBuilder(m_session);

        prebuilder->m_prebuilding = true;
        updateContext = prebuilder->build(url, node, updateContext);

        kDebug() << "pre-builder finished";
        delete prebuilder;
    } else {
        kDebug() << "prebuilding";
    }

    return DeclarationBuilderBase::build(url, node, updateContext);
}

void DeclarationBuilder::startVisiting(QmlJS::AST::Node* node)
{
    QFileInfo file(m_session->url().str());

    if (file.exists() && !file.absolutePath().contains(QLatin1String("kdevqmljssupport"))) {
        // Import all the files of the current directory in this context
        importDirectory(file.absolutePath(), nullptr);
    }

    // Remove all the imported parent contexts: imports may have been edited
    // and there musn't be any leftover parent context
    {
        DUChainWriteLocker lock;
        currentContext()->topContext()->clearImportedParentContexts();
    }

    DeclarationBuilderBase::startVisiting(node);
}

/*
 * Functions
 */
template<typename Decl>
void DeclarationBuilder::declareFunction(QmlJS::AST::Node* node,
                                         bool newPrototypeContext,
                                         const QualifiedIdentifier& name,
                                         const RangeInRevision& nameRange,
                                         QmlJS::AST::Node* parameters,
                                         const RangeInRevision& parametersRange,
                                         QmlJS::AST::Node* body,
                                         const RangeInRevision& bodyRange)
{
    setComment(node);

    // Declare the function
    FunctionType::Ptr func(new FunctionType);
    Decl* decl;

    {
        DUChainWriteLocker lock;

        decl = openDeclaration<Decl>(name, nameRange);
        decl->setKind(Declaration::Type);
    }
    openType(func);

    // Open the prototype context, if any. This has to be done before everything
    // else because this context is needed for "this" to be properly resolved
    // in the function body
    if (newPrototypeContext) {
        DUChainWriteLocker lock;
        QmlJS::FunctionDeclaration* d = reinterpret_cast<QmlJS::FunctionDeclaration*>(decl);

        d->setPrototypeContext(openContext(
            node + 2,       // Don't call setContextOnNode on node, only the body context can be associated with node
            nameRange,
            DUContext::Class,
            QualifiedIdentifier(name)
        ), true);
        closeContext();
    }

    // Parameters, if any (a function must always have an interal function context,
    // so always open a context here even if there are no parameters)
    DUContext* parametersContext = openContext(
        node + 1,       // Don't call setContextOnNode on node, only the body context can be associated with node
        parametersRange,
        DUContext::Function,
        name
    );

    if (parameters) {
        QmlJS::AST::Node::accept(parameters, this);
    }
    closeContext();

    // Body, if any
    DUContext* bodyContext = nullptr;

    if (body) {
        bodyContext = openContext(
            node,
            bodyRange,
            DUContext::Other,
            name
        );

        if (compilingContexts()) {
            DUChainWriteLocker lock;
            bodyContext->addImportedParentContext(parametersContext);
        }

        QmlJS::AST::Node::accept(body, this);
        closeContext();
    }

    // Set the inner contexts of the function
    {
        DUChainWriteLocker lock;

        decl->setInternalFunctionContext(parametersContext);

        if (bodyContext) {
            decl->setInternalContext(bodyContext);
        }
    }
}

template<typename Node>
void DeclarationBuilder::declareParameters(Node* node, QStringRef Node::*typeAttribute)
{
    for (Node *plist = node; plist; plist = plist->next) {
        const QualifiedIdentifier name(plist->name.toString());
        const RangeInRevision range = m_session->locationToRange(plist->identifierToken);

        AbstractType::Ptr type = (typeAttribute ?
            typeFromName((plist->*typeAttribute).toString()) :              // The typeAttribute attribute of plist contains the type name of the argument
            AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed))    // No type information, use mixed
        );

        {
            DUChainWriteLocker lock;
            openDeclaration<Declaration>(name, range);
        }
        openType(type);
        closeAndAssignType();

        if (FunctionType::Ptr funType = currentType<FunctionType>()) {
            funType->addArgument(type);
        }
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    declareFunction<QmlJS::FunctionDeclaration>(
        node,
        true,   // A function declaration always has its own prototype context
        QualifiedIdentifier(node->name.toString()),
        m_session->locationToRange(node->identifierToken),
        node->formals,
        m_session->locationsToInnerRange(node->lparenToken, node->rparenToken),
        node->body,
        m_session->locationsToInnerRange(node->lbraceToken, node->rbraceToken)
    );

    return false;
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionExpression* node)
{
    declareFunction<QmlJS::FunctionDeclaration>(
        node,
        false,
        QualifiedIdentifier(),
        QmlJS::emptyRangeOnLine(node->functionToken),
        node->formals,
        m_session->locationsToInnerRange(node->lparenToken, node->rparenToken),
        node->body,
        m_session->locationsToInnerRange(node->lbraceToken, node->rbraceToken)
    );

    return false;
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    declareParameters(node, (QStringRef QmlJS::AST::FormalParameterList::*)nullptr);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::UiParameterList* node)
{
    declareParameters(node, &QmlJS::AST::UiParameterList::type);

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

void DeclarationBuilder::endVisitFunction()
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

    endVisitFunction();
}

void DeclarationBuilder::endVisit(QmlJS::AST::FunctionExpression* node)
{
    DeclarationBuilderBase::endVisit(node);

    endVisitFunction();
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
        ExpressionType leftType = findType(node->left);
        ExpressionType rightType = findType(node->right);
        DUChainWriteLocker lock;

        if (leftType.declaration) {
            DUContext* leftCtx = leftType.declaration->context();

            // object.prototype.method = function(){} : when assigning a function
            // to a variable living in a Class context, set the prototype
            // context of the function to the context of the variable
            if (rightType.declaration && leftCtx->type() == DUContext::Class) {
                auto func = rightType.declaration.dynamicCast<QmlJS::FunctionDeclaration>();

                if (!leftCtx->owner() && !leftCtx->importers().isEmpty()) {
                    // MyClass.prototype.myfunc declares "myfunc" in a small context
                    // that is imported by MyClass. The prototype of myfunc should
                    // be the context of MyClass, not the small context in which
                    // it has been declared
                    leftCtx = leftCtx->importers().at(0);
                }

                if (func && !func->prototypeContext()) {
                    func->setPrototypeContext(leftCtx, false);
                }
            }

            // Merge the already-known type of the variable with the new one
            leftType.declaration->setAbstractType(TypeUtils::mergeTypes(leftType.type, rightType.type));
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
void DeclarationBuilder::declareFieldMember(const KDevelop::DeclarationPointer& declaration,
                                            const QString& member,
                                            QmlJS::AST::Node* node,
                                            const QmlJS::AST::SourceLocation& location)
{
    if (member == QLatin1String("prototype")) {
        // Don't declare "prototype", this is a special member
        return;
    }

    DUChainWriteLocker lock;
    QualifiedIdentifier identifier(member);

    // Declaration must have an internal context so that the member can be added
    // into it.
    DUContext* ctx = QmlJS::getInternalContext(declaration);

    if (!ctx) {
        return;
    }

    // No need to re-declare a field if it already exists
    if (ctx->findDeclarations(identifier.last(),
                              CursorInRevision::invalid(),
                              nullptr,
                              DUContext::DontSearchInParent).count() > 0) {
        return;
    }

    // The internal context of declaration is already closed and does not contain
    // location. This can be worked around by opening a new context, declaring the
    // new field in it, and then adding the context as a parent of
    // declaration->internalContext().
    RangeInRevision range = m_session->locationToRange(location);
    IntegralType::Ptr type = IntegralType::Ptr(new IntegralType(IntegralType::TypeMixed));
    DUContext* importedContext = openContext(node, range, DUContext::Class);

    openDeclaration<Declaration>(identifier, range);
    openType(type);
    closeAndAssignType();
    closeContext();

    ctx->addImportedParentContext(importedContext);
}

bool DeclarationBuilder::visit(QmlJS::AST::FieldMemberExpression* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    ExpressionType type = findType(node->base);

    if (type.declaration) {
        declareFieldMember(
            type.declaration,
            node->name.toString(),
            node,
            node->identifierToken
        );
    }

    return false;       // findType has already visited node->base
}

bool DeclarationBuilder::visit(QmlJS::AST::ArrayMemberExpression* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    // When the user types array["new_key"], declare "new_key" as a new field of
    // array.
    auto stringLiteral = QmlJS::AST::cast<QmlJS::AST::StringLiteral*>(node->expression);

    if (!stringLiteral) {
        return DeclarationBuilderBase::visit(node);
    }

    ExpressionType type = findType(node->base);

    if (type.declaration) {
        declareFieldMember(
            type.declaration,
            stringLiteral->value.toString(),
            node,
            stringLiteral->literalToken
        );
    }

    node->expression->accept(this);
    return false;       // findType has already visited node->base, and we have just visited node->expression
}

bool DeclarationBuilder::visit(QmlJS::AST::ObjectLiteral* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    // Object literals can appear in the "values" property of enumerations. Their
    // keys must be declared in the enumeration, not in an anonymous class
    if (currentContext()->type() == DUContext::Enum) {
        return DeclarationBuilderBase::visit(node);
    }

    // Open an anonymous class declaration, with its internal context
    StructureType::Ptr type(new StructureType);
    {
        DUChainWriteLocker lock;
        ClassDeclaration* decl = openDeclaration<ClassDeclaration>(
            QualifiedIdentifier(),
            QmlJS::emptyRangeOnLine(node->lbraceToken)
        );

        decl->setKind(Declaration::Type);
        decl->setInternalContext(openContext(
            node,
            m_session->locationsToRange(node->lbraceToken, node->rbraceToken),
            DUContext::Class
        ));

        type->setDeclaration(decl);
    }
    openType(type);

    return DeclarationBuilderBase::visit(node);
}

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
    ExpressionType type;
    bool inSymbolTable = false;

    if (currentContext()->type() == DUContext::Enum) {
        // This is an enumeration value
        auto value = QmlJS::AST::cast<QmlJS::AST::NumericLiteral*>(node->value);
        EnumeratorType::Ptr enumerator(new EnumeratorType);

        enumerator->setDataType(IntegralType::TypeInt);

        if (value) {
            enumerator->setValue((int)value->value);
        }

        type.type = AbstractType::Ptr::staticCast(enumerator);
        type.declaration = nullptr;
        inSymbolTable = true;
    } else {
        // Normal value
        type = findType(node->value);
    }

    // If a function is assigned to an object member, set the prototype context
    // of the function to the object containing the member
    if (type.declaration) {
        DUChainWriteLocker lock;
        auto func = type.declaration.dynamicCast<QmlJS::FunctionDeclaration>();

        if (func && !func->prototypeContext()) {
            func->setPrototypeContext(currentContext(), false);
        }
    }

    // Open the declaration
    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setInSymbolTable(inSymbolTable);
    }
    openType(type.type);

    return false;   // findType has already explored node->expression
}

void DeclarationBuilder::endVisit(QmlJS::AST::PropertyNameAndValue* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

void DeclarationBuilder::endVisit(QmlJS::AST::ObjectLiteral* node)
{
    DeclarationBuilderBase::endVisit(node);

    if (currentContext()->type() != DUContext::Enum) {
        // Enums are special-cased in visit(ObjectLiteral)
        closeContext();
        closeAndAssignType();
    }
}

/*
 * plugin.qmltypes files
 */
QualifiedIdentifier DeclarationBuilder::declareModule(const RangeInRevision& range)
{
    // Declare a namespace whose name is the base name of the current file
    QualifiedIdentifier name(m_session->moduleName() + m_session->moduleVersion());
    StructureType::Ptr type(new StructureType);

    {
        DUChainWriteLocker lock;
        Declaration* decl = openDeclaration<Declaration>(name, range);

        decl->setKind(Declaration::Namespace);
        type->setDeclaration(decl);
    }
    openType(type);

    return name;
}

void DeclarationBuilder::declareComponent(QmlJS::AST::UiObjectInitializer* node,
                                          const RangeInRevision &range,
                                          const QualifiedIdentifier &name)
{
    QString baseClass = QmlJS::getQMLAttributeValue(node->members, "prototype").value.section('/', -1, -1);

    // Declare the component itself
    StructureType::Ptr type(new StructureType);

    ClassDeclaration* decl;
    {
        DUChainWriteLocker lock;
        decl = openDeclaration<ClassDeclaration>(name, range);

        decl->setKind(Declaration::Type);
        decl->setClassType(ClassDeclarationData::Interface);
        decl->clearBaseClasses();

        if (!baseClass.isNull()) {
            addBaseClass(decl, baseClass);
        }

        type->setDeclaration(decl);
        decl->setType(type);            // declareExports needs to know the type of decl
    }
    openType(type);
}

void DeclarationBuilder::declareMethod(QmlJS::AST::UiObjectInitializer* node,
                                       const RangeInRevision &range,
                                       const QualifiedIdentifier &name,
                                       bool isSlot,
                                       bool isSignal)
{
    QString type_name = QmlJS::getQMLAttributeValue(node->members, "type").value;
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

void DeclarationBuilder::declareProperty(QmlJS::AST::UiObjectInitializer* node,
                                         const RangeInRevision &range,
                                         const QualifiedIdentifier &name)
{
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->members, "type").value);

    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setAbstractType(type);
    }
    openType(type);
}

void DeclarationBuilder::declareParameter(QmlJS::AST::UiObjectInitializer* node,
                                          const RangeInRevision &range,
                                          const QualifiedIdentifier &name)
{
    FunctionType::Ptr function = currentType<FunctionType>();
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->members, "type").value);

    function->addArgument(type);

    {
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
    }
    openType(type);
}

void DeclarationBuilder::declareEnum(const RangeInRevision &range,
                                     const QualifiedIdentifier &name)
{
    EnumerationType::Ptr type(new EnumerationType);

    {
        DUChainWriteLocker lock;
        ClassMemberDeclaration* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setKind(Declaration::Type);
        decl->setType(type);                // The type needs to be set here because closeContext is called before closeAndAssignType and needs to know the type of decl

        type->setDataType(IntegralType::TypeEnumeration);
        type->setDeclaration(decl);
    }
    openType(type);
}

void DeclarationBuilder::declareComponentSubclass(QmlJS::AST::UiObjectInitializer* node,
                                                  const KDevelop::RangeInRevision& range,
                                                  const QString& baseclass,
                                                  QmlJS::AST::UiQualifiedId* qualifiedId)
{
    QualifiedIdentifier name(
        QmlJS::getQMLAttributeValue(node->members, "name").value.section('/', -1, -1)
    );
    DUContext::ContextType contextType = DUContext::Class;

    if (baseclass == QLatin1String("Component")) {
        // QML component, equivalent to a QML class
        declareComponent(node, range, name);
    } else if (baseclass == QLatin1String("Method") ||
               baseclass == QLatin1String("Signal") ||
               baseclass == QLatin1String("Slot")) {
        // Method (that can also be a signal or a slot)
        declareMethod(node, range, name, baseclass == QLatin1String("Slot"), baseclass == QLatin1String("Signal"));
        contextType = DUContext::Function;
    } else if (baseclass == QLatin1String("Property")) {
        // A property
        declareProperty(node, range, name);
    } else if (baseclass == QLatin1String("Parameter") && currentType<FunctionType>()) {
        // One parameter of a signal/slot/method
        declareParameter(node, range, name);
    } else if (baseclass == QLatin1String("Enum")) {
        // Enumeration. The "values" key contains a dictionary of name -> number entries.
        declareEnum(range, name);
        contextType = DUContext::Enum;
        name = QualifiedIdentifier();   // Enum contexts should have no name so that their members have the correct scope
    } else if (baseclass == QLatin1String("Module")) {
        // QML Module, that declares a namespace
        name = declareModule(range);
        contextType = DUContext::Class;
    } else {
        // Define an anonymous subclass of the baseclass. This subclass will
        // be instantiated when "id:" is encountered
        name = QualifiedIdentifier();

        // Use ExpressionVisitor to find the declaration of the base class
        DeclarationPointer baseClass = findType(qualifiedId).declaration;
        StructureType::Ptr type(new StructureType);

        {
            DUChainWriteLocker lock;
            ClassDeclaration* decl = openDeclaration<ClassDeclaration>(
                currentContext()->type() == DUContext::Global ?
                    QualifiedIdentifier(m_session->moduleName()) :
                    name,
                QmlJS::emptyRangeOnLine(node->lbraceToken)
            );

            decl->clearBaseClasses();
            decl->setKind(Declaration::Type);
            decl->setType(type);                // The class needs to know its type early because it contains definitions that depend on that type
            type->setDeclaration(decl);

            if (baseClass) {
                addBaseClass(decl, baseClass->indexedType());
            }
        }
        openType(type);
    }

    // Open a context of the proper type and identifier
    openContext(
        node,
        m_session->locationsToInnerRange(node->lbraceToken, node->rbraceToken),
        contextType,
        name
    );

    DUContext* ctx = currentContext();
    Declaration* decl = currentDeclaration();
    ClassFunctionDeclaration* classDecl = dynamic_cast<ClassFunctionDeclaration*>(decl);

    {
        // Set the inner context of the current declaration, because nested classes
        // need to know the inner context of their parents
        DUChainWriteLocker lock;

        if (classDecl) {
            classDecl->setInternalFunctionContext(ctx);
        } else {
            decl->setInternalContext(ctx);
        }

        if (baseclass == QLatin1String("Module")) {
            // If we opened a namespace, give it a proper scope
            ctx->setLocalScopeIdentifier(decl->qualifiedIdentifier());
        } else if (contextType == DUContext::Enum) {
            ctx->setPropagateDeclarations(true);
        }
    }

    // If we have have declared a class, import the context of its base classes
    registerBaseClasses();
}

void DeclarationBuilder::declareComponentInstance(QmlJS::AST::ExpressionStatement* expression)
{
    if (!expression) {
        return;
    }

    auto identifier = QmlJS::AST::cast<QmlJS::AST::IdentifierExpression *>(expression->expression);

    if (!identifier) {
        return;
    }

    {
        DUChainWriteLocker lock;

        injectContext(topContext());
        Declaration* decl = openDeclaration<Declaration>(
            QualifiedIdentifier(identifier->name.toString()),
            m_session->locationToRange(identifier->identifierToken)
        );
        closeInjectedContext();

        // Put the declaration in the global scope
        decl->setKind(Declaration::Instance);
        decl->setType(currentAbstractType());
    }
    closeDeclaration();
}

DeclarationBuilder::ExportLiteralsAndNames DeclarationBuilder::exportedNames(QmlJS::AST::ExpressionStatement* exports)
{
    ExportLiteralsAndNames res;

    if (!exports) {
        return res;
    }

    auto exportslist = QmlJS::AST::cast<QmlJS::AST::ArrayLiteral*>(exports->expression);

    if (!exportslist) {
        return res;
    }

    // Explore all the exported symbols for this component and keep only those
    // having a version compatible with the one of this module
    QSet<QString> knownNames;

    for (auto it = exportslist->elements; it && it->expression; it = it->next) {
        auto stringliteral = QmlJS::AST::cast<QmlJS::AST::StringLiteral *>(it->expression);

        if (!stringliteral) {
            continue;
        }

        // String literal like "Namespace/Class version".
        QStringList nameAndVersion = stringliteral->value.toString().section('/', -1, -1).split(' ');
        QString name = nameAndVersion.at(0);
        QString version = (nameAndVersion.count() > 1 ? nameAndVersion.at(1) : QLatin1String("1.0"));

        if (!knownNames.contains(name)) {
            knownNames.insert(name);

            // Declare the components that appeared in a version lower or equal
            // to the version of this module. Lexicographic order can be used
            // because only the last digit of the version changes.
            if (version <= m_session->moduleVersion()) {
                res.append(qMakePair(stringliteral, name));
            }
        }
    }

    return res;
}


void DeclarationBuilder::declareExports(const ExportLiteralsAndNames& exports,
                                        ClassDeclaration* classdecl)
{
    DUChainWriteLocker lock;

    // Create the exported versions of the component
    for (auto exp : exports) {
        QmlJS::AST::StringLiteral* literal = exp.first;
        QString name = exp.second;
        StructureType::Ptr type(new StructureType);

        injectContext(currentContext()->parentContext());   // Don't declare the export in its C++-ish component, but in the scope above
        ClassDeclaration* decl = openDeclaration<ClassDeclaration>(
            QualifiedIdentifier(name),
            m_session->locationToRange(literal->literalToken)
        );
        closeInjectedContext();

        // The exported version inherits from the C++ component
        decl->setKind(Declaration::Type);
        decl->setClassType(ClassDeclarationData::Class);
        decl->clearBaseClasses();
        type->setDeclaration(decl);

        addBaseClass(decl, classdecl->indexedType());

        // Open a context for the exported class, and register its base class in it
        decl->setInternalContext(openContext(
            literal,
            DUContext::Class,
            QualifiedIdentifier(name)
        ));
        registerBaseClasses();
        closeContext();

        openType(type);
        closeAndAssignType();
    }
}

/*
 * UI
 */
void DeclarationBuilder::importDirectory(const QString& directory, QmlJS::AST::UiImport* node)
{
    DUChainWriteLocker lock;
    QString currentFilePath = currentContext()->topContext()->url().str();
    QFileInfo dir(directory);
    QFileInfoList entries;

    if (dir.isDir()) {
        // Import all the files in the given directory
        entries = QDir(directory).entryInfoList(
            QStringList() << (QLatin1String("*.") + currentFilePath.section(QLatin1Char('.'), -1, -1)),
            QDir::Files
        );
    } else if (dir.isFile()) {
        // Import the specific file given in the import statement
        entries.append(dir);
    } else {
        return;
    }

    if (node && !node->importId.isEmpty()) {
        // Open a namespace that will contain the declarations
        QualifiedIdentifier identifier(node->importId.toString());
        RangeInRevision range = m_session->locationToRange(node->importIdToken);

        Declaration* decl = openDeclaration<Declaration>(identifier, range);
        decl->setKind(Declaration::Namespace);
        decl->setInternalContext(openContext(node, range, DUContext::Class, identifier));
    }

    for (const QFileInfo& file : entries) {
        QString filePath = file.canonicalFilePath();

        if (filePath == currentFilePath) {
            continue;
        }

        ReferencedTopDUContext context = m_session->contextOfFile(filePath);

        if (!context) {
            continue;
        }

        // Add in this context one alias declaration for each local declaration
        // of context. This way, this context can see the declarations of
        // the other one, but not its imported parent contexts. This keeps
        // things simple and fast for the user.
        for (Declaration* decl : context->localDeclarations(currentContext()->topContext())) {
            if (decl->kind() != Declaration::Instance &&
                decl->kind() != Declaration::Type) {
                // Only import the top-level classes and instances, not the
                // import statements or namespace aliases
                continue;
            }
            AliasDeclaration* alias = openDeclaration<AliasDeclaration>(
                decl->qualifiedIdentifier(),
                RangeInRevision()
            );

            alias->setAliasedDeclaration(IndexedDeclaration(decl));
            closeDeclaration();
        }
    }

    if (node && !node->importId.isEmpty()) {
        // Close the namespace containing the declarations
        closeDeclaration();
        closeContext();
    }
}

void DeclarationBuilder::importModule(QmlJS::AST::UiImport* node)
{
    QmlJS::AST::UiQualifiedId *part = node->importUri;
    QString uri;

    while (part) {
        if (!uri.isEmpty()) {
            uri.append('.');
        }

        uri.append(part->name.toString());
        part = part->next;
    }

    // Version of the import
    QString version = m_session->symbolAt(node->versionToken);

    // Import the file corresponding to the URI
    ReferencedTopDUContext importedContext = m_session->contextOfModule(QString("%1_%2.qml").arg(uri, version));

    if (importedContext) {
        // Create a namespace import statement
        QualifiedIdentifier importedNamespaceName(uri + version);  // Modules contain namespaces like QtQuick1.0
        NamespaceAliasDeclaration* decl;
        DUChainWriteLocker lock;

        currentContext()->addImportedParentContext(
            importedContext,
            m_session->locationToRange(node->importToken).start
        );

        if (node->importId.isEmpty()) {
            decl = openDeclaration<NamespaceAliasDeclaration>(
                QualifiedIdentifier(globalImportIdentifier()),
                m_session->locationToRange(node->importToken)
            );
        } else {
            decl = openDeclaration<NamespaceAliasDeclaration>(
                QualifiedIdentifier(node->importId.toString()),
                m_session->locationToRange(node->importIdToken)
            );
        }

        decl->setImportIdentifier(importedNamespaceName);
        closeDeclaration();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiImport* node)
{
    if (node->importUri) {
        importModule(node);
    } else if (!node->fileName.isNull()) {
        QUrl currentFileUrl = currentContext()->topContext()->url().toUrl();
        QUrl importUrl = QUrl(node->fileName.toString());

        importDirectory(currentFileUrl.resolved(importUrl).toLocalFile(), node);
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectDefinition* node)
{
    setComment(node);

    // Do not crash if the user has typed an empty object definition
    if (!node->initializer || !node->initializer->members) {
        m_skipEndVisit.push(true);
        return DeclarationBuilderBase::visit(node);
    }

    RangeInRevision range(m_session->locationToRange(node->qualifiedTypeNameId->identifierToken));
    QString baseclass = node->qualifiedTypeNameId->name.toString();

    // "Component" needs special care: a component that appears only in a future
    // version of this module, or that already appeared in a former version, must
    // be skipped because it is useless
    ExportLiteralsAndNames exports;

    if (baseclass == QLatin1String("Component")) {
        QmlJS::AST::Statement* statement = QmlJS::getQMLAttribute(node->initializer->members, QLatin1String("exports"));

        exports = exportedNames(QmlJS::AST::cast<QmlJS::AST::ExpressionStatement *>(statement));

        if (exports.count() == 0 &&
            (statement || !m_session->moduleVersion().endsWith(QLatin1String(".0")))) {
            // No exported version of this component. The second part of the
            // condition allows non-exported components to still be processed
            // in the .0 revision of the module (so that QObject, not exported
            // in QML, is still accessible and declared in QtQuick 1.0 and 2.0)
            m_skipEndVisit.push(true);
            return false;
        }
    }

    // Declare the component subclass
    declareComponentSubclass(node->initializer, range, baseclass, node->qualifiedTypeNameId);

    // If we had a component with exported names, declare these exports
    if (baseclass == QLatin1String("Component")) {
        ClassDeclaration* classDecl = currentDeclaration<ClassDeclaration>();

        if (classDecl) {
            declareExports(exports, classDecl);
        }
    }

    m_skipEndVisit.push(false);
    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiObjectDefinition* node)
{
    DeclarationBuilderBase::endVisit(node);

    // Do not crash if the user has typed an empty object definition
    if (!m_skipEndVisit.pop()) {
        closeContext();
        closeAndAssignType();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiScriptBinding* node)
{
    setComment(node);

    if (!node->qualifiedId) {
        return DeclarationBuilderBase::visit(node);
    }

    // Special-case some binding names
    QString bindingName = node->qualifiedId->name.toString();

    if (bindingName == QLatin1String("id")) {
        // Instantiate a QML component: its type is the current type (the anonymous
        // QML class that surrounds the declaration)
        declareComponentInstance(QmlJS::AST::cast<QmlJS::AST::ExpressionStatement *>(node->statement));
    }

    // Use ExpressionVisitor to find the signal/property bound
    DeclarationPointer bindingDecl = findType(node->qualifiedId).declaration;
    DUChainPointer<ClassFunctionDeclaration> signal;

    // If a Javascript block is used as expression or if the script binding is a
    // slot, open a subcontext so that variables declared in the binding are kept
    // local, and the signal parameters can be visible to the slot
    if ((
            bindingDecl &&
            (signal = bindingDecl.dynamicCast<ClassFunctionDeclaration>()) &&
            signal->isSignal()
        ) ||
        node->statement->kind == QmlJS::AST::Node::Kind_Block) {

        openContext(
            node->statement,
            m_session->locationsToInnerRange(
                node->statement->firstSourceLocation(),
                node->statement->lastSourceLocation()
            ),
            DUContext::Other
        );

        // If this script binding is a slot, import the parameters of its signal
        if (signal && signal->isSignal() && signal->internalFunctionContext()) {
            DUChainWriteLocker lock;

            currentContext()->addImportedParentContext(signal->internalFunctionContext());
        }
    }

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiScriptBinding* node)
{
    QmlJS::AST::Visitor::endVisit(node);

    // If visit(UiScriptBinding) has opened a context, close it
    if (currentContext()->type() == DUContext::Other) {
        closeContext();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiObjectBinding* node)
{
    setComment(node);

    if (!node->qualifiedId || !node->qualifiedTypeNameId || !node->initializer) {
        return DeclarationBuilderBase::visit(node);
    }

    // Declare the component subclass. "Behavior on ... {}" is treated exactly
    // like "Behavior {}".
    RangeInRevision range = m_session->locationToRange(node->qualifiedTypeNameId->identifierToken);
    QString baseclass = node->qualifiedTypeNameId->name.toString();

    declareComponentSubclass(node->initializer, range, baseclass, node->qualifiedTypeNameId);

    return DeclarationBuilderBase::visit(node);
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiObjectBinding* node)
{
    DeclarationBuilderBase::endVisit(node);

    if (node->qualifiedId && node->qualifiedTypeNameId && node->initializer) {
        closeContext();
        closeAndAssignType();
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::UiPublicMember* node)
{
    setComment(node);

    RangeInRevision range = m_session->locationToRange(node->identifierToken);
    QualifiedIdentifier id(node->name.toString());
    QString typeName = node->memberType.toString();
    bool res = DeclarationBuilderBase::visit(node);

    // Build the type of the public member
    if (node->type == QmlJS::AST::UiPublicMember::Signal) {
        // Open a function declaration corresponding to this signal
        declareFunction<ClassFunctionDeclaration>(
            node,
            false,
            QualifiedIdentifier(node->name.toString()),
            m_session->locationToRange(node->identifierToken),
            node->parameters,
            m_session->locationToRange(node->identifierToken),  // The AST does not provide the location of the parens
            nullptr,
            RangeInRevision::invalid()
        );

        // This declaration is a signal and its return type is void
        {
            DUChainWriteLocker lock;

            currentDeclaration<ClassFunctionDeclaration>()->setIsSignal(true);
            currentType<FunctionType>()->setReturnType(typeFromName("void"));
        }
    } else {
        AbstractType::Ptr type;

        if (typeName == "alias") {
            // Property aliases take the type of their aliased property
            type = findType(node->statement).type;
            res = false;        // findType has already explored node->statement
        } else {
            type = typeFromName(typeName);

            if (node->typeModifier == QLatin1String("list")) {
                // QML list, noted "list<type>" in the source file
                ArrayType::Ptr array(new ArrayType);

                array->setElementType(type);
                type = array.cast<AbstractType>();
            }
        }

        {
            DUChainWriteLocker lock;
            Declaration* decl = openDeclaration<ClassMemberDeclaration>(id, range);

            decl->setInSymbolTable(false);
        }
        openType(type);
    }

    return res;
}

void DeclarationBuilder::endVisit(QmlJS::AST::UiPublicMember* node)
{
    DeclarationBuilderBase::endVisit(node);

    closeAndAssignType();
}

/*
 * Utils
 */
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
    auto type = IntegralType::TypeNone;
    QString realName = name;

    // Built-in types
    if (name == QLatin1String("string")) {
        type = IntegralType::TypeString;
    } else if (name == QLatin1String("bool")) {
        type = IntegralType::TypeBoolean;
    } else if (name == QLatin1String("int")) {
        type = IntegralType::TypeInt;
    } else if (name == QLatin1String("float")) {
        type = IntegralType::TypeFloat;
    } else if (name == QLatin1String("double") || name == QLatin1String("real")) {
        type = IntegralType::TypeDouble;
    } else if (name == QLatin1String("void")) {
        type = IntegralType::TypeVoid;
    } else if (name == QLatin1String("var") || name == QLatin1String("variant")) {
        type = IntegralType::TypeMixed;
    } else if (m_session->language() == QmlJS::Language::Qml) {
        // In QML files, some Qt type names need to be renamed to the QML equivalent
        if (name == QLatin1String("QFont")) {
            realName = QLatin1String("Font");
        } else if (name == QLatin1String("QColor")) {
            realName = QLatin1String("color");
        } else if (name == QLatin1String("QDateTime")) {
            realName = QLatin1String("date");
        } else if (name == QLatin1String("QDate")) {
            realName = QLatin1String("date");
        } else if (name == QLatin1String("QTime")) {
            realName = QLatin1String("time");
        } else if (name == QLatin1String("QRect") || name == QLatin1String("QRectF")) {
            realName = QLatin1String("rect");
        } else if (name == QLatin1String("QPoint") || name == QLatin1String("QPointF")) {
            realName = QLatin1String("point");
        } else if (name == QLatin1String("QSize") || name == QLatin1String("QSizeF")) {
            realName = QLatin1String("size");
        } else if (name == QLatin1String("QUrl")) {
            realName = QLatin1String("url");
        } else if (name == QLatin1String("QVector3D")) {
            realName = QLatin1String("vector3d");
        } else if (name.endsWith(QLatin1String("ScriptString"))) {
            // Q{Declarative,Qml}ScriptString represents a JS snippet
            FunctionType* func = new FunctionType;
            func->setReturnType(AbstractType::Ptr(new IntegralType(IntegralType::TypeVoid)));
            return AbstractType::Ptr(func);
        }
    }

    if (type == IntegralType::TypeNone) {
        // Not a built-in type, but a class
        return typeFromClassName(realName);
    } else {
        return AbstractType::Ptr(new IntegralType(type));
    }
}

AbstractType::Ptr DeclarationBuilder::typeFromClassName(const QString& name)
{
    DeclarationPointer decl = QmlJS::getDeclaration(QualifiedIdentifier(name), currentContext());

    if (decl) {
        return decl->abstractType();
    } else {
        return AbstractType::Ptr(new StructureType);
    }
}

void DeclarationBuilder::addBaseClass(ClassDeclaration* classDecl, const QString& name)
{
    addBaseClass(classDecl, typeFromClassName(name)->indexed());
}

void DeclarationBuilder::addBaseClass(ClassDeclaration* classDecl, const IndexedType& type)
{
    BaseClassInstance baseClass;

    baseClass.access = Declaration::Public;
    baseClass.virtualInheritance = false;
    baseClass.baseClass = type;

    classDecl->addBaseClass(baseClass);
}

void DeclarationBuilder::registerBaseClasses()
{
    ClassDeclaration* classdecl = currentDeclaration<ClassDeclaration>();
    DUContext *ctx = currentContext();

    if (classdecl) {
        DUChainWriteLocker lock;

        for (uint i=0; i<classdecl->baseClassesSize(); ++i)
        {
            const BaseClassInstance &baseClass = classdecl->baseClasses()[i];
            StructureType::Ptr baseType = StructureType::Ptr::dynamicCast(baseClass.baseClass.abstractType());
            TopDUContext* topctx = topContext();

            if (baseType && baseType->declaration(topctx)) {
                ctx->addImportedParentContext(
                    baseType->declaration(topctx)->logicalInternalContext(topctx),
                    CursorInRevision::invalid(),
                    ctx->localScopeIdentifier().isEmpty()   // Don't register anonymous classes to their base classes. This avoids "Inherited by , , , , ".
                );
            }
        }
    }
}