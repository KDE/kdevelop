/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2013 Sven Brauch <svenbrauch@googlemail.com>
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

#include "context.h"
#include "completionitem.h"

#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/codemodel.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljslexer_p.h>
#include <duchain/expressionvisitor.h>
#include <duchain/helper.h>

using namespace KDevelop;

typedef QPair<Declaration*, int> DeclarationDepthPair;

namespace QmlJS {

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const CursorInRevision& position, int depth)
    : KDevelop::CodeCompletionContext(context, extractLastLine(text), position, depth)
{
    // Determine which kind of completion should be offered
    // ...
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    Q_UNUSED (fullCompletion);

    // Compute the completion items, based on the surrounding text and the
    // type of completion to offer in the constructor

    QList<CompletionTreeItemPointer> items;

    if (abort || m_text.isEmpty()) {
        return items;
    }

    QChar lastChar = m_text.at(m_text.size() - 1);

    if (lastChar == QLatin1Char('.') || lastChar == QLatin1Char('[')) {
        // Offer completions for object members and array subscripts
        items << fieldCompletions(
            m_text.left(m_text.size() - 1),
            lastChar == QLatin1Char('[') ? CompletionItem::QuotesAndBracket : CompletionItem::NoDecoration
        );
    }

    // "object." must only display the members of object, the declarations
    // available in the current context.
    if (lastChar != QLatin1Char('.')) {
        bool inQmlObjectScope = (m_duContext->type() == DUContext::Class && containsOnlySpaces(m_text));

        if (inQmlObjectScope) {
            DUChainReadLocker lock;

            // The cursor is in a QML object and there is nothing before it. Display
            // a list of properties and signals that can be used in a script binding.
            // Note that the properties/signals of parent QML objects are not displayed here
            items << completionsInContext(m_duContext,
                                          CompletionOnlyLocal | CompletionHideWrappers,
                                          CompletionItem::Colon);
            items << completionsInContext(DUContextPointer(m_duContext->topContext()),
                                          CompletionHideWrappers,
                                          CompletionItem::NoDecoration);
        } else {
            items << completionsInContext(m_duContext,
                                          0,
                                          CompletionItem::NoDecoration);
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionsInContext(const DUContextPointer& context,
                                                                             CompletionInContextFlags flags,
                                                                             CompletionItem::Decoration decoration)
{
    QList<CompletionTreeItemPointer> items;
    DUChainReadLocker lock;

    if (context) {
        const QList<DeclarationDepthPair>& declarations = context->allDeclarations(
            CursorInRevision::invalid(),
            context->topContext(),
            !flags.testFlag(CompletionOnlyLocal)
        );

        foreach (const DeclarationDepthPair& decl, declarations) {
            DeclarationPointer declaration(decl.first);

            if (declaration->kind() == Declaration::NamespaceAlias) {
                continue;
            } else if (declaration->qualifiedIdentifier().isEmpty()) {
                continue;
            } else if (flags.testFlag(CompletionHideWrappers)) {
                ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(declaration.data());

                if (classDecl && classDecl->classType() == ClassDeclarationData::Interface) {
                    continue;
                }
            }

            items << CompletionTreeItemPointer(new CompletionItem(declaration, decl.second, decoration));
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::fieldCompletions(const QString& expression,
                                                                         CompletionItem::Decoration decoration)
{
    // expression is an incomplete expression. Try to parse as much as possible
    // of it, in order to get the most complete AST possible.
    // For instance, if expression is "test(foo.bar", test(foo.bar is invalid,
    // but foo.bar is valid and should be used (bar is also valid, but too small
    // to provide any useful context)
    QStack<int> bracketPositions;
    QmlJS::Lexer lexer(nullptr);
    bool atEnd = false;

    lexer.setCode(expression, 1, false);
    bracketPositions.push(0);

    while (!atEnd) {
        switch (lexer.lex()) {
        case QmlJSGrammar::EOF_SYMBOL:
            atEnd = true;
            break;
        case QmlJSGrammar::T_LBRACE:
        case QmlJSGrammar::T_LBRACKET:
        case QmlJSGrammar::T_LPAREN:
            bracketPositions.push(lexer.tokenStartColumn());
            break;
        case QmlJSGrammar::T_RBRACE:
        case QmlJSGrammar::T_RBRACKET:
        case QmlJSGrammar::T_RPAREN:
            bracketPositions.pop();
            break;
        case QmlJSGrammar::T_IDENTIFIER:
        case QmlJSGrammar::T_DOT:
            break;
        default:
            // "a == b", "a + b", "a && b", "typeof b", etc are not expressions
            // whose type is interesting for code-completion, but "b" is.
            // "foo(a, b", is not a valid expression, "b" is. "var a = b" is not
            // understood by ExpressionVisitor, b is.
            // Shift the current position so that everything before the unwanted char is
            // ignored. Because a stack is used, "foo(a, b)" will still correctly
            // be identified a an expression if the closing brace in present
            bracketPositions.top() = lexer.tokenStartColumn();
        }
    }

    // The last un-matched paren/brace/bracket correspond to the start of something
    // that should be a valid expression.
    QmlJS::Document::MutablePtr doc = QmlJS::Document::create("inline", Language::JavaScript);

    doc->setSource(expression.mid(bracketPositions.top()));
    doc->parseExpression();

    if (!doc || !doc->isParsedCorrectly()) {
        return QList<CompletionTreeItemPointer>();
    }

    // Use ExpressionVisitor to find the type (and associated declaration) of
    // the snippet that has been parsed. The inner context of the declaration
    // can be used to get the list of completions
    ExpressionVisitor visitor(m_duContext.data());
    doc->ast()->accept(&visitor);

    DUContext* context = getInternalContext(visitor.lastDeclaration());

    if (context) {
        return completionsInContext(DUContextPointer(context),
                                    CompletionOnlyLocal,
                                    decoration);
    } else {
        return QList<CompletionTreeItemPointer>();
    }
}

bool CodeCompletionContext::containsOnlySpaces(const QString& str)
{
    for (int i=0; i<str.size(); ++i) {
        if (!str.at(i).isSpace()) {
            return false;
        }
    }

    return true;
}

}
