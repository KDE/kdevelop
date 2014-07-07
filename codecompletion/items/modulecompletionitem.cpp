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
#include "modulecompletionitem.h"

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainutils.h>
#include <ktexteditor/document.h>

#include <QIcon>

using namespace KDevelop;

QmlJS::ModuleCompletionItem::ModuleCompletionItem(const QString& name)
{
    QStringList nameAndVersion = name
        .section(QLatin1Char('.'), 0, -2)
        .split(QLatin1Char('_'));

    m_name = nameAndVersion.at(0);
    m_version = (nameAndVersion.count() > 1 ? nameAndVersion.at(1) : QLatin1String("1.0"));
}

int QmlJS::ModuleCompletionItem::argumentHintDepth() const
{
    return 0;
}

int QmlJS::ModuleCompletionItem::inheritanceDepth() const
{
    return m_name.count(QLatin1Char('.'));
}

CodeCompletionModel::CompletionProperties QmlJS::ModuleCompletionItem::completionProperties() const
{
    return CodeCompletionModel::Namespace;
}

QVariant QmlJS::ModuleCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{
    Q_UNUSED(model)

    switch (role) {
    case CodeCompletionModel::IsExpandable:
        return QVariant(false);

    case Qt::DisplayRole:
        switch (index.column()) {
        case CodeCompletionModel::Prefix:
            return QLatin1String("module");
        case CodeCompletionModel::Name:
            return m_name;
        case CodeCompletionModel::Postfix:
            return m_version;
        }
        break;

    case CodeCompletionModel::CompletionRole:
        return (int)completionProperties();

    case Qt::DecorationRole:
        if(index.column() == CodeCompletionModel::Icon) {
            CodeCompletionModel::CompletionProperties p = completionProperties();
            return DUChainUtils::iconForProperties(p);
        }
        break;
    }

    return QVariant();
}

void QmlJS::ModuleCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    // Replace the whole line with an import statement
    document->replaceText(
        KTextEditor::Range(word.start().line(), 0, word.start().line(), INT_MAX),
        QString("import %1 %2").arg(m_name, m_version)
    );
}
