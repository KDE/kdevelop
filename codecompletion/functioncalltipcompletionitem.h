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
#ifndef __FUNCTIONCALLTIPCOMPLETIONITEM_H__
#define __FUNCTIONCALLTIPCOMPLETIONITEM_H__

#include <language/codecompletion/codecompletionitem.h>
#include <language/duchain/declaration.h>

namespace QmlJS {

/**
 * @brief Call-tip for functions
 *
 * Display the signature of a function and highlight its argument currently being
 * edited.
 */
class FunctionCalltipCompletionItem : public KDevelop::CompletionTreeItem
{
public:
    /** @note The DU-Chain lock must be held when calling this constructor */
    FunctionCalltipCompletionItem(const KDevelop::DeclarationPointer& decl,
                                  int depth,
                                  int argumentIndex);

    KDevelop::AbstractType::Ptr currentArgumentType() const;

    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
    virtual KDevelop::DeclarationPointer declaration() const;
    virtual int argumentHintDepth() const;
    virtual int inheritanceDepth() const;
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

private:
    KDevelop::DeclarationPointer m_declaration;
    KDevelop::AbstractType::Ptr m_currentArgumentType;

    QString m_prefix;
    QString m_arguments;
    int m_depth;
    int m_currentArgumentStart;
    int m_currentArgumentLength;
};

}

#endif