/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <duchain/functiontype.h>

using namespace QmlJS;
using namespace KDevelop;

CompletionItem::CompletionItem(DeclarationPointer decl, int inheritanceDepth, Decoration decoration)
: NormalDeclarationCompletionItem(decl, KSharedPtr<KDevelop::CodeCompletionContext>(), inheritanceDepth),
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

    ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration *>(decl);
    StructureType::Ptr declType = StructureType::Ptr::dynamicCast(decl->abstractType());
    auto funcType = QmlJS::FunctionType::Ptr::dynamicCast(decl->abstractType());

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

        QmlJS::FunctionType::Ptr declFunc = QmlJS::FunctionType::Ptr::dynamicCast(declType);

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
        FunctionDeclaration* funcDecl =
            dynamic_cast<FunctionDeclaration*>(funcType->declaration(decl->topContext()));

        if (funcDecl) {
            switch (index.column()) {
            case CodeCompletionModel::Prefix:
                return funcType->returnType()->toString();
            case CodeCompletionModel::Name:
                // Return the identifier of the declaration being listed, not of its
                // function declaration (because the function may have been declared
                // anonymously, even if it has been assigned to a variable)
                return decl->qualifiedIdentifier().toString();
            case CodeCompletionModel::Arguments:
            {
                QStringList args;

                for (auto arg : funcDecl->internalFunctionContext()->localDeclarations()) {
                    args.append(arg->toString());
                }

                return QLatin1Char('(') + args.join(QLatin1String(", ")) + QLatin1Char(')');
            }
            }
        }
    } else if (role == Qt::DisplayRole && index.column() == CodeCompletionModel::Prefix) {
        if (classDecl) {
            if (classDecl->classType() == ClassDeclarationData::Class) {
                // QML component
                return QString("component");
            } else if (classDecl->classType() == ClassDeclarationData::Interface) {
                // C++-ish QML component
                return QString("wrapper");
            }
        }

        if (decl && (
                decl->kind() == Declaration::NamespaceAlias ||
                decl->kind() == Declaration::Namespace
           )) {
            // Display namespaces and namespace aliases as modules
            return QString("module");
        }

        if (decl && decl->abstractType() &&
            decl->kind() == Declaration::Type &&
            decl->abstractType()->whichType() == AbstractType::TypeEnumeration) {
            // Enum
            return QString("enum");
        }

        if (declType &&
            decl->kind() == Declaration::Instance &&
            declType->declarationId().qualifiedIdentifier().isEmpty()) {
            // QML component instance. The type that should be displayed is the
            // base class of its anonymous class
            ClassDeclaration* anonymousClass = dynamic_cast<ClassDeclaration *>(declType->declaration(decl->topContext()));

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
            return QLatin1String("on") + signal.at(0).toUpper() + signal.mid(1);
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

    return NormalDeclarationCompletionItem::completionProperties();
}

void CompletionItem::executed(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    QString base = document->text(word);

    switch (m_decoration)
    {
    case QmlJS::CompletionItem::NoDecoration:
        break;

    case QmlJS::CompletionItem::Quotes:
        document->replaceText(word, "\"" + base + "\"");
        break;

    case QmlJS::CompletionItem::QuotesAndBracket:
        document->replaceText(word, "\"" + base + "\"]");
        break;

    case QmlJS::CompletionItem::ColonOrBracket:
        if (declaration() && declaration()->abstractType() &&
            declaration()->abstractType()->whichType() == AbstractType::TypeStructure) {
            document->replaceText(word, base + " {}");

            if (document->activeView()) {
                document->activeView()->setCursorPosition(word.end() + KTextEditor::Cursor(0, 2));
            }
        } else {
            document->replaceText(word, base + ": ");
        }
        break;

    case QmlJS::CompletionItem::Brackets:
        document->replaceText(word, base + "()");

        if (document->activeView()) {
            document->activeView()->setCursorPosition(word.end() + KTextEditor::Cursor(0, 1));
        }
    }
}
