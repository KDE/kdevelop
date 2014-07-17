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
        Colon,              /*!< @brief Append a colon after the item: item becomes item: */
        Brackets,           /*!< @brief Append brackets after the item and put the cursor in-between them: item becomes item(|) */
    };

    CompletionItem(KDevelop::DeclarationPointer decl, int inheritanceDepth, Decoration decoration);

    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
    virtual QString declarationName() const;
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

protected:
    virtual void executed(KTextEditor::Document* document, const KTextEditor::Range& word);

private:
    Decoration m_decoration;
};

}

#endif