/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __MODULECOMPLETIONITEM_H__
#define __MODULECOMPLETIONITEM_H__

#include <language/codecompletion/codecompletionitem.h>

namespace QmlJS {

class ModuleCompletionItem : public KDevelop::CompletionTreeItem
{
public:
    enum Decoration {
        Import,        /*!< "import module", used for QML module imports */
        Quotes         /*!< Put quotes around the module name */
    };

    ModuleCompletionItem(const QString &name, Decoration decoration);

    QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const override;
    int inheritanceDepth() const override;
    int argumentHintDepth() const override;
    KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const override;

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override;

private:
    QString m_name;
    Decoration m_decoration;
};

}

#endif
