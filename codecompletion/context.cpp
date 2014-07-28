/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
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
 */

#include "context.h"

#include "navigationwidget.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include <language/interfaces/iastcontainer.h>
#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>

#include "../duchain/parsesession.h"
#include "../duchain/clangtypes.h"
#include "../duchain/cursorkindtraits.h"

#include "../debug.h"

#include <memory>
#include <KTextEditor/Document>
#include <KTextEditor/View>

using namespace KDevelop;

namespace {
///Completion results with priority below this value will be shown in "Best Matches" group
const int MAX_PRIORITY_FOR_BEST_MATCHES = 15;

/**
 * Common base class for Clang code completion items.
 */
template<class Base>
class CompletionItem : public Base
{
public:
    CompletionItem(const QString& display, const QString& prefix, const QString& replacement)
        : Base()
        , m_display(display)
        , m_prefix(prefix)
        , m_replacement(replacement)
    {
    }

    virtual ~CompletionItem() = default;

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* /*model*/) const override
    {
        if (role == Qt::DisplayRole) {
            if (index.column() == CodeCompletionModel::Prefix) {
                return m_prefix;
            } else if (index.column() == CodeCompletionModel::Name) {
                return m_display;
            }
        }
        return {};
    }

    void execute(KTextEditor::Document* document, const KTextEditor::Range& word) override
    {
        document->replaceText(word, m_replacement);
    }

protected:
    QString m_display;
    QString m_prefix;
    QString m_replacement;
};

class OverrideItem : public CompletionItem<CompletionTreeItem>
{
public:
    OverrideItem(QString& nameAndParams, QString& returnType)
        : CompletionItem<KDevelop::CompletionTreeItem>(
              nameAndParams,
              i18n("Override %1", returnType),
              "virtual " + returnType + ' ' + nameAndParams
          )
    {
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == Qt::DecorationRole) {
            if (index.column() == KTextEditor::CodeCompletionModel::Icon) {
                static QIcon icon(KIcon("CTparents"));
                return icon;
            }
        }
        return CompletionItem<CompletionTreeItem>::data(index, role, model);
    }
};

class ImplementsItem : public CompletionItem<CompletionTreeItem>
{
public:
    ImplementsItem(FuncImplementInfo& item)
        : CompletionItem<KDevelop::CompletionTreeItem>(
              item.prototype,
              i18n("Implement %1", item.isConstructor ? "<constructor>" :
                                   item.isDestructor ? "<destructor>" : item.returnType),
              item.templatePrefix + (!item.isDestructor && !item.isConstructor ? item.returnType + ' ' : "") + item.prototype + "\n{\n}\n"
          )
    {
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == Qt::DecorationRole) {
            if (index.column() == KTextEditor::CodeCompletionModel::Icon) {
                static QIcon icon(KIcon("CTsuppliers"));
                return icon;
            }
        }
        return CompletionItem<CompletionTreeItem>::data(index, role, model);
    }
};

/**
 * Specialized completion item class for items which are represented by a Declaration
 */
class DeclarationItem : public CompletionItem<NormalDeclarationCompletionItem>
{
public:
    DeclarationItem(Declaration* dec, const QString& display, const QString& prefix, const QString& replacement)
        : CompletionItem<NormalDeclarationCompletionItem>(display, prefix, replacement)
    {
        m_declaration = dec;
    }

    QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const override
    {
        if (role == CodeCompletionModel::MatchQuality && m_bestMatchQuality) {
            return m_bestMatchQuality;
        }

        auto ret = CompletionItem<NormalDeclarationCompletionItem>::data(index, role, model);
        if (ret.isValid()) {
            return ret;
        }
        return NormalDeclarationCompletionItem::data(index, role, model);
    }

    void execute(KTextEditor::Document* document, const KTextEditor::Range& word) override
    {
        QString repl = m_replacement;
        DUChainReadLocker lock;

        if(!m_declaration){
            return;
        }

        if(m_declaration->isFunctionDeclaration()) {
            repl += "()";
            document->replaceText(word, repl);
            auto f = m_declaration->type<FunctionType>();
            if (f && f->indexedArgumentsSize()) {
                document->activeView()->setCursorPosition(word.start() + KTextEditor::Cursor(0, repl.size() - 1));
            }
        } else {
            document->replaceText(word, repl);
        }
    }

    bool createsExpandingWidget() const override
    {
        return true;
    }

    QWidget* createExpandingWidget(const CodeCompletionModel* /*model*/) const override
    {
        return new ClangNavigationWidget(m_declaration);
    }

    ///Sets match quality from 0 to 10. 10 is the best fit.
    void setBestMatchQuality(int value)
    {
        m_bestMatchQuality = value;
    }

private:
    int m_bestMatchQuality = 0;
};

/**
 * A minimalistic completion item for macros and such
 */
using SimpleItem = CompletionItem<CompletionTreeItem>;

QByteArray concatenate(const QStringList& contents)
{
    QByteArray ret;
    int sizeGuess = 0;
    foreach(const QString& line, contents) {
        sizeGuess += line.size() + 1;
    }
    ret.reserve(sizeGuess);
    foreach(const QString& line, contents) {
        ret += line.toUtf8() + '\n';
    }
    return ret;
}

}

ClangCodeCompletionContext::ClangCodeCompletionContext(const ParseSession& session,
                                                       const SimpleCursor& position,
                                                       const QStringList& contents
                                                      )
    : CodeCompletionContext({}, QString(), {}, 0)
    , m_results(nullptr, clang_disposeCodeCompleteResults)
    , m_completionHelper(session.unit(), position, ClangString(clang_getFileName(session.file())).c_str())
{
    ClangString file(clang_getFileName(session.file()));

    CXUnsavedFile unsaved;
    const QByteArray fileContents = concatenate(contents);
    unsaved.Contents = fileContents.constData();
    unsaved.Length = fileContents.size();
    unsaved.Filename = file;

    m_results.reset( clang_codeCompleteAt(session.unit(), file,
                        position.line + 1, position.column + 1,
                        &unsaved, 1,
                        clang_defaultCodeCompleteOptions()) );
}

ClangCodeCompletionContext::~ClangCodeCompletionContext()
{
}

QList<CompletionTreeItemPointer> ClangCodeCompletionContext::completionItems(const TopDUContext* const top,
                                                                             const CursorInRevision& position)
{
    QList<CompletionTreeItemPointer> items;
    QList<CompletionTreeItemPointer> macros;
    QList<CompletionTreeItemPointer> builtin;

    QSet<Declaration*> handled;

    DUContext* ctx = top->findContextAt(position);

    for (uint i = 0; i < m_results->NumResults; ++i) {
        auto result = m_results->Results[i];
        const uint chunks = clang_getNumCompletionChunks(result.CompletionString);

        // the string that would be neede to type, usually the identifier of something
        QString typed;
        // the display string we use in the code completion items, including the function signature
        QString display;
        // the return type of a function e.g.
        QString resultType;
        // the replacement text when an item gets executed
        QString replacement;
        //BEGIN function signature parsing
        // nesting depth of parentheses
        int parenDepth = 0;
        enum FunctionSignatureState {
            // not yet inside the function signature
            Before,
            // any token is part of the function signature now
            Inside,
            // finished parsing the function signature
            After
        };
        // current state
        FunctionSignatureState signatureState = Before;
        //END function signature parsing
        for (uint j = 0; j < chunks; ++j) {
            const auto kind = clang_getCompletionChunkKind(result.CompletionString, j);
            if (kind == CXCompletionChunk_CurrentParameter || kind == CXCompletionChunk_Optional) {
                continue;
            }
            const QString string = ClangString(clang_getCompletionChunkText(result.CompletionString, j)).toString();
            switch (kind) {
                case CXCompletionChunk_Informative:
                    display += string;
                    break;
                case CXCompletionChunk_TypedText:
                    display += string;
                    typed = string;
                    replacement = string;
                    break;
                case CXCompletionChunk_ResultType:
                    resultType = string;
                    continue;
                case CXCompletionChunk_Placeholder:
                    //TODO:consider KTextEditor::TemplateInterface possibility
                    //replacement += "/*" + string + "*/";
                    if (signatureState == Inside) {
                        display += string;
                    }
                    continue;
                case CXCompletionChunk_LeftParen:
                    if (signatureState == Before && !parenDepth) {
                        signatureState = Inside;
                    }
                    parenDepth++;
                    break;
                case CXCompletionChunk_RightParen:
                    --parenDepth;
                    if (signatureState == Inside && !parenDepth) {
                        display += ')';
                        signatureState = After;
                    }
                    break;
                default:
                    break;
            }
            //replacement += string;
            if (signatureState == Inside) {
                display += string;
            }
        }

        if (resultType.size() > 23) {
            // ellide text to the right for overly long result types (templates especially)
            resultType.replace(20, resultType.size() - 20, "...");
        }

        if (result.CursorKind != CXCursor_MacroDefinition && result.CursorKind != CXCursor_NotImplemented) {
            const Identifier id(typed);
            QualifiedIdentifier qid;
            ClangString parent(clang_getCompletionParent(result.CompletionString, nullptr));
            if (parent.c_str() != nullptr) {
                qid = QualifiedIdentifier(parent.toString());
            }
            qid.push(id);

            Declaration* found = 0;
            foreach(Declaration* dec, ctx->findDeclarations(qid, position)) {
                if (!handled.contains(dec)) {
                    found = dec;
                    handled.insert(dec);
                    break;
                }
            }

            if (found) {
                auto item = new DeclarationItem(found, display, resultType, replacement);

                bool bestMatch = clang_getCompletionPriority(result.CompletionString) < MAX_PRIORITY_FOR_BEST_MATCHES;

                //don't set best match property for internal identifiers, also prefer declarations from current file
                if (bestMatch && !found->indexedIdentifier().identifier().toString().startsWith("__") ) {
                    item->setBestMatchQuality(found->context()->url() == ctx->url() ? 10 : 9 );
                }

                items << CompletionTreeItemPointer(item);
                continue;
            } else {
                debug() << "Could not find declaration for" << qid;
            }
        }

        auto item = CompletionTreeItemPointer(new SimpleItem(display, resultType, replacement));
        if (result.CursorKind == CXCursor_MacroDefinition) {
            // TODO: grouping of macros and built-in stuff
            macros.append(item);
        } else {
            builtin.append(item);
        }
    }

    addOverwritableItems();
    addImplementationHelperItems();
    eventuallyAddGroup(i18n("Macros"), 900, macros);
    eventuallyAddGroup(i18n("Builtin"), 800, builtin);
    return items;
}

void ClangCodeCompletionContext::eventuallyAddGroup(const QString& name, int priority, const QList<CompletionTreeItemPointer>& items)
{
    if (items.isEmpty()) {
        return;
    }

    KDevelop::CompletionCustomGroupNode* node = new KDevelop::CompletionCustomGroupNode(name, priority);
    node->appendChildren(items);
    m_ungrouped << CompletionTreeElementPointer(node);
}

void ClangCodeCompletionContext::addOverwritableItems()
{
    auto overrideList = m_completionHelper.overrides();
    if (overrideList.isEmpty()) {
        return;
    }

    QList<CompletionTreeItemPointer> overrides;
    for (int i = 0; i < overrideList.count(); i++) {
        FuncOverrideInfo info = overrideList.at(i);
        QString nameAndParams = info.name + '(' + info.params.join(", ") + ')';
        if(info.isConst)
            nameAndParams = nameAndParams + " const";
        if(info.isVirtual)
            nameAndParams = nameAndParams + " = 0";
        overrides << CompletionTreeItemPointer(new OverrideItem(nameAndParams, info.returnType));
    }
    eventuallyAddGroup(i18n("Virtual Override"), 0, overrides);
}

void ClangCodeCompletionContext::addImplementationHelperItems()
{
    auto implementsList = m_completionHelper.implements();
    if (implementsList.isEmpty()) {
        return;
    }

    QList<CompletionTreeItemPointer> implements;
    foreach(FuncImplementInfo info, implementsList) {
        implements << CompletionTreeItemPointer(new ImplementsItem(info));
    }
    eventuallyAddGroup(i18n("Implement Function"), 0, implements);
}


QList<CompletionTreeElementPointer> ClangCodeCompletionContext::ungroupedElements()
{
    return m_ungrouped;
}
