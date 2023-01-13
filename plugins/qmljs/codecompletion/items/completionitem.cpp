/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "completionitem.h"
#include "context.h"

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/structuretype.h>

#include "../../duchain/functiontype.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace QmlJS;
using namespace KDevelop;

CompletionItem::CompletionItem(const DeclarationPointer& decl, int inheritanceDepth, Decoration decoration)
: NormalDeclarationCompletionItem(decl, QExplicitlySharedDataPointer<KDevelop::CodeCompletionContext>(), inheritanceDepth),
  m_decoration(decoration)
{
}

QVariant CompletionItem::data(const QModelIndex& index, int role, const CodeCompletionModel* model) const
{
    DUChainReadLocker lock;
    Declaration* decl = declaration().data();

    if (!decl) {
        return QVariant();
    }

    auto* classDecl = dynamic_cast<ClassDeclaration *>(decl);
    auto declType = decl->abstractType().dynamicCast<StructureType>();
    auto funcType = decl->abstractType().dynamicCast<QmlJS::FunctionType>();

    if (role == CodeCompletionModel::BestMatchesCount) {
        return 5;
    } else if (role == CodeCompletionModel::MatchQuality) {
        AbstractType::Ptr referenceType =
            static_cast<QmlJS::CodeCompletionContext*>(model->completionContext().data())->typeToMatch();

        if (!referenceType) {
            return QVariant();
        }

        AbstractType::Ptr declType = decl->abstractType();
        if (!declType) {
          return QVariant();
        }

        auto declFunc = declType.dynamicCast<QmlJS::FunctionType>();

        if (declType->equals(referenceType.constData())) {
            // Perfect type match
            return QVariant(10);
        } else if (declFunc && declFunc->returnType() &&
                   declFunc->returnType()->equals(referenceType.constData())) {
            // Also very nice: a function returning the proper type
            return QVariant(9);
        } else {
            // Completely different types, no luck
            return QVariant();
        }
    } else if (role == Qt::DisplayRole && funcType) {
        // Functions are displayed using the "type funcName(arg, arg, arg...)" format
        Declaration* funcDecl = funcType->declaration(decl->topContext());

        if (funcDecl) {
            switch (index.column()) {
            case CodeCompletionModel::Prefix:
                return funcType->returnType()->toString();
            case CodeCompletionModel::Name:
                // Return the identifier of the declaration being listed, not of its
                // function declaration (because the function may have been declared
                // anonymously, even if it has been assigned to a variable)
                return decl->identifier().toString();
            case CodeCompletionModel::Arguments:
            {
                QStringList args;
                const auto localDeclarations = funcDecl->internalContext()->localDeclarations();
                args.reserve(localDeclarations.size());
                for (auto* arg : localDeclarations) {
                    args.append(arg->toString());
                }

                return QVariant(QLatin1Char('(') + args.join(QLatin1String(", ")) +QLatin1Char(')'));
            }
            }
        }
    } else if (role == Qt::DisplayRole && index.column() == CodeCompletionModel::Prefix) {
        if (classDecl) {
            if (classDecl->classType() == ClassDeclarationData::Class) {
                // QML component
                return QStringLiteral("component");
            } else if (classDecl->classType() == ClassDeclarationData::Interface) {
                // C++-ish QML component
                return QStringLiteral("wrapper");
            }
        }

        if (decl && (
                decl->kind() == Declaration::NamespaceAlias ||
                decl->kind() == Declaration::Namespace
           )) {
            // Display namespaces and namespace aliases as modules
            return QStringLiteral("module");
        }

        if (decl && decl->abstractType() &&
            decl->kind() == Declaration::Type &&
            decl->abstractType()->whichType() == AbstractType::TypeEnumeration) {
            // Enum
            return QStringLiteral("enum");
        }

        if (declType &&
            decl->kind() == Declaration::Instance &&
            declType->declarationId().qualifiedIdentifier().isEmpty()) {
            // QML component instance. The type that should be displayed is the
            // base class of its anonymous class
            auto* anonymousClass = dynamic_cast<ClassDeclaration *>(declType->declaration(decl->topContext()));

            if (anonymousClass && anonymousClass->baseClassesSize() > 0) {
                return anonymousClass->baseClasses()[0].baseClass.abstractType()->toString();
            }
        }
    }

    return NormalDeclarationCompletionItem::data(index, role, model);
}

QString CompletionItem::declarationName() const
{
    ClassFunctionDeclaration* classFuncDecl = dynamic_cast<ClassFunctionDeclaration *>(declaration().data());

    if (classFuncDecl && classFuncDecl->isSignal() && m_decoration == QmlJS::CompletionItem::ColonOrBracket) {
        // Signals, when completed in a QML component context, are transformed into slots
        QString signal = classFuncDecl->identifier().toString();

        if (signal.size() > 0) {
            return QLatin1String("on") + signal.at(0).toUpper() + signal.midRef(1);
        }
    }

    return NormalDeclarationCompletionItem::declarationName();
}

CodeCompletionModel::CompletionProperties CompletionItem::completionProperties() const
{
    DUChainReadLocker lock;

    // Variables having a function type should have a function icon. FunctionDeclarations
    // are skipped here because they are already handled properly by completionProperties()
    if (declaration() && declaration()->abstractType() &&
        !declaration()->isFunctionDeclaration() &&
        declaration()->abstractType()->whichType() == AbstractType::TypeFunction) {
        return CodeCompletionModel::Function;
    }

    // Put declarations in a context owned by a namespace in the namespace scope
    auto properties = NormalDeclarationCompletionItem::completionProperties();

    if (declaration() && declaration()->context() && declaration()->context()->owner() && (
            declaration()->context()->owner()->kind() == Declaration::Namespace ||
            declaration()->context()->type() == DUContext::Enum
        )) {
        properties &= ~(CodeCompletionModel::LocalScope | CodeCompletionModel::GlobalScope | CodeCompletionModel::Public);
        properties |= CodeCompletionModel::NamespaceScope;
    }

    return properties;
}

void CompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    KTextEditor::Document* document = view->document();
    QString base = declarationName();

    switch (m_decoration)
    {
    case QmlJS::CompletionItem::NoDecoration:
        document->replaceText(word, base);
        break;

    case QmlJS::CompletionItem::Quotes:
        document->replaceText(word, QLatin1Char('\"') + base + QLatin1Char('\"'));
        break;

    case QmlJS::CompletionItem::QuotesAndBracket:
        document->replaceText(word, QLatin1Char('\"') + base + QLatin1String("\"]"));
        break;

    case QmlJS::CompletionItem::ColonOrBracket:
        if (declaration() && declaration()->abstractType() &&
            declaration()->abstractType()->whichType() == AbstractType::TypeStructure) {
            document->replaceText(word, base + QLatin1String(" {}"));
        } else {
            document->replaceText(word, base + QLatin1String(": "));
        }
        break;

    case QmlJS::CompletionItem::Brackets:
        document->replaceText(word, base + QLatin1String("()"));
    }
}
