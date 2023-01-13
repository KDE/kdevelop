/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "codecompletionhelper.h"

#include "../duchain/duchain.h"
#include "../duchain/declaration.h"
#include "../duchain/duchainlock.h"
#include "../duchain/types/functiontype.h"
#include "../duchain/types/integraltype.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

namespace KDevelop {
void insertFunctionParenText(KTextEditor::View* view, const KTextEditor::Cursor& pos,
                             const DeclarationPointer& declaration, bool jumpForbidden)
{
    bool spaceBeforeParen = false; ///@todo Take this from some astyle config or something
    bool spaceBetweenParens = false;
    bool spaceBetweenEmptyParens = false;

    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());

    bool haveArguments = false;

    if (!declaration)
        return;

    TypePtr<FunctionType> funcType = declaration->type<FunctionType>();

    if (declaration->kind() == Declaration::Type || (funcType && funcType->indexedArgumentsSize()))
        haveArguments = true;

    if (declaration->kind() == Declaration::Instance && !declaration->isFunctionDeclaration())
        haveArguments = true; //probably a constructor initializer

    //Need to have a paren behind
    QString suffix = view->document()->text(KTextEditor::Range(pos, pos + KTextEditor::Cursor(1, 0)));
    if (suffix.trimmed().startsWith(QLatin1Char('('))) {
        //Move the cursor behind the opening paren
        if (view)
            view->setCursorPosition(pos + KTextEditor::Cursor(0, suffix.indexOf(QLatin1Char('(')) + 1));
    } else {
        //We need to insert an opening paren
        QString openingParen;
        if (spaceBeforeParen)
            openingParen = QStringLiteral(" (");
        else
            openingParen = QStringLiteral("(");

        if (spaceBetweenParens && (haveArguments || spaceBetweenEmptyParens))
            openingParen += QLatin1Char(' ');

        QString closingParen;
        if (spaceBetweenParens && (haveArguments)) {
            closingParen = QStringLiteral(" )");
        } else
            closingParen = QStringLiteral(")");

        KTextEditor::Cursor jumpPos = pos + KTextEditor::Cursor(0, openingParen.length());

        // when function returns void, also add a semicolon
        if (funcType) {
            if (auto type = funcType->returnType().dynamicCast<IntegralType>()) {
                if (type->dataType() == IntegralType::TypeVoid) {
                    const QChar nextChar = view->document()->characterAt(pos);
                    if (nextChar != QLatin1Char(';') && nextChar != QLatin1Char(')') && nextChar != QLatin1Char(',')) {
                        closingParen += QLatin1Char(';');
                    }
                }
            }
        }

        //If no arguments, move the cursor behind the closing paren (or semicolon)
        if (!haveArguments)
            jumpPos += KTextEditor::Cursor(0, closingParen.length());

        lock.unlock();
        if (view) {
            view->document()->insertText(pos, openingParen + closingParen);
            if (!jumpForbidden) {
                view->setCursorPosition(jumpPos);
            }
        }
    }
}
}
