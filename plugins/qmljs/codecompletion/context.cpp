/*
    SPDX-FileCopyrightText: 2013 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include <qmljs/qmljsdocument.h>
#include <qmljs/parser/qmljslexer_p.h>
#include "../duchain/expressionvisitor.h"
#include "../duchain/helper.h"
#include "../duchain/cache.h"
#include "../duchain/frameworks/nodejs.h"

#include <QDir>
#include <QRegExp>

using namespace KDevelop;

using DeclarationDepthPair = QPair<Declaration*, int>;

namespace QmlJS {

CodeCompletionContext::CodeCompletionContext(const DUContextPointer& context, const QString& text,
                                             const CursorInRevision& position, int depth)
: KDevelop::CodeCompletionContext(context, extractLastLine(text), position, depth),
  m_completionKind(NormalCompletion)
{
    // Detect "import ..." and provide import completions
    if (m_text.startsWith(QLatin1String("import "))) {
        m_completionKind = ImportCompletion;
    }

    // Node.js module completions
    if (m_text.endsWith(QLatin1String("require("))) {
        m_completionKind = NodeModulesCompletion;
    }

    // Detect whether the cursor is in a comment
    bool isLastLine = true;
    bool inString = false;

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
        } else if (prev != QLatin1Char('\\') && (c == QLatin1Char('"') || c == QLatin1Char('\''))) {
            // Toggle whether we are in a string or not
            inString = !inString;
        }
    }

    if (inString) {
        m_completionKind = StringCompletion;
    }

    // Some specific constructs don't need any code-completion at all (mainly
    // because the user will declare new things, not use ones)
    if (m_text.contains(QRegExp(QLatin1String("(var|function)\\s+$"))) ||                   // "var |" or "function |"
        m_text.contains(QRegExp(QLatin1String("property\\s+[a-zA-Z0-9_]+\\s+$"))) ||        // "property <type> |"
        m_text.contains(QRegExp(QLatin1String("function(\\s+[a-zA-Z0-9_]+)?\\s*\\($"))) ||  // "function (|" or "function <name> (|"
        m_text.contains(QRegExp(QLatin1String("id:\\s*")))                                  // "id: |"
    ) {
        m_completionKind = NoCompletion;
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
    case NodeModulesCompletion:
        return nodeModuleCompletions();
    case StringCompletion:
    case NoCompletion:
        break;
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
    bool inQmlObjectScope = (m_duContext->type() == DUContext::Class);

    // Start with the function call-tips, because functionCallTips is also responsible
    // for setting m_declarationForTypeMatch
    items << functionCallTips();

    if (lastChar == QLatin1Char('.') || lastChar == QLatin1Char('[')) {
        // Offer completions for object members and array subscripts
        items << fieldCompletions(
            m_text.left(m_text.size() - 1),
            lastChar == QLatin1Char('[') ? CompletionItem::QuotesAndBracket :
            CompletionItem::NoDecoration
        );
    }

    // "object." must only display the members of object, the declarations
    // available in the current context.
    if (lastChar != QLatin1Char('.')) {
        if (inQmlObjectScope) {
            DUChainReadLocker lock;

            // The cursor is in a QML object and there is nothing before it. Display
            // a list of properties and signals that can be used in a script binding.
            // Note that the properties/signals of parent QML objects are not displayed here
            items << completionsInContext(m_duContext,
                                          CompletionOnlyLocal | CompletionHideWrappers,
                                          CompletionItem::ColonOrBracket);
            items << completionsFromImports(CompletionHideWrappers);
            items << completionsInContext(DUContextPointer(m_duContext->topContext()),
                                          CompletionHideWrappers,
                                          CompletionItem::NoDecoration);
        } else {
            items << completionsInContext(m_duContext,
                                          CompletionInContextFlags(),
                                          CompletionItem::NoDecoration);
            items << completionsFromImports(CompletionInContextFlags());
            items << completionsFromNodeModule(CompletionInContextFlags(), QStringLiteral("__builtin_ecmascript"));

            if (!QmlJS::isQmlFile(m_duContext.data())) {
                items << completionsFromNodeModule(CompletionInContextFlags(), QStringLiteral("__builtin_dom"));
            }
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
    const auto fragment = m_text.section(QLatin1Char(' '), -1, -1);

    auto addModules = [&items, &fragment](const QString& dataDir) {
        if (dataDir.isEmpty())
            return;

        QDir dir(dataDir);
        if (!dir.exists())
            return;

        const auto dirEntries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        items.reserve(dirEntries.size());
        for (const QString& entry : dirEntries) {
            items.append(CompletionTreeItemPointer(new ModuleCompletionItem(
                fragment + entry.section(QLatin1Char('.'), 0, 0),
                ModuleCompletionItem::Import
            )));
        }
    };

    // Use the cache to find the directory corresponding to the fragment
    // (org.kde is, for instance, /usr/lib64/kde4/imports/org/kde), and list
    // its subdirectories
    addModules(Cache::instance().modulePath(m_duContext->url(), fragment));

    if (items.isEmpty()) {
        // fallback to list all directories we can find
        const auto paths = Cache::instance().libraryPaths(m_duContext->url());
        auto fragmentPath = fragment;
        fragmentPath.replace(QLatin1Char('.'), QLatin1Char('/'));
        for (const auto& path : paths) {
            addModules(path.cd(fragmentPath).path());
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::nodeModuleCompletions()
{
    QList<CompletionTreeItemPointer> items;
    QDir dir;

    const auto& paths = NodeJS::instance().moduleDirectories(m_duContext->url().str());
    for (auto& path : paths) {
        dir.setPath(path.toLocalFile());

        const auto& entries = dir.entryList(QDir::Files, QDir::Name);
        for (QString entry : entries) {
            entry.remove(QLatin1String(".js"));

            if (entry.startsWith(QLatin1String("__"))) {
                // Internal module, don't show
                continue;
            }

            items.append(CompletionTreeItemPointer(
                new ModuleCompletionItem(entry, ModuleCompletionItem::Quotes)
            ));
        }
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::functionCallTips()
{
    Stack<ExpressionStackEntry> stack = expressionStack(m_text);
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
                auto  item = new FunctionCalltipCompletionItem(
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
    const QList<Declaration*> imports = m_duContext->findDeclarations(globalImportIdentifier());
    QList<Declaration*> realImports;

    for (Declaration* import : imports) {
        if (import->kind() != Declaration::NamespaceAlias) {
            continue;
        }

        auto* decl = static_cast<NamespaceAliasDeclaration *>(import);
        realImports << m_duContext->findDeclarations(decl->importIdentifier());
    }

    items.reserve(realImports.size());
    foreach (Declaration* import, realImports) {
        items << completionsInContext(
            DUContextPointer(import->internalContext()),
            flags,
            CompletionItem::NoDecoration
        );
    }

    return items;
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionsFromNodeModule(CompletionInContextFlags flags,
                                                                                  const QString& module)
{
    return completionsInContext(
        DUContextPointer(QmlJS::getInternalContext(
            QmlJS::NodeJS::instance().moduleExports(module, m_duContext->url())
        )),
        flags | CompletionOnlyLocal,
        CompletionItem::NoDecoration
    );
}

QList<CompletionTreeItemPointer> CodeCompletionContext::completionsInContext(const DUContextPointer& context,
                                                                             CompletionInContextFlags flags,
                                                                             CompletionItem::Decoration decoration)
{
    QList<CompletionTreeItemPointer> items;
    DUChainReadLocker lock;

    if (context) {
        const auto declarations = context->allDeclarations(
            CursorInRevision::invalid(),
            context->topContext(),
            !flags.testFlag(CompletionOnlyLocal)
        );

        for (const DeclarationDepthPair& decl : declarations) {
            DeclarationPointer declaration(decl.first);
            CompletionItem::Decoration decorationOfThisItem = decoration;

            if (declaration->identifier() == globalImportIdentifier()) {
                continue;
            } if (declaration->qualifiedIdentifier().isEmpty()) {
                continue;
            } else if (context->owner() && (
                            context->owner()->kind() == Declaration::Namespace ||
                            context->owner()->kind() == Declaration::NamespaceAlias
                       ) && decl.second != 0 && decl.second != 1001) {
                // Only show the local declarations of modules, or the declarations
                // immediately in its imported parent contexts (that are global
                // contexts, hence the distance of 1001). This prevents "String()",
                // "QtQuick1.0" and "builtins" from being listed when the user
                // types "PlasmaCore.".
                continue;
            } else if (decorationOfThisItem == CompletionItem::NoDecoration &&
                       declaration->abstractType() &&
                       declaration->abstractType()->whichType() == AbstractType::TypeFunction) {
                // Decorate function calls with brackets
                decorationOfThisItem = CompletionItem::Brackets;
            } else if (flags.testFlag(CompletionHideWrappers)) {
                auto* classDecl = dynamic_cast<ClassDeclaration*>(declaration.data());

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

Stack<CodeCompletionContext::ExpressionStackEntry> CodeCompletionContext::expressionStack(const QString& expression)
{
    Stack<CodeCompletionContext::ExpressionStackEntry> stack;
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
            entry.startPosition = lexer.tokenStartColumn() + lexer.tokenLength() - 1;
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
            break;
        default:
            // The last operator of every sub-expression is stored on the stack
            // so that "A = foo." can know that attributes of foo having the same
            // type as A should be highlighted.
            stack.top().operatorStart = lexer.tokenStartColumn() - 1;
            stack.top().operatorEnd = lexer.tokenStartColumn() + lexer.tokenLength() - 1;
            break;
        }
    }

    return stack;
}

DeclarationPointer CodeCompletionContext::declarationAtEndOfString(const QString& expression)
{
    // Build the expression stack of expression and use the valid portion of the
    // top sub-expression to find the right-most declaration that can be found
    // in expression.
    QmlJS::Document::MutablePtr doc = QmlJS::Document::create(QStringLiteral("inline"), Dialect::JavaScript);
    ExpressionStackEntry topEntry = expressionStack(expression).top();

    doc->setSource(expression.mid(topEntry.operatorEnd + topEntry.commas));
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
    return std::all_of(str.begin(), str.end(), [](const QChar c) {
        return c.isSpace();
    });
}

}
