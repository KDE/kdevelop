/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __COMPLETIONITEM_H__
#define __COMPLETIONITEM_H__

#include <language/codecompletion/normaldeclarationcompletionitem.h>

namespace QmlJS {

class CompletionItem : public KDevelop::NormalDeclarationCompletionItem
{
public:
    /**
     * @brief Decoration to put around an item when it is inserted into the code editor
     */
    enum Decoration
    {
        NoDecoration,       /*!< @brief No decoration at all */
        Quotes,             /*!< @brief Wrap the item in quotes: item becomes "item" */
        QuotesAndBracket,   /*!< @brief Wrap the item as in array subscripts: item becomes "item"] */
        ColonOrBracket,     /*!< @brief Append a colon or a bracket after the item: item becomes "item:" or "item {|}" if item has a structure type */
        Brackets,           /*!< @brief Append brackets after the item and put the cursor in-between them: item becomes item(|) */
    };

    CompletionItem(const KDevelop::DeclarationPointer& decl, int inheritanceDepth, Decoration decoration);

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;
    QString declarationName() const override;
    KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const override;

protected:
    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override;

private:
    Decoration m_decoration;
};

}

#endif
