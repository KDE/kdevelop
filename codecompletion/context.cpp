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

#include "items/modulecompletionitem.h"
#include "items/functioncalltipcompletionitem.h"

#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/codemodel.h>
#include <kstandarddirs.h>

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljslexer_p.h>
#include <duchain/expressionvisitor.h>
#include <duchain/helper.h>

#include <QtCore/QDir>

using namespace KDevelop;

typedef QPair<Declaration*, int> DeclarationDepthPair;

namespace QmlJS {

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const CursorInRevision& position, int depth)
: KDevelop::CodeCompletionContext(context, extractLastLine(text), position, depth),
  m_completionKind(NormalCompletion)
{
    // Detect "import ..." and provide import completions
    if (m_text.startsWith("import ")) {
        m_completionKind = ImportCompletion;
    }

    // Detect whether the cursor is in a comment
    bool isLastLine = true;

    for (int index = text.size()-1; index > 0; --index) {
        const QChar c = text.at(index);
        const QChar prev = text.at(index - 1);

        if (c == QLatin1Char('\n')) {
            isLastLine = false;
        } else if (isLastLine && prev == QLatin1Char('/') && c == QLatin1Char('/')) {
            // Single-line comment on the current line, we are in a comment
            m_completionKind = CommentCompletion;
            break;
        } else if (prev == QLatin1Char('/') && c == QLatin1Char('*')) {
            // Start of a multi-line comment encountered
            m_completionKind = CommentCompletion;
            break;
        } else if (prev == QLatin1Char('*') && c == QLatin1Char('/')) {
            // End of a multi-line comment. Because /* and */ cannot be nested,
            // encountering a */ is enough to know that the cursor is outside a
            // comment
            break;
        }
    }
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionItems(bool& abort, bool fullCompletion)
{
    Q_UNUSED (fullCompletion);

    if (abort) {
        return QList<CompletionTreeItemPointer>();
    }

    switch (m_completionKind) {
    case NormalCompletion:
        return normalCompletion();
    case CommentCompletion:
        return commentCompletion();
    case ImportCompletion:
        return importCompletion();
    }

    return QList<CompletionTreeItemPointer>();
}

AbstractType::Ptr CodeCompletionContext::typeToMatch() const
{
    return m_typeToMatch;
}

QList<KDevelop::CompletionTreeItemPointer> CodeCompletionContext::normalCompletion()
{
    QList<CompletionTreeItemPointer> items;
    QChar lastChar = m_text.size() > 0 ? m_text.at(m_text.size() - 1) : QLatin1Char('\0');

    // Start with the function call-tips, because functionCallTips is also responsible
    // for setting m_declarationForTypeMatch
    items << functionCallTips();

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
            items << completionsFromImports(CompletionHideWrappers);
            items << completionsInContext(DUContextPointer(m_duContext->topContext()),
                                          CompletionHideWrappers,
                                          CompletionItem::NoDecoration);
        } else {
            items << completionsInContext(m_duContext,
                                          0,
                                          CompletionItem::NoDecoration);
            items << completionsFromImports(0);
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::commentCompletion()
{
    return QList<CompletionTreeItemPointer>();
}

QList<CompletionTreeItemPointer> CodeCompletionContext::importCompletion()
{
    QList<CompletionTreeItemPointer> items;

    // List $KDEDATA/kdevqmljssupport/qmlplugins/ and add one completion item
    // per file found there
    QString dataDir = KGlobal::dirs()->findDirs("data",
        QLatin1String("kdevqmljssupport/qmlplugins")
    ).at(0);
    QDir dir(dataDir);

    for (const QString &entry : dir.entryList(QDir::Files, QDir::Name)) {
        items.append(CompletionTreeItemPointer(new ModuleCompletionItem(entry)));
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::functionCallTips()
{
    QStack<ExpressionStackEntry> stack = expressionStack(m_text);
    QList<CompletionTreeItemPointer> items;
    int argumentHintDepth = 1;
    bool isTopOfStack = true;
    DUChainReadLocker lock;

    while (!stack.isEmpty()) {
        ExpressionStackEntry entry = stack.pop();

        if (isTopOfStack && entry.operatorStart > entry.startPosition) {
            // Deduce the declaration for type matching using operatorStart:
            //
            // table[document.base +
            //       [             ^
            //
            // ^ = operatorStart. Just before operatorStart is a code snippet that ends
            // with the declaration whose type should be used.
            DeclarationPointer decl = declarationAtEndOfString(m_text.left(entry.operatorStart));

            if (decl) {
                m_typeToMatch = decl->abstractType();
            }
        }

        if (entry.startPosition > 0 && m_text.at(entry.startPosition - 1) == QLatin1Char('(')) {
            // The current entry represents a function call, create a call-tip for it
            DeclarationPointer functionDecl = declarationAtEndOfString(m_text.left(entry.startPosition - 1));

            if (functionDecl) {
                FunctionCalltipCompletionItem* item = new FunctionCalltipCompletionItem(
                    functionDecl,
                    argumentHintDepth,
                    entry.commas
                );

                items << CompletionTreeItemPointer(item);
                argumentHintDepth++;

                if (isTopOfStack && !m_typeToMatch) {
                    m_typeToMatch = item->currentArgumentType();
                }
            }
        }

        isTopOfStack = false;
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionsFromImports(CompletionInContextFlags flags)
{
    QList<CompletionTreeItemPointer> items;

    // Iterate over all the imported namespaces and add their definitions
    DUChainReadLocker lock;
    QList<Declaration*> imports = m_duContext->findDeclarations(globalImportIdentifier());
    QList<Declaration*> realImports;

    foreach (Declaration* import, imports) {
        if (import->kind() != Declaration::NamespaceAlias) {
            continue;
        }

        NamespaceAliasDeclaration* decl = static_cast<NamespaceAliasDeclaration *>(import);
        realImports << m_duContext->findDeclarations(decl->importIdentifier());
    }

    foreach (Declaration* import, realImports) {
        items << completionsInContext(
            DUContextPointer(import->internalContext()),
            flags,
            CompletionItem::NoDecoration
        );
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
            CompletionItem::Decoration decorationOfThisItem = decoration;

            if (declaration->identifier() == globalImportIdentifier()) {
                continue;
            } if (declaration->qualifiedIdentifier().isEmpty()) {
                continue;
            } else if (decorationOfThisItem == CompletionItem::NoDecoration &&
                       declaration->abstractType() &&
                       declaration->abstractType()->whichType() == AbstractType::TypeFunction) {
                // Decorate function calls with brackets
                decorationOfThisItem = CompletionItem::Brackets;
            } else if (flags.testFlag(CompletionHideWrappers)) {
                ClassDeclaration* classDecl = dynamic_cast<ClassDeclaration*>(declaration.data());

                if (classDecl && classDecl->classType() == ClassDeclarationData::Interface) {
                    continue;
                }
            }

            items << CompletionTreeItemPointer(new CompletionItem(declaration, decl.second, decorationOfThisItem));
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::fieldCompletions(const QString& expression,
                                                                         CompletionItem::Decoration decoration)
{
    // The statement given to this method ends with an expression that may identify
    // a declaration ("foo" in "test(1, 2, foo"). List the declarations of this
    // inner context
    DUContext* context = getInternalContext(declarationAtEndOfString(expression));

    if (context) {
        return completionsInContext(DUContextPointer(context),
                                    CompletionOnlyLocal,
                                    decoration);
    } else {
        return QList<CompletionTreeItemPointer>();
    }
}

QStack<CodeCompletionContext::ExpressionStackEntry> CodeCompletionContext::expressionStack(const QString& expression)
{
    QStack<CodeCompletionContext::ExpressionStackEntry> stack;
    ExpressionStackEntry entry;
    QmlJS::Lexer lexer(nullptr);
    bool atEnd = false;

    lexer.setCode(expression, 1, false);

    entry.startPosition = 0;
    entry.operatorStart = 0;
    entry.operatorEnd = 0;
    entry.commas = 0;

    stack.push(entry);

    // NOTE: KDevelop uses 0-indexed columns while QMLJS uses 1-indexed columns
    while (!atEnd) {
        switch (lexer.lex()) {
        case QmlJSGrammar::EOF_SYMBOL:
            atEnd = true;
            break;
        case QmlJSGrammar::T_LBRACE:
        case QmlJSGrammar::T_LBRACKET:
        case QmlJSGrammar::T_LPAREN:
            entry.startPosition = lexer.tokenEndColumn() - 1;
            entry.operatorStart = entry.startPosition;
            entry.operatorEnd = entry.startPosition;
            entry.commas = 0;

            stack.push(entry);
            break;
        case QmlJSGrammar::T_RBRACE:
        case QmlJSGrammar::T_RBRACKET:
        case QmlJSGrammar::T_RPAREN:
            if (stack.count() > 1) {
                stack.pop();
            }
            break;
        case QmlJSGrammar::T_IDENTIFIER:
        case QmlJSGrammar::T_DOT:
        case QmlJSGrammar::T_THIS:
            break;
        case QmlJSGrammar::T_COMMA:
            stack.top().commas++;
        default:
            // The last operator of every sub-expression is stored on the stack
            // so that "A = foo." can know that attributes of foo having the same
            // type as A should be highlighted.
            stack.top().operatorStart = lexer.tokenStartColumn() - 1;
            stack.top().operatorEnd = lexer.tokenEndColumn() - 1;
        }
    }

    return stack;
}

DeclarationPointer CodeCompletionContext::declarationAtEndOfString(const QString& expression)
{
    // Build the expression stack of expression and use the valid portion of the
    // top sub-expression to find the right-most declaration that can be found
    // in expression.
    QmlJS::Document::MutablePtr doc = QmlJS::Document::create("inline", Language::JavaScript);
    ExpressionStackEntry topEntry = expressionStack(expression).top();

    doc->setSource(expression.mid(topEntry.operatorEnd));
    doc->parseExpression();

    if (!doc || !doc->isParsedCorrectly()) {
        return DeclarationPointer();
    }

    // Use ExpressionVisitor to find the type (and associated declaration) of
    // the snippet that has been parsed. The inner context of the declaration
    // can be used to get the list of completions
    ExpressionVisitor visitor(m_duContext.data());
    doc->ast()->accept(&visitor);

    return visitor.lastDeclaration();
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
