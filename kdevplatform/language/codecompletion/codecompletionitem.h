/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_KDEV_CODECOMPLETIONITEM_H
#define KDEVPLATFORM_KDEV_CODECOMPLETIONITEM_H

#include "../duchain/duchainpointer.h"

#include <KTextEditor/CodeCompletionModel>

namespace KTextEditor {
class CodeCompletionModel;
class Range;
class Cursor;
}

class QModelIndex;

namespace KDevelop {
class CodeCompletionModel;

struct CompletionTreeNode;
class CompletionTreeItem;
class IndexedType;

class KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeElement
    : public QSharedData
{
public:
    CompletionTreeElement();

    virtual ~CompletionTreeElement();

    CompletionTreeElement* parent() const;

    /// Reparenting is not supported. This is only allowed if parent() is still zero.
    void setParent(CompletionTreeElement*);

    int rowInParent() const;

    int columnInParent() const;

    /// Each element is either a node, or an item.

    CompletionTreeNode* asNode();

    CompletionTreeItem* asItem();

    template <class T>
    T* asItem()
    {
        return dynamic_cast<T*>(this);
    }

    template <class T>
    const T* asItem() const
    {
        return dynamic_cast<const T*>(this);
    }

    const CompletionTreeNode* asNode() const;

    const CompletionTreeItem* asItem() const;

private:
    CompletionTreeElement* m_parent;
    int m_rowInParent;
};

struct KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeNode
    : public CompletionTreeElement
{
    CompletionTreeNode();
    ~CompletionTreeNode() override;

    KTextEditor::CodeCompletionModel::ExtraItemDataRoles role;
    QVariant roleValue;

    /// Will append the child, and initialize it correctly to create a working tree-structure
    void appendChild(QExplicitlySharedDataPointer<CompletionTreeElement> );
    void appendChildren(const QList<QExplicitlySharedDataPointer<CompletionTreeElement>>& children);
    void appendChildren(const QList<QExplicitlySharedDataPointer<CompletionTreeItem>>& children);

    /// @warning Do not manipulate this directly, that's bad for consistency. Use appendChild instead.
    QList<QExplicitlySharedDataPointer<CompletionTreeElement>> children;
};

class KDEVPLATFORMLANGUAGE_EXPORT CompletionTreeItem
    : public CompletionTreeElement
{
public:

    /// Execute the completion item. The default implementation does nothing.
    virtual void execute(KTextEditor::View* view, const KTextEditor::Range& word);

    /// Should return normal completion data, @see KTextEditor::CodeCompletionModel
    /// The default implementation returns "unimplemented", so re-implement it!
    /// The duchain is not locked when this is called
    virtual QVariant data(const QModelIndex& index, int role, const CodeCompletionModel* model) const;

    /// Should return the inheritance-depth. The completion-items don't need to return it through the data() function.
    virtual int inheritanceDepth() const;
    /// Should return the argument-hint depth. The completion-items don't need to return it through the data() function.
    virtual int argumentHintDepth() const;

    /// The default-implementation calls DUChainUtils::completionProperties
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

    /// If this item represents a Declaration, this should return the declaration.
    /// The default-implementation returns zero.
    virtual DeclarationPointer declaration() const;

    /// Should return the types should be used for matching items against this one when it's an argument hint.
    /// The matching against all types should be done, and the best one will be used as final match result.
    virtual QList<IndexedType> typeForArgumentMatching() const;

    /// Should return whether this completion-items data changes with input done by the user during code-completion.
    /// Returning true is very expensive.
    virtual bool dataChangedWithInput() const;
};

/// A custom-group node, that can be used as-is. Just create it, and call appendChild to add group items.
/// The items in the group will be shown in the completion-list with a group-header that contains the given name
struct KDEVPLATFORMLANGUAGE_EXPORT CompletionCustomGroupNode
    : public CompletionTreeNode
{
    /// @param inheritanceDepth See KTextEditor::CodeCompletionModel::GroupRole
    explicit CompletionCustomGroupNode(const QString& groupName, int inheritanceDepth = 700);

    int inheritanceDepth;
};

using CompletionTreeItemPointer = QExplicitlySharedDataPointer<CompletionTreeItem>;
using CompletionTreeElementPointer = QExplicitlySharedDataPointer<CompletionTreeElement>;
}

Q_DECLARE_METATYPE(KDevelop::CompletionTreeElementPointer)

#endif
