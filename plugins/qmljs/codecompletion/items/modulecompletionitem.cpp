/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "modulecompletionitem.h"

#include <language/codecompletion/codecompletionmodel.h>
#include <language/duchain/duchainutils.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QIcon>

using namespace KDevelop;

QmlJS::ModuleCompletionItem::ModuleCompletionItem(const QString& name, Decoration decoration)
: m_name(name),
  m_decoration(decoration)
{
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

void QmlJS::ModuleCompletionItem::execute(KTextEditor::View* view, const KTextEditor::Range& word)
{
    switch (m_decoration) {
    case Import:
        // Replace the whole line with an import statement
        view->document()->replaceText(
            KTextEditor::Range(word.start().line(), 0, word.start().line(), INT_MAX),
            QLatin1String("import ") + m_name
        );
        break;
    case Quotes:
        view->document()->replaceText(word, QStringLiteral("\"%1\"").arg(m_name));
        break;
    }
}
