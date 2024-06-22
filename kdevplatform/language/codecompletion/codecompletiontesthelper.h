/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H
#define KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H

#include <QTest>
#include <QStandardItemModel>

#include "../duchain/declaration.h"
#include "../duchain/duchain.h"
#include "codecompletionitem.h"
#include <language/codegen/coderepresentation.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include <tests/testhelpers.h>

using namespace KTextEditor;

using namespace KDevelop;

/**
 * Helper-class for testing completion-items
 * Just initialize it with the context and the text, and then use the members, for simple cases only "names"
 * the template parameter is your language specific CodeCompletionContext
 */
template <class T>
struct CodeCompletionItemTester
{
    using Element = QExplicitlySharedDataPointer<KDevelop::CompletionTreeElement>;
    using Item = QExplicitlySharedDataPointer<KDevelop::CompletionTreeItem>;
    using Context = QExplicitlySharedDataPointer<T>;

    //Standard constructor
    CodeCompletionItemTester(DUContext* context, const QString& text = QStringLiteral( "; " ),
                             const QString& followingText = QString(),
                             const CursorInRevision& position = CursorInRevision::invalid())
        : completionContext(new T(DUContextPointer(context), text, followingText,
                position.isValid() ? position : context->range().end))
    {
        init();
    }

    //Can be used if you already have the completion context
    CodeCompletionItemTester(const Context& context)
        : completionContext(context)
    {
        init();
    }

    //Creates a CodeCompletionItemTester for the parent context
    CodeCompletionItemTester parent() const
    {
        Context parent = Context(dynamic_cast<T*>(completionContext->parentContext()));
        Q_ASSERT(parent);
        return CodeCompletionItemTester(parent);
    }

    void addElements(const QList<Element>& elements)
    {
        for (auto& element : elements) {
            Item item(dynamic_cast<CompletionTreeItem*>(element.data()));
            if (item)
                items << item;
            auto* node = dynamic_cast<CompletionTreeNode*>(element.data());
            if (node)
                addElements(node->children);
        }
    }

    bool containsDeclaration(Declaration* dec) const
    {
        for (auto& item : items) {
            if (item->declaration().data() == dec) {
                return true;
            }
        }

        return false;
    }

    QList<Item> items; ///< All items retrieved
    QStringList names; ///< Names of all completion-items
    Context completionContext;

    //Convenience-function to retrieve data from completion-items by name
    QVariant itemData(const QString& itemName, int column = KTextEditor::CodeCompletionModel::Name,
                      int role = Qt::DisplayRole) const
    {
        return itemData(names.indexOf(itemName), column, role);
    }

    QVariant itemData(int itemNumber, int column = KTextEditor::CodeCompletionModel::Name,
                      int role = Qt::DisplayRole) const
    {
        if (itemNumber < 0 || itemNumber >= items.size())
            return QVariant();

        return itemData(items[itemNumber], column, role);
    }

    QVariant itemData(Item item, int column = KTextEditor::CodeCompletionModel::Name, int role = Qt::DisplayRole) const
    {
        return item->data(fakeModel().index(0, column), role, nullptr);
    }

    Item findItem(const QString& itemName) const
    {
        const auto idx = names.indexOf(itemName);
        if (idx < 0) {
            return {};
        }
        return items[idx];
    }

private:
    void init()
    {
        if (!completionContext || !completionContext->isValid()) {
            qWarning() << "invalid completion context";
            return;
        }

        bool abort = false;
        items = completionContext->completionItems(abort);

        addElements(completionContext->ungroupedElements());

        names.reserve(items.size());
        for (const Item& i : std::as_const(items)) {
            names <<
                i->data(fakeModel().index(0, KTextEditor::CodeCompletionModel::Name), Qt::DisplayRole,
                        nullptr).toString();
        }
    }

    static QStandardItemModel& fakeModel()
    {
        static QStandardItemModel model;
        model.setColumnCount(10);
        model.setRowCount(10);
        return model;
    }
};

/**
 * Helper class that inserts the given text into the duchain under the specified name,
 * allows parsing it with a simple call to parse(), and automatically releases the top-context
 *
 * The duchain must not be locked when this object is destroyed
 */
struct InsertIntoDUChain
{
    ///Artificially inserts a file called @p name with the text @p text
    InsertIntoDUChain(const QString& name, const QString& text) : m_insertedCode(IndexedString(name), text)
        , m_topContext(nullptr)
    {
    }

    ~InsertIntoDUChain()
    {
        get();
        release();
    }

    ///The duchain must not be locked when this is called
    void release()
    {
        if (m_topContext) {
            DUChainWriteLocker lock;

            m_topContext = nullptr;

            const QList<TopDUContext*> chains = DUChain::self()->chainsForDocument(m_insertedCode.file());
            for (TopDUContext* top : chains) {
                DUChain::self()->removeDocumentChain(top);
            }
        }
    }

    TopDUContext* operator->()
    {
        get();
        return m_topContext.data();
    }

    TopDUContext* tryGet()
    {
        DUChainReadLocker lock;
        return DUChain::self()->chainForDocument(m_insertedCode.file(), false);
    }

    void get()
    {
        if (!m_topContext)
            m_topContext = tryGet();
    }

    ///Helper function: get a declaration based on its qualified identifier
    Declaration* declaration(const QString& id)
    {
        get();
        if (!topContext())
            return nullptr;
        return DeclarationId(IndexedQualifiedIdentifier(QualifiedIdentifier(id))).declaration(topContext());
    }

    TopDUContext* topContext()
    {
        return m_topContext.data();
    }

    /**
     * Parses this inserted code as a stand-alone top-context
     * The duchain must not be locked when this is called
     *
     * @param features The features that should be requested for the top-context
     * @param update Whether the top-context should be updated if it already exists. Else it will be deleted.
     */
    void parse(TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses, bool update = false)
    {
        if (!update)
            release();
        m_topContext = DUChain::self()->waitForUpdate(m_insertedCode.file(), features, false);
        Q_ASSERT(m_topContext);
        DUChainReadLocker lock;
        Q_ASSERT(!m_topContext->parsingEnvironmentFile()->isProxyContext());
    }

    InsertArtificialCodeRepresentation m_insertedCode;
    ReferencedTopDUContext m_topContext;
};

#endif // KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H
