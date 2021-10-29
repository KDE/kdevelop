/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;
    KDevelop::DeclarationPointer declaration() const override;
    int argumentHintDepth() const override;
    int inheritanceDepth() const override;
    KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const override;

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
