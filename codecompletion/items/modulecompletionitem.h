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

#ifndef __MODULECOMPLETIONITEM_H__
#define __MODULECOMPLETIONITEM_H__

#include <language/codecompletion/codecompletionitem.h>

namespace QmlJS {

class ModuleCompletionItem : public KDevelop::CompletionTreeItem
{
public:
    ModuleCompletionItem(const QString &name);

    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
    virtual int inheritanceDepth() const;
    virtual int argumentHintDepth() const;
    virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

private:
    QString m_name;
    QString m_version;
};

}

#endif