/*
    SPDX-FileCopyrightText: 2012 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "declarationbuilder.h"
#include "debug.h"

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
#include <util/algorithm.h>

#include "expressionvisitor.h"
#include "parsesession.h"
#include "functiondeclaration.h"
#include "functiontype.h"
#include "helper.h"
#include "cache.h"
#include "frameworks/nodejs.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <KLocalizedString>

using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder(ParseSession* session)
: m_prebuilding(false)
{
    m_session = session;
}

ReferencedTopDUContext DeclarationBuilder::build(const IndexedString& url,
                                                 QmlJS::AST::Node* node,
                                                 const ReferencedTopDUContext& updateContext_)
{
    Q_ASSERT(m_session->url() == url);

    ReferencedTopDUContext updateContext(updateContext_);
    // The declaration builder needs to run twice, so it can resolve uses of e.g. functions
    // which are called before they are defined (which is easily possible, due to JS's dynamic nature).
    if (!m_prebuilding) {
        qCDebug(KDEV_QMLJS_DUCHAIN) << "building, but running pre-builder first";
        auto  prebuilder = new DeclarationBuilder(m_session);

        prebuilder->m_prebuilding = true;
        updateContext = prebuilder->build(url, node, updateContext);

        qCDebug(KDEV_QMLJS_DUCHAIN) << "pre-builder finished";
        delete prebuilder;

        if (!m_session->allDependenciesSatisfied()) {
            qCDebug(KDEV_QMLJS_DUCHAIN) << "dependencies were missing, don't perform the second parsing pass";
            return updateContext;
        }
    } else {
        qCDebug(KDEV_QMLJS_DUCHAIN) << "prebuilding";
    }

    return DeclarationBuilderBase::build(url, node, updateContext);
}

void DeclarationBuilder::startVisiting(QmlJS::AST::Node* node)
{
    DUContext* builtinQmlContext = nullptr;

    if (QmlJS::isQmlFile(currentContext()) && !currentContext()->url().str().contains(QLatin1String("__builtin_qml.qml"))) {
        builtinQmlContext = m_session->contextOfFile(
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevqmljssupport/nodejsmodules/__builtin_qml.qml"))
        );
    }

    {
        DUChainWriteLocker lock;

        // Remove all the imported parent contexts: imports may have been edited
        // and there musn't be any leftover parent context
        currentContext()->topContext()->clearImportedParentContexts();

        // Initialize Node.js
        QmlJS::NodeJS::instance().initialize(this);

        // Built-in QML types (color, rect, etc)
        if (builtinQmlContext) {
            topContext()->addImportedParentContext(builtinQmlContext);
        }
    }

    DeclarationBuilderBase::startVisiting(node);
}

/*
 * Functions
 */
template<typename Decl>
void DeclarationBuilder::declareFunction(QmlJS::AST::Node* node,
                                         bool newPrototypeContext,
                                         const Identifier& name,
                                         const RangeInRevision& nameRange,
                                         QmlJS::AST::Node* parameters,
                                         const RangeInRevision& parametersRange,
                                         QmlJS::AST::Node* body,
                                         const RangeInRevision& bodyRange)
{
    setComment(node);

    // Declare the function
    QmlJS::FunctionType::Ptr func(new QmlJS::FunctionType);
    Decl* decl;

    {
        DUChainWriteLocker lock;

        decl = openDeclaration<Decl>(name, nameRange);
        decl->setKind(Declaration::Type);
        func->setDeclaration(decl);
        decl->setType(func);
    }
    openType(func);

    // Parameters, if any (a function must always have an internal function context,
    // so always open a context here even if there are no parameters)
    DUContext* parametersContext = openContext(
        node + 1,                                               // Don't call setContextOnNode on node, only the body context can be associated with node
        RangeInRevision(parametersRange.start, bodyRange.end),  // Ensure that this context contains both the parameters and the body
        DUContext::Function,
        QualifiedIdentifier(name)
    );

    if (parameters) {
        QmlJS::AST::Node::accept(parameters, this);
    }

    // The internal context of the function is its parameter context
    {
        DUChainWriteLocker lock;
        decl->setInternalContext(parametersContext);
    }

    // Open the prototype context, if any. This has to be done before the body
    // because this context is needed for "this" to be properly resolved
    // in it.
    if (newPrototypeContext) {
        DUChainWriteLocker lock;
        auto* d = reinterpret_cast<QmlJS::FunctionDeclaration*>(decl);

        d->setPrototypeContext(openContext(
            node + 2,                   // Don't call setContextOnNode on node, only the body context can be associated with node
            RangeInRevision(parametersRange.start, parametersRange.start),
            DUContext::Function,        // This allows QmlJS::getOwnerOfContext to know that the parent of this context is the function declaration
            QualifiedIdentifier(name)
        ));

        if (name != Identifier(QStringLiteral("Object"))) {
            // Every class inherit from Object
            QmlJS::importObjectContext(currentContext(), topContext());
        }

        closeContext();
    }

    // Body, if any (it is a child context of the parameters)
    openContext(
        node,
        bodyRange,
        DUContext::Other,
        QualifiedIdentifier(name)
    );

    if (body) {
        QmlJS::AST::Node::accept(body, this);
    }

    // Close the body context and then the parameters context
    closeContext();
    closeContext();
}

template<typename Node>
void DeclarationBuilder::declareParameters(Node* node, QmlJS::AST::UiQualifiedId* Node::*typeFunc)
{
    for (Node *plist = node; plist; plist = plist->next) {
        const Identifier name(plist->name.toString());
        const RangeInRevision range = m_session->locationToRange(plist->identifierToken);

        AbstractType::Ptr type = (typeFunc ?
            typeFromName((plist->*typeFunc)->name.toString()) :             // The typeAttribute attribute of plist contains the type name of the argument
            AbstractType::Ptr(new IntegralType(IntegralType::TypeMixed))    // No type information, use mixed
        );

        {
            DUChainWriteLocker lock;
            openDeclaration<Declaration>(name, range);
        }
        openType(type);
        closeAndAssignType();

        if (QmlJS::FunctionType::Ptr funType = currentType<QmlJS::FunctionType>()) {
            funType->addArgument(type);
        }
    }
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionDeclaration* node)
{
    declareFunction<QmlJS::FunctionDeclaration>(
        node,
        true,   // A function declaration always has its own prototype context
        Identifier(node->name.toString()),
        m_session->locationToRange(node->identifierToken),
        node->formals,
        m_session->locationsToRange(node->lparenToken, node->rparenToken),
        node->body,
        m_session->locationsToRange(node->lbraceToken, node->rbraceToken)
    );

    return false;
}

bool DeclarationBuilder::visit(QmlJS::AST::FunctionExpression* node)
{
    declareFunction<QmlJS::FunctionDeclaration>(
        node,
        false,
        Identifier(),
        QmlJS::emptyRangeOnLine(node->functionToken),
        node->formals,
        m_session->locationsToRange(node->lparenToken, node->rparenToken),
        node->body,
        m_session->locationsToRange(node->lbraceToken, node->rbraceToken)
    );

    return false;
}

bool DeclarationBuilder::visit(QmlJS::AST::FormalParameterList* node)
{
    declareParameters(node, (QmlJS::AST::UiQualifiedId* QmlJS::AST::FormalParameterList::*)nullptr);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::UiParameterList* node)
{
    declareParameters(node, &QmlJS::AST::UiParameterList::type);

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::ReturnStatement* node)
{
    if (QmlJS::FunctionType::Ptr func = currentType<QmlJS::FunctionType>()) {
        AbstractType::Ptr returnType;

        if (node->expression) {
            returnType = findType(node->expression).type;
        } else {
            returnType = new IntegralType(IntegralType::TypeVoid);
        }

        DUChainWriteLocker lock;

        func->setReturnType(QmlJS::mergeTypes(func->returnType(), returnType));
    }

    return false;   // findType has already explored node
}

void DeclarationBuilder::endVisitFunction()
{
    QmlJS::FunctionType::Ptr func = currentType<QmlJS::FunctionType>();

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
void DeclarationBuilder::inferArgumentsFromCall(QmlJS::AST::Node* base, QmlJS::AST::ArgumentList* arguments)
{
    ContextBuilder::ExpressionType expr = findType(base);
    auto func_type = expr.type.dynamicCast<QmlJS::FunctionType>();
    DUChainWriteLocker lock;

    if (!func_type) {
        return;
    }

    auto func_declaration = dynamic_cast<FunctionDeclaration*>(func_type->declaration(topContext()));

    if (!func_declaration || !func_declaration->internalContext()) {
        return;
    }

    // Put the argument nodes in a list that has a definite size
    QVector<Declaration *> argumentDecls = func_declaration->internalContext()->localDeclarations();
    QVector<QmlJS::AST::ArgumentList *> args;

    for (auto argument = arguments; argument; argument = argument->next) {
        args.append(argument);
    }

    // Don't update a function when it is called with the wrong number
    // of arguments
    if (args.size() != argumentDecls.count()) {
        return;
    }

    // Update the types of the function arguments
    QmlJS::FunctionType::Ptr new_func_type(new QmlJS::FunctionType);

    for (int i=0; i<args.size(); ++i) {
        QmlJS::AST::ArgumentList *argument = args.at(i);
        AbstractType::Ptr current_type = argumentDecls.at(i)->abstractType();

        // Merge the current type of the argument with its type in the call expression
        AbstractType::Ptr call_type = findType(argument->expression).type;
        AbstractType::Ptr new_type = QmlJS::mergeTypes(current_type, call_type);

        // Update the declaration of the argument and its type in the function type
        if (func_declaration->topContext() == topContext()) {
            new_func_type->addArgument(new_type);
            argumentDecls.at(i)->setAbstractType(new_type);
        }

        // Add a warning if it is possible that the argument types don't match
        if (!m_prebuilding && !areTypesEqual(current_type, call_type)) {
            m_session->addProblem(argument, i18n(
                "Possible type mismatch between the argument type (%1) and the value passed as argument (%2)",
                current_type->toString(),
                call_type->toString()
            ), IProblem::Hint);
        }
    }

    // Replace the function's type with the new type having updated arguments
    if (func_declaration->topContext() == topContext()) {
        new_func_type->setReturnType(func_type->returnType());
        new_func_type->setDeclaration(func_declaration);
        func_declaration->setAbstractType(new_func_type);

        if (expr.declaration) {
            // expr.declaration is the variable that contains the function, while
            // func_declaration is the declaration of the function. They can be
            // different and both need to be updated
            expr.declaration->setAbstractType(new_func_type);
        }
    }

    return;
}

bool DeclarationBuilder::visit(QmlJS::AST::VariableDeclaration* node)
{
    setComment(m_session->commentForLocation(node->firstSourceLocation()).toUtf8());

    const Identifier name(node->name.toString());
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
            DUContext* leftInternalCtx = QmlJS::getInternalContext(leftType.declaration);

            // object.prototype.method = function(){} : when assigning a function
            // to a variable living in a Class context, set the prototype
            // context of the function to the context of the variable
            if (rightType.declaration && leftCtx->type() == DUContext::Class) {
                auto func = rightType.declaration.dynamicCast<QmlJS::FunctionDeclaration>();

                if (!QmlJS::getOwnerOfContext(leftCtx) && !leftCtx->importers().isEmpty()) {
                    // MyClass.prototype.myfunc declares "myfunc" in a small context
                    // that is imported by MyClass. The prototype of myfunc should
                    // be the context of MyClass, not the small context in which
                    // it has been declared
                    leftCtx = leftCtx->importers().at(0);
                }

                if (func && !func->prototypeContext()) {
                    func->setPrototypeContext(leftCtx);
                }
            }

            if (leftType.declaration->topContext() != topContext()) {
                // Do not modify a declaration of another file
            } else if (leftType.isPrototype && leftInternalCtx) {
                // Assigning something to a prototype is equivalent to making it
                // inherit from a class: "Class.prototype = ClassOrObject;"
                leftInternalCtx->clearImportedParentContexts();

                QmlJS::importDeclarationInContext(
                    leftInternalCtx,
                    rightType.declaration
                );
            } else {
                // Merge the already-known type of the variable with the new one
                leftType.declaration->setAbstractType(QmlJS::mergeTypes(leftType.type, rightType.type));
            }
        }

        return false;   // findType has already explored node
    }

    return DeclarationBuilderBase::visit(node);
}

bool DeclarationBuilder::visit(QmlJS::AST::CallExpression* node)
{
    inferArgumentsFromCall(node->base, node->arguments);
    return false;
}

bool DeclarationBuilder::visit(QmlJS::AST::NewMemberExpression* node)
{
    inferArgumentsFromCall(node->base, node->arguments);
    return false;
}

/*
 * Arrays
 */
void DeclarationBuilder::declareFieldMember(const KDevelop::DeclarationPointer& declaration,
                                            const QString& member,
                                            QmlJS::AST::Node* node,
                                            const QmlJS::AST::SourceLocation& location)
{
    if (QmlJS::isPrototypeIdentifier(member)) {
        // Don't declare "prototype", this is a special member
        return;
    }

    if (!m_session->allDependenciesSatisfied()) {
        // Don't declare anything automatically if dependencies are missing: the
        // checks hereafter may pass now but fail later, thus causing disappearing
        // declarations
        return;
    }

    DUChainWriteLocker lock;
    Identifier identifier(member);

    // Declaration must have an internal context so that the member can be added
    // into it.
    DUContext* ctx = QmlJS::getInternalContext(declaration);

    if (!ctx || ctx->topContext() != topContext()) {
        return;
    }

    // No need to re-declare a field if it already exists
    // TODO check if we can make getDeclaration receive an Identifier directly
    if (QmlJS::getDeclaration(QualifiedIdentifier(identifier), ctx, false)) {
        return;
    }

    // The internal context of declaration is already closed and does not contain
    // location. This can be worked around by opening a new context, declaring the
    // new field in it, and then adding the context as a parent of
    // declaration->internalContext().
    RangeInRevision range = m_session->locationToRange(location);
    IntegralType::Ptr type = IntegralType::Ptr(new IntegralType(IntegralType::TypeMixed));
    DUContext* importedContext = openContext(node, range, DUContext::Class);
    auto* decl = openDeclaration<Declaration>(identifier, range);

    decl->setInSymbolTable(false);  // This declaration is in an anonymous context, and the symbol table acts as if the declaration was in the global context
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
        auto* decl = openDeclaration<ClassDeclaration>(
            Identifier(),
            QmlJS::emptyRangeOnLine(node->lbraceToken)
        );

        decl->setKind(Declaration::Type);
        decl->setInternalContext(openContext(
            node,
            m_session->locationsToRange(node->lbraceToken, node->rbraceToken),
            DUContext::Class
        ));

        type->setDeclaration(decl);

        // Every object literal inherits from Object
        QmlJS::importObjectContext(currentContext(), topContext());
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
    Identifier name(QmlJS::getNodeValue(node->name));

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

        type.type = enumerator;
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
            func->setPrototypeContext(currentContext());
        }
    }

    // Open the declaration
    {
        DUChainWriteLocker lock;
        auto* decl = openDeclaration<ClassMemberDeclaration>(name, range);

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
 * plugins.qmltypes files
 */
void DeclarationBuilder::declareComponent(QmlJS::AST::UiObjectInitializer* node,
                                          const RangeInRevision &range,
                                          const Identifier &name)
{
    QString baseClass = QmlJS::getQMLAttributeValue(node->members, QStringLiteral("prototype")).value.section(QLatin1Char('/'), -1, -1);

    // Declare the component itself
    StructureType::Ptr type(new StructureType);

    ClassDeclaration* decl;
    {
        DUChainWriteLocker lock;
        decl = openDeclaration<ClassDeclaration>(name, range);

        decl->setKind(Declaration::Type);
        decl->setClassType(ClassDeclarationData::Interface);
        decl->clearBaseClasses();

        if (!baseClass.isEmpty()) {
            addBaseClass(decl, baseClass);
        }

        type->setDeclaration(decl);
        decl->setType(type);            // declareExports needs to know the type of decl
    }
    openType(type);
}

void DeclarationBuilder::declareMethod(QmlJS::AST::UiObjectInitializer* node,
                                       const RangeInRevision &range,
                                       const Identifier &name,
                                       bool isSlot,
                                       bool isSignal)
{
    QString type_name = QmlJS::getQMLAttributeValue(node->members, QStringLiteral("type")).value;
    QmlJS::FunctionType::Ptr type(new QmlJS::FunctionType);

    if (type_name.isEmpty()) {
        type->setReturnType(typeFromName(QStringLiteral("void")));
    } else {
        type->setReturnType(typeFromName(type_name));
    }

    {
        DUChainWriteLocker lock;
        auto* decl = openDeclaration<ClassFunctionDeclaration>(name, range);

        decl->setIsSlot(isSlot);
        decl->setIsSignal(isSignal);
        type->setDeclaration(decl);
    }
    openType(type);
}

void DeclarationBuilder::declareProperty(QmlJS::AST::UiObjectInitializer* node,
                                         const RangeInRevision &range,
                                         const Identifier &name)
{
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->members, QStringLiteral("type")).value);

    {
        DUChainWriteLocker lock;
        auto* decl = openDeclaration<ClassMemberDeclaration>(name, range);

        decl->setAbstractType(type);
    }
    openType(type);
}

void DeclarationBuilder::declareParameter(QmlJS::AST::UiObjectInitializer* node,
                                          const RangeInRevision &range,
                                          const Identifier &name)
{
    QmlJS::FunctionType::Ptr function = currentType<QmlJS::FunctionType>();
    AbstractType::Ptr type = typeFromName(QmlJS::getQMLAttributeValue(node->members, QStringLiteral("type")).value);

    Q_ASSERT(function);
    function->addArgument(type);

    {
        DUChainWriteLocker lock;
        openDeclaration<Declaration>(name, range);
    }
    openType(type);
}

void DeclarationBuilder::declareEnum(const RangeInRevision &range,
                                     const Identifier &name)
{
    EnumerationType::Ptr type(new EnumerationType);

    {
        DUChainWriteLocker lock;
        auto* decl = openDeclaration<ClassMemberDeclaration>(name, range);

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
    Identifier name(
        QmlJS::getQMLAttributeValue(node->members, QStringLiteral("name")).value.section(QLatin1Char('/'), -1, -1)
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
    } else if (baseclass == QLatin1String("Parameter") && currentType<QmlJS::FunctionType>()) {
        // One parameter of a signal/slot/method
        declareParameter(node, range, name);
    } else if (baseclass == QLatin1String("Enum")) {
        // Enumeration. The "values" key contains a dictionary of name -> number entries.
        declareEnum(range, name);
        contextType = DUContext::Enum;
        name = Identifier();   // Enum contexts should have no name so that their members have the correct scope
    } else {
        // Define an anonymous subclass of the baseclass. This subclass will
        // be instantiated when "id:" is encountered
        name = Identifier();

        // Use ExpressionVisitor to find the declaration of the base class
        DeclarationPointer baseClass = findType(qualifiedId).declaration;
        StructureType::Ptr type(new StructureType);

        {
            DUChainWriteLocker lock;
            auto* decl = openDeclaration<ClassDeclaration>(
                currentContext()->type() == DUContext::Global ?
                    Identifier(m_session->moduleName()) :
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
        QualifiedIdentifier(name)
    );

    DUContext* ctx = currentContext();
    Declaration* decl = currentDeclaration();

    {
        // Set the inner context of the current declaration, because nested classes
        // need to know the inner context of their parents
        DUChainWriteLocker lock;

        decl->setInternalContext(ctx);

        if (contextType == DUContext::Enum) {
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
        auto* decl = openDeclaration<Declaration>(
            Identifier(identifier->name.toString()),
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
        QStringList nameAndVersion = stringliteral->value.toString().section(QLatin1Char('/'), -1, -1).split(QLatin1Char(' '));
        QString name = nameAndVersion.at(0);

        if (Algorithm::insert(knownNames, name).inserted) {
            res.append(qMakePair(stringliteral, name));
        }
    }

    return res;
}


void DeclarationBuilder::declareExports(const ExportLiteralsAndNames& exports,
                                        ClassDeclaration* classdecl)
{
    DUChainWriteLocker lock;

    // Create the exported versions of the component
    for (auto& exp : exports) {
        QmlJS::AST::StringLiteral* literal = exp.first;
        QString name = exp.second;
        StructureType::Ptr type(new StructureType);

        injectContext(currentContext()->parentContext());   // Don't declare the export in its C++-ish component, but in the scope above
        auto* decl = openDeclaration<ClassDeclaration>(
            Identifier(name),
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
            QStringList{
                (QLatin1String("*.") + currentFilePath.section(QLatin1Char('.'), -1, -1)),
                QStringLiteral("*.qmltypes"),
                QStringLiteral("*.so")},
            QDir::Files
        );
    } else if (dir.isFile()) {
        // Import the specific file given in the import statement
        entries.append(dir);
    } else if (!m_prebuilding) {
        m_session->addProblem(node, i18n("Module not found, some types or properties may not be recognized"));
        return;
    }

    // Translate the QFileInfos into QStrings (and replace .so files with
    // qmlplugindump dumps)
    lock.unlock();
    const QStringList filePaths = QmlJS::Cache::instance().getFileNames(entries);
    lock.lock();

    if (node && !node->importId.isEmpty()) {
        // Open a namespace that will contain the declarations
        Identifier identifier(node->importId.toString());
        RangeInRevision range = m_session->locationToRange(node->importIdToken);

        auto* decl = openDeclaration<Declaration>(identifier, range);
        decl->setKind(Declaration::Namespace);
        decl->setInternalContext(openContext(node, range, DUContext::Class, QualifiedIdentifier(identifier)));
    }

    for (const QString& filePath : filePaths) {
        if (filePath == currentFilePath) {
            continue;
        }

        ReferencedTopDUContext context = m_session->contextOfFile(filePath);

        if (context) {
            currentContext()->addImportedParentContext(context.data());
        }
    }

    if (node && !node->importId.isEmpty()) {
        // Close the namespace containing the declarations
        closeContext();
        closeDeclaration();
    }
}

void DeclarationBuilder::importModule(QmlJS::AST::UiImport* node)
{
    QmlJS::AST::UiQualifiedId *part = node->importUri;
    QString uri;

    while (part) {
        if (!uri.isEmpty()) {
            uri.append(QLatin1Char('.'));
        }

        uri.append(part->name.toString());
        part = part->next;
    }

    // Version of the import
    QString version = m_session->symbolAt(node->versionToken);

    // Import the directory containing the module
    QString modulePath = QmlJS::Cache::instance().modulePath(m_session->url(), uri, version);
    importDirectory(modulePath, node);
}

bool DeclarationBuilder::visit(QmlJS::AST::UiImport* node)
{
    if (node->importUri) {
        importModule(node);
    } else if (!node->fileName.isEmpty() && node->fileName != QLatin1String(".")) {
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
        QmlJS::AST::Statement* statement = QmlJS::getQMLAttribute(node->initializer->members, QStringLiteral("exports"));

        exports = exportedNames(QmlJS::AST::cast<QmlJS::AST::ExpressionStatement *>(statement));

        if (statement && exports.count() == 0) {
            // This component has an "exports:" member but no export matched
            // the version of this module. Skip the component
            m_skipEndVisit.push(true);
            return false;
        }
    } else if (baseclass == QLatin1String("Module")) {
        // "Module" is disabled. This allows the declarations of a module
        // dump to appear in the same namespace as the .qml files in the same
        // directory.
        m_skipEndVisit.push(true);
        return true;
    }

    // Declare the component subclass
    declareComponentSubclass(node->initializer, range, baseclass, node->qualifiedTypeNameId);

    // If we had a component with exported names, declare these exports
    if (baseclass == QLatin1String("Component")) {
        auto* classDecl = currentDeclaration<ClassDeclaration>();

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
        if (signal && signal->isSignal() && signal->internalContext()) {
            DUChainWriteLocker lock;

            currentContext()->addIndirectImport(DUContext::Import(
                signal->internalContext(),
                nullptr
            ));
        }
    } else {
        // Check that the type of the value matches the type of the property
        AbstractType::Ptr expressionType = findType(node->statement).type;
        DUChainReadLocker lock;

        if (!m_prebuilding && bindingDecl && !areTypesEqual(bindingDecl->abstractType(), expressionType)) {
            m_session->addProblem(node->qualifiedId, i18n(
                "Mismatch between the value type (%1) and the property type (%2)",
                expressionType->toString(),
                bindingDecl->abstractType()->toString()
            ), IProblem::Error);
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
    Identifier id(node->name.toString());
    QString typeName = node->memberTypeName().toString();
    bool res = DeclarationBuilderBase::visit(node);

    // Build the type of the public member
    if (node->type == QmlJS::AST::UiPublicMember::Signal) {
        // Open a function declaration corresponding to this signal
        declareFunction<ClassFunctionDeclaration>(
            node,
            false,
            Identifier(node->name.toString()),
            m_session->locationToRange(node->identifierToken),
            node->parameters,
            m_session->locationToRange(node->identifierToken),  // The AST does not provide the location of the parens
            nullptr,
            m_session->locationToRange(node->identifierToken)   // A body range must be provided
        );

        // This declaration is a signal and its return type is void
        {
            DUChainWriteLocker lock;

            currentDeclaration<ClassFunctionDeclaration>()->setIsSignal(true);
            currentType<QmlJS::FunctionType>()->setReturnType(typeFromName(QStringLiteral("void")));
        }
    } else {
        AbstractType::Ptr type;

        if (typeName == QLatin1String("alias")) {
            // Property aliases take the type of their aliased property
            type = findType(node->statement).type;
            res = false;        // findType has already explored node->statement
        } else {
            type = typeFromName(typeName);

            if (node->typeModifier == QLatin1String("list")) {
                // QML list, noted "list<type>" in the source file
                ArrayType::Ptr array(new ArrayType);
                array->setElementType(type);
                type = array;
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

    if (auto type = lastType()) {
        DUChainWriteLocker lock;
        dec->setType(type);
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
    } else if (name == QLatin1String("half")) {
        type = IntegralType::TypeHalf;
    } else if (name == QLatin1String("float")) {
        type = IntegralType::TypeFloat;
    } else if (name == QLatin1String("double") || name == QLatin1String("real")) {
        type = IntegralType::TypeDouble;
    } else if (name == QLatin1String("void")) {
        type = IntegralType::TypeVoid;
    } else if (name == QLatin1String("var") || name == QLatin1String("variant")) {
        type = IntegralType::TypeMixed;
    } else if (m_session->language() == QmlJS::Dialect::Qml) {
        // In QML files, some Qt type names need to be renamed to the QML equivalent
        if (name == QLatin1String("QFont")) {
            realName = QStringLiteral("Font");
        } else if (name == QLatin1String("QColor")) {
            realName = QStringLiteral("color");
        } else if (name == QLatin1String("QDateTime")) {
            realName = QStringLiteral("date");
        } else if (name == QLatin1String("QDate")) {
            realName = QStringLiteral("date");
        } else if (name == QLatin1String("QTime")) {
            realName = QStringLiteral("time");
        } else if (name == QLatin1String("QRect") || name == QLatin1String("QRectF")) {
            realName = QStringLiteral("rect");
        } else if (name == QLatin1String("QPoint") || name == QLatin1String("QPointF")) {
            realName = QStringLiteral("point");
        } else if (name == QLatin1String("QSize") || name == QLatin1String("QSizeF")) {
            realName = QStringLiteral("size");
        } else if (name == QLatin1String("QUrl")) {
            realName = QStringLiteral("url");
        } else if (name == QLatin1String("QVector3D")) {
            realName = QStringLiteral("vector3d");
        } else if (name.endsWith(QLatin1String("ScriptString"))) {
            // Q{Declarative,Qml}ScriptString represents a JS snippet
            auto  func = new QmlJS::FunctionType;
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

    if (!decl) {
        if (name == QLatin1String("QRegExp")) {
            decl = QmlJS::NodeJS::instance().moduleMember(QStringLiteral("__builtin_ecmascript"), QStringLiteral("RegExp"), currentContext()->url());
        }
    }

    if (decl) {
        return decl->abstractType();
    } else {
        DelayedType::Ptr type(new DelayedType);
        type->setKind(DelayedType::Unresolved);
        type->setIdentifier(IndexedTypeIdentifier(name));
        return type;
    }
}

void DeclarationBuilder::addBaseClass(ClassDeclaration* classDecl, const QString& name)
{
    addBaseClass(classDecl, IndexedType(typeFromClassName(name)));
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
    auto* classdecl = currentDeclaration<ClassDeclaration>();
    DUContext *ctx = currentContext();

    if (classdecl) {
        DUChainWriteLocker lock;

        for (uint i=0; i<classdecl->baseClassesSize(); ++i)
        {
            const BaseClassInstance &baseClass = classdecl->baseClasses()[i];
            auto baseType = baseClass.baseClass.abstractType().dynamicCast<StructureType>();
            TopDUContext* topctx = topContext();

            if (baseType && baseType->declaration(topctx)) {
                QmlJS::importDeclarationInContext(ctx, DeclarationPointer(baseType->declaration(topctx)));
            }
        }
    }
}

static bool enumContainsEnumerator(const AbstractType::Ptr& a, const AbstractType::Ptr& b)
{
    Q_ASSERT(a->whichType() == AbstractType::TypeEnumeration);
    auto aEnum = a.staticCast<EnumerationType>();
    Q_ASSERT(b->whichType() == AbstractType::TypeEnumerator);
    auto bEnumerator = b.staticCast<EnumeratorType>();
    return bEnumerator->qualifiedIdentifier().beginsWith(aEnum->qualifiedIdentifier());
}

static bool isNumeric(const IntegralType::Ptr& type)
{
    return type->dataType() == IntegralType::TypeInt
        || type->dataType() == IntegralType::TypeIntegral
        || type->dataType() == IntegralType::TypeHalf
        || type->dataType() == IntegralType::TypeFloat
        || type->dataType() == IntegralType::TypeDouble;
}

bool DeclarationBuilder::areTypesEqual(const AbstractType::Ptr& a, const AbstractType::Ptr& b)
{
    if (!a || !b) {
        return true;
    }

    if (a->whichType() == AbstractType::TypeUnsure || b->whichType() == AbstractType::TypeUnsure) {
        // Don't try to guess something if one of the types is unsure
        return true;
    }

    const auto bIntegral = b.dynamicCast<IntegralType>();
    if (bIntegral && (bIntegral->dataType() == IntegralType::TypeString || bIntegral->dataType() == IntegralType::TypeMixed)) {
        // In QML/JS, a string can be converted to nearly everything else, similarly ignore mixed types
        return true;
    }

    const auto aIntegral = a.dynamicCast<IntegralType>();
    if (aIntegral && (aIntegral->dataType() == IntegralType::TypeString || aIntegral->dataType() == IntegralType::TypeMixed)) {
        // In QML/JS, nearly everything can be to a string, similarly ignore mixed types
        return true;
    }
    if (aIntegral && bIntegral) {
        if (isNumeric(aIntegral) && isNumeric(bIntegral)) {
            // Casts between integral types is possible
            return true;
        }
    }

    if (a->whichType() == AbstractType::TypeEnumeration && b->whichType() == AbstractType::TypeEnumerator) {
        return enumContainsEnumerator(a, b);
    } else if (a->whichType() == AbstractType::TypeEnumerator && b->whichType() == AbstractType::TypeEnumeration) {
        return enumContainsEnumerator(b, a);
    }

    {
        auto aId = dynamic_cast<const IdentifiedType*>(a.constData());
        auto bId = dynamic_cast<const IdentifiedType*>(b.constData());
        if (aId && bId && aId->qualifiedIdentifier() == bId->qualifiedIdentifier())
            return true;
    }

    {
        auto aStruct = a.dynamicCast<StructureType>();
        auto bStruct = b.dynamicCast<StructureType>();
        if (aStruct && bStruct) {
            auto top = currentContext()->topContext();
            auto aDecl = dynamic_cast<ClassDeclaration*>(aStruct->declaration(top));
            auto bDecl = dynamic_cast<ClassDeclaration*>(bStruct->declaration(top));
            if (aDecl && bDecl) {
                if (aDecl->isPublicBaseClass(bDecl, top) || bDecl->isPublicBaseClass(aDecl, top)) {
                    return true;
                }
            }
        }
    }

    return a->equals(b.constData());
}
