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

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/structuretype.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

using namespace QmlJS;
using namespace KDevelop;

CompletionItem::CompletionItem(DeclarationPointer decl, int inheritanceDepth, Decoration decoration)
: NormalDeclarationCompletionItem(decl, KSharedPtr<CodeCompletionContext>(), inheritanceDepth),
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

    if (role == Qt::DisplayRole && index.column() == CodeCompletionModel::Prefix) {
        if (classDecl) {
            if (classDecl->classType() == ClassDeclarationData::Class) {
                // QML component
                return QString("component");
            } else if (classDecl->classType() == ClassDeclarationData::Interface) {
                // C++-ish QML component
                return QString("wrapper");
            }
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

    if (classFuncDecl && classFuncDecl->isSignal() && m_decoration == QmlJS::CompletionItem::Colon) {
        // Signals, when completed in a QML component context, are transformed into slots
        QString signal = classFuncDecl->identifier().toString();

        if (signal.size() > 0) {
            return QLatin1String("on") + signal.at(0).toUpper() + signal.mid(1);
        }
    }

    return NormalDeclarationCompletionItem::declarationName();
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

    case QmlJS::CompletionItem::Colon:
        document->replaceText(word, base + ": ");
        break;

    case QmlJS::CompletionItem::Brackets:
        document->replaceText(word, base + "()");

        if (document->activeView()) {
            document->activeView()->setCursorPosition(word.end() + KTextEditor::Cursor(0, 1));
        }
    }
}
